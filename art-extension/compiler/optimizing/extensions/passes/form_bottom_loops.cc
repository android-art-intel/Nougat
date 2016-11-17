/*
 * Copyright (C) 2015 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "form_bottom_loops.h"

#include "cloning.h"
#include "ext_utility.h"
#include "loop_iterators.h"
#include "reference_type_propagation.h"

// For debugging purposes.
#ifdef __ANDROID__
#include "cutils/properties.h"
#endif

namespace art {

// Stores all required information about blocks of a loop being optimized.
struct FBLContext {
  enum InstructionPosition {
    kHeader,
    kExit,
    kPhi,
    kBackEdge,
    kClone
  };

  FBLContext(HLoopInformation_X86* loop,
             HBasicBlock* pre_header_block,
             HBasicBlock* header_block,
             HBasicBlock* first_block,
             HBasicBlock* back_block,
             HBasicBlock* exit_block) :
    loop_(loop),
    pre_header_block_(pre_header_block),
    header_block_(header_block),
    first_block_(first_block),
    back_block_(back_block),
    exit_block_(exit_block) { }

  InstructionPosition GetInstructionPosition(HInstruction* inst) {
    HBasicBlock* user_block = inst->GetBlock();
    if (user_block != header_block_) {
        if (clones_.find(inst) == clones_.end()) {
          return loop_->Contains(*user_block) ? kBackEdge : kExit;
        } else {
          return kClone;
        }
    } else {
      return inst->IsPhi() ? kPhi : kHeader;
    }
  }

  HLoopInformation_X86* loop_;
  HBasicBlock* pre_header_block_;
  HBasicBlock* header_block_;
  HBasicBlock* first_block_;
  HBasicBlock* back_block_;
  HBasicBlock* exit_block_;
  // Contains all clones.
  std::unordered_set<HInstruction*> clones_;

  // Maps phi on phi' that is used to fix up its uses.
  InstrToInstrMap phi_fixup_;

  // Maps phi on Phi(phi(0), phi') that is used to fix up its uses.
  InstrToInstrMap interlace_phi_fixup_inside_;
  InstrToInstrMap interlace_phi_fixup_outside_;

  // Maps insn on Phi(insn, clone).
  InstrToInstrMap header_fixup_inside_;
  InstrToInstrMap header_fixup_outside_;
};

void HFormBottomLoops::Run() {
  DCHECK(handles_ != nullptr);
  HGraph_X86* graph = GetGraphX86();
  HLoopInformation_X86* graph_loop = graph->GetLoopInformation();

  PRINT_PASS_OSTREAM_MESSAGE(this, "Begin: " << GetMethodName(graph));

  // Store the loops we have formed for debug purposes only.
  std::vector<HLoopInformation_X86*> bottom_formed_loops;
  // Whether we have changed something.
  bool changed = false;

  // Walk all the inner loops in the graph.
  for (HOnlyInnerLoopIterator it(graph_loop); !it.Done(); it.Advance()) {
    HLoopInformation_X86* loop = it.Current();
    HBasicBlock* pre_header = loop->GetPreHeader();

    PRINT_PASS_OSTREAM_MESSAGE(this, "Visit " << loop->GetHeader()->GetBlockId()
                                     << ", preheader = " << pre_header->GetBlockId());

    HBasicBlock* loop_header = loop->GetHeader();
    HBasicBlock* exit_block = loop->GetExitBlock();

    if (!ShouldTransformLoop(loop, loop_header, exit_block)) {
      // Loop is already correct, or not valid for rewrite.
      continue;
    }

    RewriteLoop(loop, loop_header, exit_block);
    MaybeRecordStat(MethodCompilationStat::kIntelFormBottomLoop);
    changed = true;
    if (IsVerbose()) {
      bottom_formed_loops.push_back(loop);
    }
  }

  if (changed) {
    // Rebuild the loop data structures.
    graph->RebuildDomination();
    if (has_reference_phis_) {
      ReferenceTypePropagation(graph_,
                               dex_compilation_unit_.GetDexCache(),
                               handles_,
                               /* is_first_run */ false).Run();
    }
  }

  if (IsVerbose()) {
    // Can dump loops only after domination info is valid.
    for (auto loop : bottom_formed_loops) {
      loop->Dump(LOG(INFO));
    }
  }
  PRINT_PASS_OSTREAM_MESSAGE(this, "End: " << GetMethodName(graph));
}

static bool IsSingleGotoBackEdge(HBasicBlock* bb) {
  HLoopInformation* loop = bb->GetLoopInformation();
  return bb->IsSingleGoto()
         && (loop != nullptr && loop->IsBackEdge(*bb));
}

bool HFormBottomLoops::ShouldTransformLoop(HLoopInformation_X86* loop,
                                           HBasicBlock* loop_header,
                                           HBasicBlock* exit_block) const {
  if (exit_block == nullptr) {
    // We need exactly 1 exit block from the loop.
    PRINT_PASS_MESSAGE(this, "Too many or too few exit blocks");
    return false;
  }

  if (loop->IsOrHasIrreducibleLoop()) {
    PRINT_PASS_OSTREAM_MESSAGE(this, "Loop is irreducible.");
    return false;
  }

  // Exit block is alone and it always has one predecessor due to
  // critical edge elimination.
  DCHECK_EQ(exit_block->GetPredecessors().size(), static_cast<size_t>(1));
  HBasicBlock* loop_to_exit = exit_block->GetPredecessors()[0];

  HBasicBlock* first_back_edge = loop->GetBackEdges()[0];

  // Is this a top tested loop?
  HInstruction* last_insn = loop_header->GetLastInstruction();
  DCHECK(last_insn != nullptr);
  if (!last_insn->IsIf()) {
    PRINT_PASS_MESSAGE(this, "Loop header doesn't end with HIf");

    // Perhaps this loop is already bottom tested.
    // We must ensure that:
    // 1) loop_to_exit ends up with if.
    // 2) There is only one back edge.
    // 3) back edge is a successor of loop_to_exit.
    // 4) back edge is single goto.
    bool is_bottom_tested =
      (loop_to_exit->GetLastInstruction()->IsIf()) &&
      (loop->NumberOfBackEdges() == 1) &&
      (std::find(first_back_edge->GetPredecessors().begin(),
                  first_back_edge->GetPredecessors().end(),
                  loop_to_exit) != first_back_edge->GetPredecessors().end()) &&
      (IsSingleGotoBackEdge(first_back_edge));

    if (is_bottom_tested) {
      PRINT_PASS_MESSAGE(this, "Loop is already bottom tested");
      loop->SetBottomTested(true);
    }

    return false;
  }

  // We don't know how to rewrite a loop with multiple back edges at this time.
  if (loop->NumberOfBackEdges() != 1) {
    PRINT_PASS_MESSAGE(this, "More than one back edge");
    return false;
  }

  // Does the loop header exit the loop, making it top tested?
  if (loop_to_exit == loop_header) {
    // Perhaps this loop is already bottom tested.
    // We must ensure that:
    // 1) loop_to_exit ends up with if - already done.
    // 2) There is only one back edge - checked above.
    // 3) back edge is a successor of loop_to_exit.
    // 4) back edge is single goto.
    bool is_bottom_tested =
        (std::find(first_back_edge->GetPredecessors().begin(),
                    first_back_edge->GetPredecessors().end(),
                    loop_to_exit) != first_back_edge->GetPredecessors().end()) &&
        (IsSingleGotoBackEdge(first_back_edge));
    if (is_bottom_tested) {
      // More complicated single block loop.
      PRINT_PASS_MESSAGE(this, "Complex Loop is already bottom tested (after gotos)");
      loop->SetBottomTested(true);
      return false;
    }
  } else {
    PRINT_PASS_MESSAGE(this, "Loop header doesn't exit the loop");
    return false;
  }

  if (loop->HasTryCatchHandler()) {
    PRINT_PASS_OSTREAM_MESSAGE(this, "Found a try or catch handler inside the loop");
    return false;
  }

  // Are the instructions in the header okay?
  if (!CheckLoopHeader(loop_header)) {
    return false;
  }

  // Leave support for debugging the transformation in debug builds only.
  // Only works with -j1.
  if (kIsDebugBuild) {
    static int max = -1;
    static int count = 0;
    if (max == -1) {
#ifdef __ANDROID__
      char buff[PROPERTY_VALUE_MAX];
      if (property_get("dex2oat.bottom.max", buff, "1000000") > 0) {
        max = atoi(buff);
      }
#else
      char* p = getenv("BOTTOM_MAX");
      max = p ? atoi(p) : 1000000;
#endif
    }
    if (++count > max) {
      PRINT_PASS_MESSAGE(this, "MAX transform count exceeded");
      return false;
    }
  }

  PRINT_PASS_MESSAGE(this, "Loop gate passed");
  // Looks fine to go.
  return true;
}

void HFormBottomLoops::RewriteLoop(HLoopInformation_X86* loop,
                                   HBasicBlock* loop_header,
                                   HBasicBlock* exit_block) {
  // Paranoia: we have ensured it in gate.
  DCHECK(loop != nullptr);
  DCHECK(loop_header != nullptr);
  DCHECK(exit_block != nullptr);

  HGraph_X86* graph = GetGraphX86();
  HBasicBlock* pre_header = loop->GetPreHeader();

  PRINT_PASS_OSTREAM_MESSAGE(this, "Rewrite loop " << loop_header->GetBlockId()
                                    << ", preheader = " << pre_header->GetBlockId());
  PRINT_PASS_OSTREAM_MESSAGE(this, "Exit block = " << exit_block->GetBlockId());

  // Find the first block in the loop after the loop header.  There must be
  // one due to our pre-checks.
  DCHECK(loop_header->GetLastInstruction() != nullptr);
  HIf* if_insn = loop_header->GetLastInstruction()->AsIf();
  if (if_insn == nullptr) {
    // Impossible due to we checked it in ShouldTransformLoop
    // But to make KW be happy...
    return;
  }
  bool first_successor_is_exit = if_insn->IfTrueSuccessor() == exit_block;

  HBasicBlock* first_block = first_successor_is_exit
                               ? if_insn->IfFalseSuccessor()
                               : if_insn->IfTrueSuccessor();

  DCHECK(first_block != nullptr);

  // Move the SuspendCheck from the loop header to the first block.
  HSuspendCheck* suspend_check = loop->GetSuspendCheck();
  if (suspend_check != nullptr) {
    PRINT_PASS_OSTREAM_MESSAGE(this, "Moving suspend check to block "
                                     << first_block->GetBlockId());
    suspend_check->MoveBefore(first_block->GetFirstInstruction());
  }

  // Get loop back edge.
  HBasicBlock* back_block = loop->GetBackEdges()[0];

  // Fiil the context information that will be required in future.
  FBLContext context(loop,
                     pre_header,
                     loop_header,
                     first_block,
                     back_block,
                     exit_block);

  // Trahsform the CFG.
  DoCFGTransformation(graph, context, first_successor_is_exit);

  // First, clone all instructions to the end of back edge.
  HInstructionCloner cloner(graph);
  CloneInstructions(context, cloner);

  // After this transformation, the structure of the loop is following:
  //   [Preheader]
  //        |
  //        V
  //       [H]---> Exit
  //        |
  //        V
  //  ---->[B]  // Was back edge before.
  //  |     |
  //  |     V
  //  -----[C]---> Exit
  // Here H denotes former Header,
  //      B denotes former Back Edge,
  //      C denotes Clone of Header.
}


bool HFormBottomLoops::CheckLoopHeader(HBasicBlock* loop_header) const {
  // Walk through the instructions in the loop, looking for instructions that aren't
  // clonable, or that have results that are used outside of this block.
  HInstructionCloner cloner(GetGraphX86(), false);

  for (HInstructionIterator inst_it(loop_header->GetInstructions());
       !inst_it.Done();
       inst_it.Advance()) {
    HInstruction* insn = inst_it.Current();

    PRINT_PASS_OSTREAM_MESSAGE(this, "Look at: " << insn);

    switch (insn->GetKind()) {
      case HInstruction::kSuspendCheck:
        // Special case SuspendCheck.  We don't care about it.
        continue;
      case HInstruction::kIf:
        // If the instruction is HIf, then we must have checked it in the gate.
        continue;
      default:
        if (insn->IsControlFlow() && !insn->CanThrow()) {
          // We can't handle any control flow except an HIf.
          PRINT_PASS_MESSAGE(this, "Instruction has control flow");
          return false;
        }

        // Can we clone this instruction?
        insn->Accept(&cloner);
        if (!cloner.AllOkay()) {
          PRINT_PASS_MESSAGE(this, "Unable to clone");
          return false;
        }
        break;
    }
  }

  // Cycled Phis like:
  //
  //   phi_1 = Phi(x, phi_2);
  //   phi_2 = Phi(y, phi_1);
  //
  // Cause us problems. We cannot add them correctly.
  // So for now we reject loops where this situation
  // is possible. TODO: Consider covering it in future.
  std::unordered_set<HInstruction*> seen_phis;
  bool looks_phis_forward = false;
  bool looks_phis_backward = false;

  // Make sure that Phis don't form cycles.
  for (HInstructionIterator inst_it(loop_header->GetPhis());
       !inst_it.Done();
       inst_it.Advance()) {
    HInstruction* phi = inst_it.Current();
    if (phi->InputCount() == 2u) {
      HInstruction* in_2 = phi->InputAt(1)->AsPhi();
      if (in_2 == phi) {
        // InstructionSimplifier and probably some other optimisations
        // can produce degenerate phis like "j2 Phi [j1,j2]".
        // This breakes the phis fixup in preheader so remove it here.
        PRINT_PASS_OSTREAM_MESSAGE(this, "Removing degenerate phi " << phi);
        phi->ReplaceWith(phi->InputAt(0));
        loop_header->RemovePhi(phi->AsPhi());
      } else if (in_2 != nullptr && in_2->GetBlock() == loop_header) {
        if (seen_phis.find(in_2) == seen_phis.end()) {
          looks_phis_forward = true;
        } else {
          looks_phis_backward = true;
        }
      }
    }

    if (looks_phis_forward && looks_phis_backward) {
      PRINT_PASS_MESSAGE(this, "Rejecting due to cycled Phis");
      return false;
    }

    seen_phis.insert(phi);
  }

  // All instructions are okay.
  return true;
}

void HFormBottomLoops::DoCFGTransformation(HGraph_X86* graph,
                                           FBLContext& context,
                                           bool first_successor_is_exit) {
  // Retrieve all information from the context.
  HLoopInformation_X86* loop = context.loop_;
  HBasicBlock* pre_header = context.pre_header_block_;
  HBasicBlock* loop_header = context.header_block_;
  HBasicBlock* first_block = context.first_block_;
  HBasicBlock* back_block = context.back_block_;
  HBasicBlock* exit_block = context.exit_block_;


  PRINT_PASS_OSTREAM_MESSAGE(this, "Back block = " << back_block->GetBlockId());

  // Fix the successors of the block.
  if (first_successor_is_exit) {
    PRINT_PASS_OSTREAM_MESSAGE(this, "First successor is exit");
    back_block->ReplaceSuccessor(loop_header, exit_block);
    back_block->AddSuccessor(first_block);
  } else {
    PRINT_PASS_OSTREAM_MESSAGE(this, "Second successor is exit");
    back_block->ReplaceSuccessor(loop_header, first_block);
    back_block->AddSuccessor(exit_block);
  }

  HLoopInformation_X86* pre_header_loop =
    LOOPINFO_TO_LOOPINFO_X86(pre_header->GetLoopInformation());

  // Ensure that the exit block doesn't get messed up by SplitCriticalEdge.
  HBasicBlock* split_exit_block = graph->CreateNewBasicBlock();
  PRINT_PASS_OSTREAM_MESSAGE(this, "New loop exit block " << split_exit_block->GetBlockId());
  split_exit_block->AddInstruction(new (graph->GetArena()) HGoto());
  if (pre_header_loop != nullptr) {
    pre_header_loop->AddToAll(split_exit_block);
  }
  split_exit_block->InsertBetween(back_block, exit_block);
  split_exit_block->SetBlockCount(exit_block);

  // We also need to ensure that the branch around the loop isn't a critical edge.
  HBasicBlock* around_block = graph->CreateNewBasicBlock();
  PRINT_PASS_OSTREAM_MESSAGE(this, "New around to exit block " << around_block->GetBlockId());
  around_block->AddInstruction(new (graph->GetArena()) HGoto());
  if (pre_header_loop != nullptr) {
    pre_header_loop->AddToAll(around_block);
  }
  around_block->InsertBetween(loop_header, exit_block);
  around_block->SetBlockCount(exit_block);

  // We also need to ensure that the branch to the top of the loop isn't a critical edge.
  HBasicBlock* top_block = graph->CreateNewBasicBlock();
  PRINT_PASS_OSTREAM_MESSAGE(this, "New around to top block " << top_block->GetBlockId());
  top_block->AddInstruction(new (graph->GetArena()) HGoto());
  loop->AddToAll(top_block);
  top_block->InsertBetween(back_block, first_block);
  top_block->SetBlockCount(back_block);
  loop->ReplaceBackEdge(back_block, top_block);
  DCHECK(top_block->GetLoopInformation() == loop);

  // Ensure that a new loop header doesn't get messed up by SplitCriticalEdge.
  if (loop_header->GetSuccessors().size() > 1 &&
      first_block->GetPredecessors().size() > 1) {
    HBasicBlock* new_block = graph->CreateNewBasicBlock();
    PRINT_PASS_OSTREAM_MESSAGE(this, "Fixing up loop pre-header for " <<
                                     first_block->GetBlockId() <<
                                     ", new block = " << new_block->GetBlockId());
    new_block->AddInstruction(new (graph->GetArena()) HGoto());
    if (pre_header_loop != nullptr) {
      pre_header_loop->AddToAll(new_block);
    }
    new_block->InsertBetween(loop_header, first_block);
    new_block->SetBlockCount(pre_header);
  }

  // Fix the loop.
  loop->SetHeader(first_block);
  loop->Remove(loop_header);
  loop_header->SetLoopInformation(pre_header_loop);
  loop->SetBottomTested(true);

  // This is a required condition because we may create Phis
  // in the exit block that use instructions from Header.
  DCHECK(loop_header->Dominates(exit_block));
}

HPhi* HFormBottomLoops::NewPhi(HInstruction* in_1,
                               HInstruction* in_2,
                               HBasicBlock* block) {
  auto arena = graph_->GetArena();
  // Synthetic phis do not receive a proper VR number.
  // We could apply a VR number if we recycled the original phi instead of creating new one.
  HPhi* phi =  new (arena) HPhi(arena,
                                kNoRegNumber,
                                2u,
                                HPhi::ToPhiType(in_1->GetType()));

  phi->SetRawInputAt(0, in_1);
  if (phi->GetType() == Primitive::kPrimNot) {
    has_reference_phis_ = true;
  }
  if (in_2 != nullptr && block != nullptr) {
    phi->SetRawInputAt(1, in_2);
    block->AddPhi(phi);
    PRINT_PASS_OSTREAM_MESSAGE(this, "Created new " << phi
                                     << " in block " << block->GetBlockId());
  }

  return phi;
}

void HFormBottomLoops::CloneInstructions(FBLContext& context,
                                         HInstructionCloner& cloner) {
  HBasicBlock* loop_header = context.header_block_;
  HBasicBlock* back_block = context.back_block_;

  PRINT_PASS_OSTREAM_MESSAGE(this, "Cloning instructions from header block #"
                                   << loop_header->GetBlockId()
                                   << " to the end of back edge "
                                   << back_block->GetBlockId());

  // Find the Goto at the end of the back block.
  HInstruction* goto_insn = back_block->GetLastInstruction();
  DCHECK(goto_insn != nullptr);
  DCHECK(goto_insn->IsGoto());
  DCHECK_EQ(back_block->GetSuccessors().size(), 2u);

  PRINT_PASS_OSTREAM_MESSAGE(this, "Removing GOTO instruction " << goto_insn);
  back_block->RemoveInstruction(goto_insn);

  // Create the cloned instructions.
  for (HInstructionIterator inst_it(loop_header->GetInstructions());
                            !inst_it.Done();
                            inst_it.Advance()) {
    HInstruction* insn = inst_it.Current();

    // Special case for HLoadClass, we want to clone it to back branch and
    // the original node will go to pre-header which dominates the back
    // branch, so instead of cloning we can use just original HLoadClass node.
    // We do same for ClInitCheck because there is no gap expected between
    // the class loading and the check.
    if (insn->IsLoadClass() || insn->IsClinitCheck()) {
      cloner.AddCloneManually(insn, insn);
    } else {
      // Clone the instruction.
      insn->Accept(&cloner);
      HInstruction* cloned_insn = cloner.GetClone(insn);

      // Add the cloned instruction to the back block.
      if (cloned_insn != nullptr) {
        if (cloned_insn->GetBlock() == nullptr) {
          back_block->AddInstruction(cloned_insn);
          context.clones_.insert(cloned_insn);
          FixHeaderInsnUses(insn, cloned_insn, context);
        }

        PRINT_PASS_OSTREAM_MESSAGE(this, "Clone " << insn << " to " << cloned_insn);
      }
    }
  }

  HInstruction* if_insn = back_block->GetLastInstruction();
  DCHECK(if_insn != nullptr);
  // Check that in the end we have an IF instruction.
  DCHECK(if_insn->IsIf()) << "The last instruction of the back"
                         << " block is expected to be IF ("
                         << "found " <<  if_insn << ")";

  std::vector<HPhi*> to_remove;
  // Now move Phi nodes to the first block.
  for (HInstructionIterator inst_it(loop_header->GetPhis());
                            !inst_it.Done();
                            inst_it.Advance()) {
    HPhi* phi = inst_it.Current()->AsPhi();

    DCHECK_EQ(phi->InputCount(), 2u);
    FixHeaderPhisUses(phi, context);
    to_remove.push_back(phi);
  }

  for (auto phi : to_remove) {
    // We don't check !HasUses here because obsolete Phis
    // may still use each other.
    PRINT_PASS_OSTREAM_MESSAGE(this, "Removing obsolete phi " << phi);
    // Since they can use one another, we should prepare them
    // for deleting to avoid crashes in future making this hack.
    phi->ReplaceInput(phi->InputAt(0), 1);
  }

  for (auto phi : to_remove) {
    TryKillUseTree(this, phi);
    DCHECK(phi->GetBlock() == nullptr) << phi << " was not removed as expected!";
  }
}

void HFormBottomLoops::FixHeaderInsnUses(HInstruction* insn,
                                         HInstruction* clone,
                                         FBLContext& context) {
  HBasicBlock* first_block = context.first_block_;
  HBasicBlock* exit_block = context.exit_block_;
  bool has_header_phi_use = false;

  PRINT_PASS_OSTREAM_MESSAGE(this, "Fix uses for header instruction " << insn);
  // Fixup uses of instructions in header according to the following rules:
  // 1. use in header -> Do nothing;
  // 2. use in first block -> Replace with Phi(insn, clone) in first block;
  // 3. use is a clone -> Replace with clone;
  // 4. use is in/after exit -> Replace with Phi(insn, clone) in exit block.
  for (HAllUseIterator use_it(insn); !use_it.Done(); use_it.Advance()) {
    HInstruction* user = use_it.Current();
    HInstruction* new_input = nullptr;

    switch (context.GetInstructionPosition(user)) {
      case FBLContext::kHeader:
      case FBLContext::kClone:
        break;
      case FBLContext::kExit:
        new_input = GetHeaderFixup(insn, clone, context.header_fixup_outside_, exit_block);
        break;
      case FBLContext::kBackEdge:
        new_input = GetHeaderFixup(insn, clone, context.header_fixup_inside_, first_block);
        break;
      case FBLContext::kPhi:
        has_header_phi_use = true;
        break;
    }

    if (new_input != nullptr) {
      PRINT_PASS_OSTREAM_MESSAGE(this, "Replacing input of " << user << " with " << new_input);
      use_it.ReplaceInput(new_input);
    }
  }

  if (has_header_phi_use) {
    // We will need this for Phi fixup in future.
    GetHeaderFixup(insn, clone, context.header_fixup_inside_, first_block);
  }
}

void HFormBottomLoops::FixHeaderPhisUses(HPhi* phi,
                                         FBLContext& context) {
  HBasicBlock* first_block = context.first_block_;
  HBasicBlock* exit_block = context.exit_block_;

  PRINT_PASS_OSTREAM_MESSAGE(this, "Fix uses for header phi " << phi);
  // Fixup uses of phis in header according to the following rules:
  // 1. use in header -> Do nothing for phi, replace with phi(0) for insn;
  // 2. use in first block -> Replace with Phi(phi(0), phi') in first block;
  // 3. use is a clone -> Replace with phi';
  // 4. use is in/after exit -> Replace with Phi(phi(0), phi') in exit block.
  for (HAllUseIterator use_it(phi); !use_it.Done(); use_it.Advance()) {
    HInstruction* user = use_it.Current();
    HInstruction* new_input = nullptr;

    switch (context.GetInstructionPosition(user)) {
      case FBLContext::kHeader:
        new_input = phi->InputAt(0);
        break;
      case FBLContext::kExit:
        new_input = GetPhiInterlaceFixup(phi, context, context.interlace_phi_fixup_outside_, exit_block);
        break;
      case FBLContext::kBackEdge:
        new_input = GetPhiInterlaceFixup(phi, context, context.interlace_phi_fixup_inside_, first_block);
        break;
      case FBLContext::kClone:
        new_input = GetPhiFixup(phi, context);
        break;
      case FBLContext::kPhi:
        break;
    }

    if (new_input != nullptr) {
      PRINT_PASS_OSTREAM_MESSAGE(this, "Replacing input of " << user << " with " << new_input);
      use_it.ReplaceInput(new_input);
    }
  }
}

HInstruction* HFormBottomLoops::GetPhiFixup(HPhi* phi,
                                            FBLContext& context) {
  auto iter = context.phi_fixup_.find(phi);

  if (iter != context.phi_fixup_.end()) {
    return iter->second;
  }

  HBasicBlock* first_block = context.first_block_;

  HInstruction* fixup = nullptr;
  HInstruction* phi_1 = phi->InputAt(1);

  // Phi fixup phi' is defined by following rules:
  //  phi' = phi(1),
  //    if phi(1) in old back edge or old preheader;
  //  phi' = Phi(phi(1), clone(phi(1))),
  //    if phi(1) is an insn from old header;
  //  phi' = Phi(phi(1)(0), phi(1)'),
  //    if phi(1) is a Phi from old header.
  switch (context.GetInstructionPosition(phi_1)) {
    case FBLContext::kHeader: {
        auto iter_2 = context.header_fixup_inside_.find(phi_1);
        DCHECK(iter_2 != context.header_fixup_inside_.end());
        fixup = iter_2->second;
      }
      break;
    case FBLContext::kClone:
    case FBLContext::kExit:
    case FBLContext::kBackEdge:
      fixup = phi_1;
      break;
    case FBLContext::kPhi:
      fixup = GetPhiInterlaceFixup(phi_1->AsPhi(), context, context.interlace_phi_fixup_inside_, first_block);
      break;
  }

  context.phi_fixup_[phi] = fixup;
  return fixup;
}

HInstruction* HFormBottomLoops::GetPhiInterlaceFixup(HPhi* phi,
                                                     FBLContext& context,
                                                     InstrToInstrMap& mapping,
                                                     HBasicBlock* block) {
  auto iter = mapping.find(phi);

  if (iter != mapping.end()) {
    return iter->second;
  }

  HPhi* fixup = NewPhi(phi->InputAt(0));
  mapping[phi] = fixup;

  HInstruction* in_2 = GetPhiFixup(phi, context);

  DCHECK(in_2->GetBlock() != nullptr);
  fixup->SetRawInputAt(1, in_2);
  block->AddPhi(fixup);

  PRINT_PASS_OSTREAM_MESSAGE(this, "Created interlace phi fixup for " << phi
                                    << " in block " << block->GetBlockId());
  return fixup;
}

HInstruction* HFormBottomLoops::GetHeaderFixup(HInstruction* insn,
                                               HInstruction* clone,
                                               InstrToInstrMap& mapping,
                                               HBasicBlock* block) {
  auto iter = mapping.find(insn);

  if (iter != mapping.end()) {
    return iter->second;
  }

  HPhi* fixup = NewPhi(insn, clone, block);
  mapping[insn] = fixup;

  PRINT_PASS_OSTREAM_MESSAGE(this, "Created header fixup for " << insn
                                   << " in block " << block->GetBlockId());
  return fixup;
}

}  // namespace art
