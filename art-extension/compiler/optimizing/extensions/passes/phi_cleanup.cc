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

#include "ext_utility.h"
#include "loop_iterators.h"
#include "phi_cleanup.h"

namespace art {

void HPhiCleanup::Run() {
  if (Gate()) {
    PRINT_PASS_OSTREAM_MESSAGE(this, "Begin: " << GetMethodName(graph_));
    CleanUpPhis();
    PRINT_PASS_OSTREAM_MESSAGE(this, "End " << GetMethodName(graph_));
  } else {
    PRINT_PASS_OSTREAM_MESSAGE(this, "Skip the method " << GetMethodName(graph_)
                                     << " because it has no loops");
  }
}

bool HPhiCleanup::Gate() const {
  // Apply only for methods with loops.
  return GetGraphX86()->GetLoopInformation() != nullptr;
}

void HPhiCleanup::CleanUpPhis() {
  // This is to avoid looking into the same Phis multiple times.
  std::unordered_set<HInstruction*> seen_insns;

  // Replace Phi(x, x, ... , x) with x.
  for (HPostOrderIterator it(*graph_); !it.Done(); it.Advance()) {
    HBasicBlock* block = it.Current();
    // Inputs of a CatchPhi don't dominate the catch block and DCHECK below fails.
    if (block->IsCatchBlock()) {
      continue;
    }
    for (HInstructionIterator phi_it(block->GetPhis());
                              !phi_it.Done(); phi_it.Advance()) {
      HPhi* phi = phi_it.Current()->AsPhi();
      DCHECK(phi != nullptr);
      if (phi->GetBlock() == nullptr) {
        // Has already been removed while removing another Phi.
        continue;
      }
      if (AllInputsSame(phi)) {
        PRINT_PASS_OSTREAM_MESSAGE(this, "Removing " << phi << " because all"
                                         << " its inputs are the same.");
        DCHECK(phi->InputAt(0)->GetBlock()->Dominates(block));
        if (phi->HasUses()) {
          phi->ReplaceWith(phi->InputAt(0));
        }
        DCHECK(!phi->HasUses());
        TryKillUseTree(this, phi);
        DCHECK(phi->GetBlock() == nullptr) << phi << " was not removed as expected!";
        MaybeRecordStat(MethodCompilationStat::kIntelPhiNodeEliminated);
      } else if (RemoveClique(phi, seen_insns)) {
        PRINT_PASS_OSTREAM_MESSAGE(this, "Clique removed successfully");
      }
    }
  }
}


bool HPhiCleanup::AllInputsSame(HPhi* phi) const {
  HInstruction* in_0 = phi->InputAt(0);
  for (size_t i = 1, e = phi->InputCount(); i < e; i++) {
    if (phi->InputAt(i) != in_0) {
      return false;
    }
  }
  return true;
}

bool HPhiCleanup::RemoveCliqueHelper(HInstruction* to_check,
                                     std::unordered_set<HInstruction*>& seen_insns,
                                     std::unordered_set<HInstruction*>& candidates) {
  if (candidates.find(to_check) != candidates.end()) {
    // We are already considering this instruction as a candidate for removal.
    return true;
  } else if (seen_insns.find(to_check) != seen_insns.end()) {
    // We have already rejected this instruction.
    return false;
  } else if (to_check->HasEnvironmentUses()) {
    // The Phi should not have env uses.
    return false;
  }

  // Consider this instruction as a candidate for removal.
  candidates.insert(to_check);
  seen_insns.insert(to_check);

  // We reject the instruction if its use is either a control flow or
  // it has side effects. In other cases we run the recursive check for it.
  const HUseList<HInstruction*>& uses = to_check->GetUses();
  for (auto it = uses.begin(), end = uses.end(); it != end; ++it) {
    HInstruction* use = it->GetUser();
    if (use->IsControlFlow() ||
        use->GetSideEffects().HasSideEffectsExcludingGC() ||
        use->CanThrow()) {
      // Instruction with side effects or control flow is not something that we can just remove.
      return false;
    } else if (!RemoveCliqueHelper(use, seen_insns, candidates)) {
      // Recursive check failed.
      return false;
    }
  }

  // This instruction seems a fine candidate for removal.
  return true;
}

bool HPhiCleanup::RemoveClique(HPhi* phi,
                               std::unordered_set<HInstruction*>& seen_insns) {
  // For quick checks we keep the clique candidates in set.
  std::unordered_set<HInstruction*> candidates;

  if (RemoveCliqueHelper(phi, seen_insns, candidates)) {
    PRINT_PASS_OSTREAM_MESSAGE(this, "Found a clique of: " << candidates.size() << " elements");

    std::unordered_set<HInstruction*> inputs_of_removed_phis;

    for (auto to_remove : candidates) {
      DCHECK(to_remove->GetBlock() != nullptr);
      // Consider all inputs of removed Phis for removal.
      for (HInputIterator it(to_remove); !it.Done(); it.Advance()) {
        auto input = it.Current();
        // This can be null due to unsafe removal.
        if (input != nullptr) {
          inputs_of_removed_phis.insert(input);
        }
      }

      // Remove the instruction unsafely.
      if (to_remove->IsPhi()) {
        to_remove->GetBlock()->RemovePhi(to_remove->AsPhi(), false);
      } else {
        to_remove->GetBlock()->RemoveInstruction(to_remove, false);
      }
      RemoveAsUser(to_remove);
    }

    for (auto input : inputs_of_removed_phis) {
      DCHECK(input != nullptr);
      if (input->GetBlock() != nullptr) {
        TryKillUseTree(this, input);
      }
    }

    return true;
  }

  return false;
}

}  // namespace art
