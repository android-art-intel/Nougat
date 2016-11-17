/*
 * Copyright (C) 2016 Intel Corporation
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
 */

#include "common_dominator.h"
#include "ext_utility.h"
#include "pass_option.h"
#include "speculation_pass.h"

namespace art {

bool HSpeculationPass::Gate() const {
  if (compiler_driver_->GetInstructionSet() != kX86 &&
      compiler_driver_->GetInstructionSet() != kX86_64) {
    PRINT_PASS_MESSAGE(this, "Skipping pass because ISA is not supported.");
    return false;
  }

  // Speculation passes insert instructions which may not have an implementation in other ISAs.
  return true;
}

void HSpeculationPass::GroupCandidatesWithOrdering(CandidatesMap& candidates_grouped,
                           const std::vector<HInstruction*>* candidates) {
  DCHECK(candidates != nullptr);

  if (candidates->size() == 1) {
    std::vector<HInstruction*> v;
    candidates_grouped.insert(std::make_pair((*candidates)[0], v));
    return;
  }

  std::set<HInstruction*> handled_candidates;
  for (const auto it: *candidates) {
    std::vector<HInstruction*> vec;
    HInstruction* top = it;
    if (handled_candidates.find(it) != handled_candidates.end()) {
      continue;
    }

    for (const auto it2: *candidates) {
      if (top == it2) {
        continue;
      }
      if (handled_candidates.find(it2) != handled_candidates.end()) {
        continue;
      }
      if (IsPredictionSame(top, it2)) {
        if (top->StrictlyDominates(it2)) {
          // If this is a root element, delete it from root.
          if (candidates_grouped.find(it2) != candidates_grouped.end()) {
            candidates_grouped.erase(it2);
          }
          vec.push_back(it2);
          handled_candidates.insert(it2);
        } else if (it2->StrictlyDominates(top)) {
          vec.push_back(top);
          handled_candidates.insert(top);
          top = it2;
        }
      }
    }
    candidates_grouped.insert(std::make_pair(top, vec));
  }
}

bool HSpeculationPass::HandleDeopt(HInstruction* candidate) {
  HSpeculationGuard* guard = InsertSpeculationGuard(candidate, candidate);

  // If guard insertion fails return false to mark we failed deopt generation.
  if (guard == nullptr) {
    return false;
  }

  // Create a deoptimization node that depends on this.
  HDeoptimize* deopt = new (graph_->GetArena()) HDeoptimize(guard, candidate->GetDexPc());
  guard->GetBlock()->InsertInstructionAfter(deopt, guard);
  deopt->CopyEnvironmentFrom(candidate->GetEnvironment());

  return true;
}

void HSpeculationPass::CodeVersioningHelper(HInstruction* candidate,
                                            HInstructionCloner& cloner,
                                            HSpeculationGuard* guard,
                                            bool needs_trap) {
  HGraph_X86* graph = GetGraphX86();

  HBasicBlock* pre_block = candidate->GetBlock();
  HBasicBlock* post_block = pre_block->SplitBeforeForInlining(candidate);
  graph_->AddBlock(post_block);
  HBasicBlock* fail_block = graph->CreateNewBasicBlock(candidate->GetDexPc());
  HBasicBlock* success_block = graph->CreateNewBasicBlock(candidate->GetDexPc());

  // Add control flow instruction which uses the result of guard.
  HIf* hif = new (graph->GetArena()) HIf(guard, candidate->GetDexPc());
  pre_block->AddInstruction(hif);

  graph->CreateLinkBetweenBlocks(pre_block, fail_block, true, true);
  graph->CreateLinkBetweenBlocks(pre_block, success_block, true, true);

  if (needs_trap) {
    HTrap* trap = new (graph->GetArena()) HTrap(candidate->GetDexPc());
    fail_block->AddInstruction(trap);
    HBasicBlock* exit_block = graph->GetExitBlock();
    // Compute dominator for exit block since it is no longer valid.
    HBasicBlock* common_dominator = CommonDominator::ForPair(fail_block, exit_block);
    exit_block->GetDominator()->RemoveDominatedBlock(exit_block);
    common_dominator->AddDominatedBlock(exit_block);
    exit_block->SetDominator(common_dominator);
    // Create link between the two blocks.
    fail_block->AddSuccessor(exit_block);
  } else {
    // Add a goto to the end of the success block.
    HGoto* got = new (graph->GetArena()) HGoto(candidate->GetDexPc());
    success_block->AddInstruction(got);
    // Move the candidate into our new block.
    graph->MoveInstructionBefore(candidate, got);

    // Duplicate the success block now.
    for (HInstructionIterator inst_it(success_block->GetInstructions());
         !inst_it.Done();
         inst_it.Advance()) {
      HInstruction* insn = inst_it.Current();
      insn->Accept(&cloner);
      DCHECK(cloner.AllOkay());
      HInstruction* clone = cloner.GetClone(insn);
      fail_block->AddInstruction(clone);
    }

    // Link the two blocks to the rest of graph.
    DCHECK_NE(success_block, post_block);
    DCHECK_NE(fail_block, post_block);
    graph->CreateLinkBetweenBlocks(fail_block, post_block, false, true);
    success_block->AddSuccessor(post_block);
    post_block->SetDominator(pre_block);
    pre_block->AddDominatedBlock(post_block);

    // Create a phi node to represent the final result.
    if (candidate->GetType() != Primitive::kPrimVoid) {
      HPhi* phi = new (graph->GetArena()) HPhi(graph->GetArena(), kNoRegNumber, 0,
                                               HPhi::ToPhiType(candidate->GetType()),
                                               candidate->GetDexPc());
      post_block->AddPhi(phi);
      candidate->ReplaceWith(phi);
      phi->AddInput(cloner.GetClone(candidate));
      phi->AddInput(candidate);

      // Ensure we set the RTI to something valid.
      if (candidate->GetType() == Primitive::kPrimNot) {
        phi->SetReferenceTypeInfo(candidate->GetReferenceTypeInfo());
      }
    }
  }

  // Fix loop and try-catch information.
  graph_->UpdateLoopAndTryInformationOfNewBlock(
      post_block, pre_block, /* replace_if_back_edge */ true);

  graph_->UpdateLoopAndTryInformationOfNewBlock(
      fail_block, pre_block, /* replace_if_back_edge */ false);

  graph_->UpdateLoopAndTryInformationOfNewBlock(
      success_block, pre_block, /* replace_if_back_edge */ false);

  PRINT_PASS_OSTREAM_MESSAGE(this, "Successfully handled " << candidate << " via code versioning.");
}

bool HSpeculationPass::HandleCodeVersioning(HInstruction* candidate,
                                            std::vector<HInstruction*>* similar_candidates,
                                            HInstructionCloner& cloner,
                                            bool with_counting ATTRIBUTE_UNUSED,
                                            bool needs_trap,
                                            VersioningApproach versioning) {
  DCHECK(candidate != nullptr);
  DCHECK(similar_candidates != nullptr);

  if (versioning == kVersioningLoop || versioning == kVersioningRange) {
    // TODO Add support for these versioning schemes when interesting.
    PRINT_PASS_OSTREAM_MESSAGE(this, "Failed to handle " << candidate << " because versioning " <<
                               versioning << " is not supported.");
    return false;
  } else if (versioning == kVersioningAny || versioning == kVersioningLocal) {
    // We treat the any versioning as local versioning so we do not bloat code.

    // First try to insert the guard. If we fail, we must reject this case.
    HSpeculationGuard* guard =
        InsertSpeculationGuard(candidate, candidate);
    if (guard == nullptr) {
      PRINT_PASS_OSTREAM_MESSAGE(this, "Failed to handle " << candidate << " because guard "
                                 "insertion failed.");
      return false;
    }

    // Now we add control flow instructions which use the result of
    // the guard.

    // Handle first candidate in the list.
    CodeVersioningHelper(candidate, cloner, guard, needs_trap);

    // Handle all other candidates.
    if (!(*similar_candidates).empty()) {
      for (auto it: *similar_candidates) {
        CodeVersioningHelper(it, cloner, guard, needs_trap);
      }
    }
  }

  return true;
}

static bool NeedsCounting(SpeculationRecoveryApproach recovery) {
  return recovery == kRecoveryCodeVersioningWithCounting;
}

static bool NeedsDeopt(SpeculationRecoveryApproach recovery) {
  return recovery == kRecoveryDeopt;
}

static bool NeedsCodeVersioning(SpeculationRecoveryApproach recovery) {
  return recovery == kRecoveryCodeVersioning || recovery == kRecoveryCodeVersioningWithCounting ||
      recovery == kRecoveryFault;
}

static bool NeedsTrap(SpeculationRecoveryApproach recovery) {
  return recovery == kRecoveryFault;
}

static bool NeedsNoRecovery(SpeculationRecoveryApproach recovery) {
  return recovery == kRecoveryNotNeeded;
}

class ScopedMessage {
 public:
  explicit ScopedMessage(HSpeculationPass* pass) :
      pass_instance_(pass) {
    if (pass_instance_->IsVerbose()) {
      const DexCompilationUnit& compilation_unit = pass_instance_->GetDexCompilationUnit();
      std::string method_name = PrettyMethod(
          compilation_unit.GetDexMethodIndex(),
          *compilation_unit.GetDexFile(), true);
      PRINT_PASS_OSTREAM_MESSAGE(pass_instance_,
                                 "Started devirtualization attempt for " << method_name);
    }
  }
  ~ScopedMessage() {
    if (pass_instance_->IsVerbose()) {
      const DexCompilationUnit& compilation_unit = pass_instance_->GetDexCompilationUnit();
      std::string method_name = PrettyMethod(
          compilation_unit.GetDexMethodIndex(),
          *compilation_unit.GetDexFile(), true);
      PRINT_PASS_OSTREAM_MESSAGE(pass_instance_,
                                 "Finished devirtualization attempt for " << method_name);
    }
  }

 private:
  HSpeculationPass* pass_instance_;
};

void HSpeculationPass::Run() {
  ScopedMessage entry_exit_marker(this);

  if (!Gate()) {
    return;
  }

  std::vector<HInstruction*> candidates;

  // Walk the graph to find candidates.
  for (HReversePostOrderIterator it(*graph_); !it.Done(); it.Advance()) {
    HBasicBlock* block = it.Current();
    for (HInstructionIterator inst_it(block->GetInstructions());
         !inst_it.Done(); inst_it.Advance()) {
      HInstruction* instr = inst_it.Current();
      if (IsCandidate(instr)) {
        RecordFoundCandidate();
        if (HasPrediction(instr, true)) {
          PRINT_PASS_OSTREAM_MESSAGE(this, "The following candidate has been found " << instr);
          // Fill vector with eligible candidates.
          candidates.push_back(instr);
        }
      }
    }
  }

  // If there are no candidates, no reason to continue with this pass.
  if (candidates.empty()) {
    return;
  }

  CandidatesMap candidates_grouped;
  GroupCandidatesWithOrdering(candidates_grouped, &candidates);

  // The new map should have at least one candidate.
  DCHECK(!candidates_grouped.empty());

  // Check map has no duplicates.
  if (kIsDebugBuild) {
    std::set<HInstruction*> candidates_set;
    for (auto it : candidates_grouped) {
      DCHECK(candidates_set.find(it.first) == candidates_set.end());
      candidates_set.insert(it.first);
      std::vector<HInstruction*>* similar_candidates = &it.second;
      for (auto it2 : *similar_candidates) {
        DCHECK(candidates_set.find(it2) == candidates_set.end());
        candidates_set.insert(it2);
      }
    }
  }

  HInstructionCloner cloner(GetGraphX86());

  // Now choose and apply the best strategy for each candidate.
  for (auto it : candidates_grouped) {
    HInstruction* candidate = it.first;
    std::vector<HInstruction*>* similar_candidates = &it.second;
    DCHECK(candidate != nullptr);

    SpeculationRecoveryApproach recovery = GetRecoveryMethod(candidate);

    if (recovery == kRecoveryAny) {
      // We pick code versioning because this is the least costly version
      // in case of failed speculation.
      recovery = kRecoveryCodeVersioning;
    } else if (recovery == kRecoveryCodeVersioningWithCounting) {
      // TODO Add support for counting - only useful when there is a framework that makes
      // use of it for purpose of reoptimization.
      recovery = kRecoveryCodeVersioning;
    }

    PRINT_PASS_OSTREAM_MESSAGE(this, "Trying to speculate for " << candidate <<
                               " using recovery mode " << recovery);

    if (IsPredictionWorthIt(candidate, recovery, similar_candidates)) {
      if (!SupportsRecoveryMethod(recovery, candidate)) {
        // Code versioning is always supported.
        recovery = kRecoveryCodeVersioning;
        DCHECK(SupportsRecoveryMethod(recovery, candidate));
      }

      if (kIsDebugBuild) {
        for (auto similar : *similar_candidates) {
          DCHECK(SupportsRecoveryMethod(recovery, similar));
        }
      }

      bool success = false;
      if (NeedsNoRecovery(recovery)) {
        // When recovery is not needed - we have nothing to do.
        success = true;
      } else if (NeedsDeopt(recovery)) {
        success = HandleDeopt(candidate);
      } else if (NeedsCodeVersioning(recovery)) {
        bool needs_trap = NeedsTrap(recovery);
        bool needs_counting = NeedsCounting(recovery);
        VersioningApproach versioning = GetVersioningApproach(candidate);
        success = HandleCodeVersioning(candidate, similar_candidates, cloner, needs_counting, needs_trap, versioning);
      } else {
        LOG(FATAL) << "Unhandled recovery mode for speculation: " << recovery;
      }

      if (success) {
        bool guard_inserted = !NeedsNoRecovery(recovery);
        size_t candidates_handled = 0;
        if (HandleSpeculation(candidate, cloner.GetClone(candidate), guard_inserted)) {
          if (guard_inserted) {
            PRINT_PASS_OSTREAM_MESSAGE(this, "Successfully speculated for " << candidate <<
                                       " via " << recovery);
          } else {
            PRINT_PASS_OSTREAM_MESSAGE(this, "Successfully speculated for " << candidate <<
                                       " with no guard.");
          }
          candidates_handled++;
        }

        for (auto similar : *similar_candidates) {
          if (HandleSpeculation(similar, cloner.GetClone(similar), guard_inserted)) {
            PRINT_PASS_OSTREAM_MESSAGE(this, "Successfully speculated for " << similar <<
                                       " by using guard used for " << candidate);
            candidates_handled++;
          }
        }

        if (candidates_handled != 0) {
          RecordSpeculation(candidates_handled);
          MaybeRecordStat(kIntelSpeculationEliminated, candidates_handled - 1);
        }
      }
    } else {
      PRINT_PASS_OSTREAM_MESSAGE(this, "Speculating is not worth it for " << candidate);
    }
  }
}

bool HSpeculationPass::IsPredictionWorthIt(HInstruction* instr,
                                           SpeculationRecoveryApproach recovery,
                                           std::vector<HInstruction*>* similar_candidates) {
  // Prediction is always positive if recovery is not needed.
  if (recovery == kRecoveryNotNeeded) {
    // It MUST be the case that there is no chance of mispredict if recovery is not needed.
    std::pair<uint64_t, uint64_t> t = GetMispredictRate(instr);
    DCHECK_EQ(t.first, 0u);
    return true;
  }

  double cost_of_recovery = GetCostOfRecoveryMethod(recovery);
  double cost = static_cast<double>(GetCost(instr));
  double profit = static_cast<double>(GetProfit(instr));
  if (similar_candidates != nullptr) {
    // The profit accumulates for each application.
    for (auto it : *similar_candidates) {
      profit += static_cast<double>(GetProfit(it));
    }
  }
  std::pair<uint64_t, uint64_t> t = GetMispredictRate(instr);
  double mispredict_rate = static_cast<double>(t.first) / static_cast<double>(t.second);
  double net_benefit = profit * (1.0 - mispredict_rate) - cost -
      static_cast<double>(cost_of_recovery) * mispredict_rate;
  return net_benefit > 0.0;
}

int32_t HSpeculationPass::GetCostOfRecoveryMethod(SpeculationRecoveryApproach recovery) {
  switch (recovery) {
    case kRecoveryNotNeeded:
    return 0;
    case kRecoveryAny:
    case kRecoveryFault:
    case kRecoveryCodeVersioning:
      return kCostCodeVersioning;
    case kRecoveryDeopt:
      return kCostOfDeopt;
    case kRecoveryCodeVersioningWithCounting:
      return kCostCodeVersioning + kCostToCount;
    default:
      UNIMPLEMENTED(FATAL)<< "Unknown cost of recovery method";
  }

  return 0;
}

}  // namespace art
