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

#include "bb_simplifier.h"
#include "ext_utility.h"

namespace art {

bool HBBSimplifier::TrySimplifyIf(HBasicBlock* block, HIf* if_insn) {
  HGraph_X86* graph = GetGraphX86();

  // Paranoia.
  DCHECK(if_insn != nullptr);
  DCHECK_EQ(block->GetSuccessors().size(), 2u);

  // Simplify pattern:
  //    A
  //   / \
  //  B   C
  //   \ /
  //    D
  // Where B and C are single GOTO blocks, and D has no Phis.
  HBasicBlock* succ_1 = block->GetSuccessors()[0];
  HBasicBlock* succ_2 = block->GetSuccessors()[1];

  DCHECK(succ_1 != nullptr);
  DCHECK(succ_2 != nullptr);

  if (succ_1->IsSingleGoto()
      && succ_2->IsSingleGoto()
      && succ_1->GetPredecessors().size() == 1u
      && succ_2->GetPredecessors().size() == 1u) {
    HBasicBlock* lower_block = succ_1->GetSingleSuccessor();
    if (lower_block != block
        && lower_block == succ_2->GetSingleSuccessor()
        && lower_block->GetPhis().IsEmpty()) {

      PRINT_PASS_OSTREAM_MESSAGE(this, "Simplifying branching in block #"
                                       << block->GetBlockId());

      // First, remove block C.
      block->RemoveSuccessor(succ_2);
      block->RemoveDominatedBlock(succ_2);
      graph->DeleteBlock(succ_2, /* remove_from_loops */ true);

      // Second, remove block B, taking care about back edges.
      bool succ_1_is_a_back_edge = false;
      for (auto loop = LOOPINFO_TO_LOOPINFO_X86(succ_1->GetLoopInformation());
           loop != nullptr && !succ_1_is_a_back_edge;
           loop = loop->GetParent()) {
        succ_1_is_a_back_edge |= loop->IsBackEdge(*succ_1);
      }

      if (!succ_1_is_a_back_edge) {
        // We remove succ_1 because it is safe.
        block->ReplaceSuccessor(succ_1, lower_block);
        block->RemoveDominatedBlock(succ_1);
        graph->DeleteBlock(succ_1, /* remove_from_loops */ true);
      } else {
        // The succ_1 remains. Should we update domination with respect to that?
        if (lower_block->GetDominator() == block) {
          lower_block->SetDominator(succ_1);
          succ_1->AddDominatedBlock(lower_block);
        }
      }

      // Third, replace IF with GOTO.
      HInstruction* condition = if_insn->InputAt(0);
      block->ReplaceAndRemoveInstructionWith(if_insn, new (graph_->GetArena()) HGoto());

      // Finally, try to remove the condition.
      std::unordered_set<HInstruction*> all_removed;
      TryKillUseTree(this, condition, &all_removed);
      if (!all_removed.empty()) {
        MaybeRecordStat(MethodCompilationStat::kIntelBranchConditionDeleted);
        PRINT_PASS_OSTREAM_MESSAGE(this, "Removed condition and "
                                         << (all_removed.size() - 1)
                                         << " other instructions");
      }

      MaybeRecordStat(MethodCompilationStat::kIntelBranchSimplified);
      return true;
    }
  }

  return false;
}

bool HBBSimplifier::TrySimplify(HBasicBlock* block) {
  // Paranoia.
  DCHECK(block != nullptr);

  // Skip entry, exit blocks and catch blocks.
  if (block->IsEntryBlock()
      || block->IsExitBlock()
      || block->IsCatchBlock()) {
    return false;
  }

  // Simplification of blocks with two or more successors.
  HInstruction* last = block->GetLastInstruction();
  DCHECK(last != nullptr);
  DCHECK(last->IsControlFlow());
  if (last->IsIf()) {
    return TrySimplifyIf(block, last->AsIf());
  }

  return false;
}

void HBBSimplifier::Run() {
  HGraph_X86* graph = GetGraphX86();
  PRINT_PASS_OSTREAM_MESSAGE(this, "Start " << GetMethodName(graph));

  bool changed = false;

  for (HPostOrderIterator it(*graph_); !it.Done(); it.Advance()) {
    changed |= TrySimplify(it.Current());
  }

  PRINT_PASS_OSTREAM_MESSAGE(this, "End " << GetMethodName(graph));
}

}  // namespace art
