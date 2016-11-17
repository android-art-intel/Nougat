/*
 * Copyright (C) 2015 Intel Corporation.
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

#include "aur.h"
#include "ext_utility.h"

namespace art {

class AggressiveEnvUseRemover : public HGraphVisitor {
 public:
  AggressiveEnvUseRemover(HGraph* graph,
                          OptimizingCompilerStats* const stats,
                          HAggressiveUseRemoverPass* const pass) :
        HGraphVisitor(graph),
        stats_(stats),
        pass_(pass) {}
  virtual ~AggressiveEnvUseRemover() {}

  // These are throwers which are guaranteed to end current frame if there are no catches.
  // Thus everything live into them is automatically dead.
  void VisitBoundsCheck(HBoundsCheck* instr) OVERRIDE { HandleThrower(instr); }
  void VisitDivZeroCheck(HDivZeroCheck* instr) OVERRIDE { HandleThrower(instr); }
  void VisitNullCheck(HNullCheck* instr) OVERRIDE { HandleThrower(instr); }
  void VisitThrow(HThrow* instr) OVERRIDE { HandleThrower(instr); }

  // The following are guaranteed to trigger GC.
  void VisitSuspend(HSuspend* instr) OVERRIDE { HandleSuspend(instr); }
  void VisitSuspendCheck(HSuspendCheck* instr) OVERRIDE { HandleSuspend(instr); }

  // The following runtime calls may trigger GC but won't trigger deopt when app is not debuggable.
  void VisitLoadClass(HLoadClass* instr) OVERRIDE { HandlePotentialGC(instr); }
  void VisitClinitCheck(HClinitCheck* instr) OVERRIDE { HandlePotentialGC(instr); }
  void VisitInstanceOf(HInstanceOf* instr) OVERRIDE { HandlePotentialGC(instr); }
  void VisitCheckCast(HCheckCast* instr) OVERRIDE { HandlePotentialGC(instr); }
  void VisitMonitorOperation(HMonitorOperation* instr) OVERRIDE { HandlePotentialGC(instr); }
  void VisitNewArray(HNewArray* instr) OVERRIDE { HandlePotentialGC(instr); }
  void VisitNewInstance(HNewInstance* instr) OVERRIDE { HandlePotentialGC(instr); }
  void VisitArraySet(HArraySet* instr) OVERRIDE { HandlePotentialGC(instr); }

  // Debuggable applications need to support full-stack deopt. But non-debuggable
  // applications do not need to - this is because only last frame can get deoptimized
  // via HDeoptimize. However, tests which do not understand this constraint and walk
  // the stack on non-debuggable applications might fail finding the relevant values
  // which were killed via AUR.
  void VisitInvokeInterface(HInvokeInterface* instr) OVERRIDE { HandlePotentialGC(instr); }
  void VisitInvokeVirtual(HInvokeVirtual* instr) OVERRIDE { HandlePotentialGC(instr); }
  void VisitInvokeStaticOrDirect(HInvokeStaticOrDirect* instr) OVERRIDE { HandlePotentialGC(instr); }

  // The following are cases which are intentionally not handled.
  void VisitDeoptimize(HDeoptimize* instr ATTRIBUTE_UNUSED) OVERRIDE {
    /* Cannot remove any uses since needed to recover frame. */
  }

  void VisitBasicBlock(HBasicBlock* block) OVERRIDE {
    HBackwardInstructionIterator it(block->GetInstructions());
    DCHECK(it.Current()->IsControlFlow());
    for (; !it.Done(); it.Advance()) {
      HInstruction* insn = it.Current();
      // We need to check this because we could remove some
      // of the instructions while visiting others.
      if (insn->GetBlock() != nullptr) {
        insn->Accept(this);
      }
    }
  }

 private:
  bool IsInTryBlock(HInstruction* instr) {
    return instr->GetBlock()->IsTryBlock();
  }

  void RemoveUnusedInstructions(std::unordered_set<HInstruction*>& to_remove) {
    // Nothing to remove.
    if (to_remove.empty()) {
      return;
    }

    // These instructions will be removed during this processing.
    std::unordered_set<HInstruction*> removed;

    for (auto instruction : to_remove) {
      TryKillUseTree(pass_, instruction, &removed);
    }

    if (stats_ != nullptr) {
      for (auto instruction : removed) {
        stats_->RecordStat(MethodCompilationStat::kRemovedDeadInstruction);
        // This stat is a lower estimate since we do not actively remove instructions
        // that were caused to be dead from the current removal.
        stats_->RecordStat(MethodCompilationStat::kIntelRemovedDeadInstructionViaAUR);
        if (instruction->GetType() == Primitive::kPrimNot) {
          // We also count reference removals since those are great for GC
          // since the root set becomes smaller.
          stats_->RecordStat(MethodCompilationStat::kIntelRemovedDeadReferenceViaAUR);
        }
      }
    }
  }

  void MaybeConsiderForRemoval(bool should_remove,
                               HInstruction* candidate,
                               HInstruction* instruction,
                               HEnvironment* environment,
                               int32_t index,
                               std::unordered_set<HInstruction*>& to_remove) {
    if (should_remove) {
      PRINT_PASS_OSTREAM_MESSAGE(pass_, "Environment of " << candidate << " no longer uses"
                                        " the value defined by " << instruction);

      environment->RemoveAsUserOfInput(index);
      environment->SetRawEnvAt(index, nullptr);

      // Verify that the instruction has a definition - it must be the case since
      // it was used.
      DCHECK_NE(instruction->GetType(), Primitive::kPrimVoid);

      to_remove.insert(instruction);
    } else {
      PRINT_PASS_OSTREAM_MESSAGE(pass_, "Environment of " << candidate << " still uses"
                                        " the value defined by " << instruction << " because the"
                                        " object is live into environment.");
    }
  }

  void RemoveEnvAsUser(HInstruction* candidate, bool remove_references, bool is_in_try_block) {
    // We will maybe remove these instructions from code.
    std::unordered_set<HInstruction*> to_remove;

    for (HEnvironment* environment = candidate->GetEnvironment();
        environment != nullptr;
        environment = environment->GetParent()) {
      for (size_t i = 0, e = environment->Size(); i < e; ++i) {
        HInstruction* instruction = environment->GetInstructionAt(i);
        if (instruction != nullptr) {
          bool should_remove = true;

          // If we are in try block, we should keep the values that have
          // the Catch phis. Do not remove them from the env even if they
          // are not references.
          if (is_in_try_block &&
              candidate->CanThrowIntoCatchBlock()) {
            DCHECK(candidate->GetBlock()->GetTryCatchInformation() != nullptr);
            // Make sure that this vreg does not have a catch phi.
            auto& entry = candidate->GetBlock()->GetTryCatchInformation()->GetTryEntry();
            for (HBasicBlock* catch_block : entry.GetExceptionHandlers()) {
              for (HInstructionIterator phi_it(catch_block->GetPhis());
                                               !phi_it.Done();
                                               phi_it.Advance()) {
                HPhi* catch_phi = phi_it.Current()->AsPhi();
                DCHECK(catch_phi != nullptr);
                if (catch_phi->GetRegNumber() == i) {
                  // We need to keep this value because of catch phi.
                  should_remove = false;
                  break;
                }
              }

              if (!should_remove) {
                break;
              }
            }
          }

          if (!remove_references && should_remove) {
            if (instruction->GetType() == Primitive::kPrimNot) {
              if (!instruction->IsNullConstant()) {
                // So the reference is not null. Well it might still be dead if this
                // environment use is the last real use. One way we can determine if this
                // if all the uses strictly dominate this candidate.
                const HUseList<HInstruction*>& uses = instruction->GetUses();
                for (auto it = uses.begin(), end = uses.end(); it != end; ++it) {
                  HInstruction* user = it->GetUser();
                  if (!user->StrictlyDominates(candidate) ||
                      user->CanBeSubstitutedWithItsInput()) {
                    should_remove = false;
                    break;
                  }
                }
              }
            }
          }
          // Remove current instruction from env and consider it for further
          // removal from the code.
          MaybeConsiderForRemoval(should_remove, candidate, instruction,
                                  environment, i, to_remove);
        }
      }
    }

    RemoveUnusedInstructions(to_remove);
  }

  void HandleCaller(HInstruction* instr, bool may_trigger_gc) {
    // Only remove references if we cannot trigger GC (or triggering GC
    // does not affect current frame) and if we are not in the try block.
    const bool remove_references = !may_trigger_gc && !IsInTryBlock(instr);
    RemoveEnvAsUser(instr, remove_references, IsInTryBlock(instr));
  }

  void HandleThrower(HInstruction* instr) {
    DCHECK(instr->HasEnvironment());
    // We mark that we may not trigger GC because throwers are guaranteed to kill
    // the current frame (thus those objects are also dead).
    const bool may_trigger_gc = false;
    HandleCaller(instr, may_trigger_gc);
  }

  void HandlePotentialGC(HInstruction* instr) {
    if (!instr->HasEnvironment()) {
      return;
    }
    const bool may_trigger_gc = instr->GetSideEffects().Includes(SideEffects::CanTriggerGC());
    HandleCaller(instr, may_trigger_gc);
  }

  void HandleSuspend(HInstruction* suspend) {
    DCHECK(suspend->HasEnvironment());
    // The whole point of suspend check is that we are not removing objects.
    const bool remove_references = false;
    RemoveEnvAsUser(suspend, remove_references, IsInTryBlock(suspend));
  }

  OptimizingCompilerStats* const stats_;
  HAggressiveUseRemoverPass* const pass_;
  DISALLOW_COPY_AND_ASSIGN(AggressiveEnvUseRemover);
};

void HAggressiveUseRemoverPass::Run() {
  PRINT_PASS_OSTREAM_MESSAGE(this, "Start " << GetMethodName(graph_));

  // The aggressive use removal makes it so method is no longer debuggable. Thus if debuggability
  // is needed, we cannot run this pass.
  // The boot image also needs to be treated as debuggable for this purpose.
  // We also cannot do AUR in OSR mode, because it can invalidate stack maps,
  // and this leads to OSR jumps into wrong places.
  if (is_boot_image_ || graph_->IsDebuggable() || graph_->IsCompilingOsr()) {
    const char* reject_message =
      is_boot_image_ ? "Rejecting because we are compiling boot image." :
      graph_->IsDebuggable() ? "Rejecting because the graph is marked as being debuggable." :
      "Rejecting because of compilation in OSR mode.";
    PRINT_PASS_OSTREAM_MESSAGE(this, "End " << GetMethodName(graph_) << ". " << reject_message);
    return;
  }

  AggressiveEnvUseRemover aur(graph_, stats_, this);
  // We use same walk as DCE.
  for (HPostOrderIterator b(*graph_); !b.Done(); b.Advance()) {
    HBasicBlock* block = b.Current();
    aur.VisitBasicBlock(block);
  }

  PRINT_PASS_OSTREAM_MESSAGE(this, "End " << GetMethodName(graph_));
}

}  // namespace art
