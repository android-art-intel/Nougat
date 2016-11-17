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

#include "remove_suspend.h"

#include "ext_utility.h"
#include "loop_iterators.h"
#include "pass_option.h"

namespace art {

void HRemoveLoopSuspendChecks::Run() {
  HGraph_X86* graph = GetGraphX86();
  HLoopInformation_X86 *graph_loop_info = graph->GetLoopInformation();
  PRINT_PASS_OSTREAM_MESSAGE(this, "Begin: " << GetMethodName(graph));

  // Don't mess with suspend checks if OSR is enabled.
  if (graph_->IsCompilingOsr()) {
    PRINT_PASS_MESSAGE(this, "Skip this method because it uses On-Stack-Replacement");
  } else {
    // Walk all the inner loops in the graph.
    for (HOnlyInnerLoopIterator it(graph_loop_info); !it.Done(); it.Advance()) {
      HLoopInformation_X86* loop_info = it.Current();
      HBasicBlock* pre_header = loop_info->GetPreHeader();

      PRINT_PASS_OSTREAM_MESSAGE(this, "Visit " << loop_info->GetHeader()->GetBlockId()
                                       << ", preheader = " << pre_header->GetBlockId());

      // There must be a suspend check to remove.
      HSuspendCheck* suspend_check = loop_info->GetSuspendCheck();
      if (suspend_check == nullptr) {
        PRINT_PASS_MESSAGE(this, "No SuspendCheck in the loop");
        continue;
      }

      // This must be a countable loop.
      if (!loop_info->HasKnownNumIterations()) {
        PRINT_PASS_MESSAGE(this, "Loop is not countable");
        continue;
      }

      DCHECK(loop_info->IsInner());
      // This must be a simple loop.
      bool is_simple = true;
      for (HBlocksInLoopIterator bb_it(*loop_info); is_simple && !bb_it.Done(); bb_it.Advance()) {
        HBasicBlock* bb = bb_it.Current();
        for (HInstructionIterator insn_it(bb->GetInstructions()); !insn_it.Done(); insn_it.Advance()) {
          HInstruction* insn = insn_it.Current();

          if (insn->IsReturn() ||
              insn->IsNewInstance() ||
              insn->IsNewArray() ||
              insn->IsArrayLength() ||
              insn->IsThrow() ||
              insn->IsInvoke() ||
              insn->IsLoadException()) {
            is_simple = false;
            break;
          }
        }
      }
      if (!is_simple) {
        PRINT_PASS_MESSAGE(this, "Loop is not very simple");
        continue;
      }

      // There must not be another way to exit the loop.
      if (loop_info->CanSideExit()) {
        PRINT_PASS_MESSAGE(this, "Loop can side exit");
        continue;
      }

      uint64_t cost = 0;
      if (!loop_info->GetLoopCost(&cost) || cost == 0) {
        // We ran into an issue while counting instructions in the loop.
        PRINT_PASS_MESSAGE(this, "Unable to compute loop cost");
        continue;
      }

      // Compute the total cost of the loop.
      uint64_t num_iterations = loop_info->GetNumIterations(loop_info->GetHeader());
      if (num_iterations >= (std::numeric_limits<uint64_t>::max() / cost)) {
        // We would overflow computing the total cost.
        PRINT_PASS_MESSAGE(this, "cost * num_iterations is too large");
        continue;
      }
      cost *= num_iterations;

      static PassOption<int64_t> max_suspend_cost(this, driver_, "MaxSuspendFreeLoopCost",
        MAX_SUSPEND_TIME_CYCLES);
      if (cost > static_cast<uint64_t>(max_suspend_cost.GetValue())) {
        PRINT_PASS_OSTREAM_MESSAGE(this, "The cost of the loop (" << cost
                                         << ") exceeds " << max_suspend_cost.GetValue());
        continue;
      }

      // Remove the suspend check.
      PRINT_PASS_OSTREAM_MESSAGE(this, "Remove the suspend check from loop "
                               << loop_info->GetHeader()->GetBlockId()
                               << ", preheader = " << pre_header->GetBlockId());
      TryKillUseTree(this, suspend_check);
      DCHECK(suspend_check->GetBlock() == nullptr) << suspend_check
                                                   << " was not removed as expected!";
      loop_info->SetSuppressSuspendCheck(true);
      loop_info->SetSuspendCheck(nullptr);
      MaybeRecordStat(MethodCompilationStat::kIntelRemoveSuspendCheck);
    }
  }
  PRINT_PASS_OSTREAM_MESSAGE(this, "End: " << GetMethodName(graph));
}

}  // namespace art
