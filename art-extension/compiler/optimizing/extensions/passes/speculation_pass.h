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

#ifndef ART_COMPILER_OPTIMIZING_EXTENSIONS_PASSES_SPECULATION_PASS_H_
#define ART_COMPILER_OPTIMIZING_EXTENSIONS_PASSES_SPECULATION_PASS_H_

#include "cloning.h"
#include "optimization_x86.h"
#include "speculation.h"

namespace art {

class CompilerDriver;
class DexCompilationUnit;

typedef std::map<HInstruction*, std::vector<HInstruction*>> CandidatesMap;

class HSpeculationPass : public HOptimization_X86 {
 public:
  HSpeculationPass(HGraph* graph,
                   const char* pass_name,
                   const DexCompilationUnit& compilation_unit,
                   CompilerDriver* compiler_driver,
                   OptimizingCompilerStats* stats = nullptr)
      : HOptimization_X86(graph, pass_name, stats),
        compiler_driver_(compiler_driver),
        compilation_unit_(compilation_unit) { }
  virtual ~HSpeculationPass() { }

  void Run() OVERRIDE;
  const DexCompilationUnit& GetDexCompilationUnit() {
    return compilation_unit_;
  }

 protected:
  /**
   * @brief Used to check whether this pass is applicable on graph.
   * @return Returns true if pass should be attempted and false otherwise.
   */
  virtual bool Gate() const;

  /**
   * @brief This is a lightweight check whether current instruction is a potential candidate.
   * @param instr The instruction to check.
   * @return Returns true if this instruction is a potential candidate.
   */
  virtual bool IsCandidate(HInstruction* instr) = 0;

  /**
   * @details This is a heavier-weight check whether current instruction has a prediction. It
   *  must be guaranteed that IsCandidate is a superset of this.
   * @see IsCandidate.
   * @param instr The instruction to check.
   * @param update Whether any internal data structure updates are allowed. This flag is passed
   * in because as part of this method we might calculate whether we have prediction. Since
   * we do not want to recalculate again, we may want to cache the result. If update is false,
   * we do not cache anything.
   * @return Returns true if the current instruction has prediction.
   */
  virtual bool HasPrediction(HInstruction* instr, bool update) = 0;

  /**
   * @details Provides the max cost of the added guard along with its dependencies.
   *  The general guidance is that this will provide a cost in terms of path length.
   *  Each pass can keep its own internal measure of cost as long as profit and cost
   *  calculations are consistent.
   * @see GetProfit.
   * @return Returns the maximum cost of the guard.
   */
  virtual uint64_t GetMaxCost() = 0;

  /**
   * @brief Gets the cost of doing a prediction on the current instruction.
   * @param instr The instruction to check.
   * @return Returns the cost of the guard.
   */
  virtual uint64_t GetCost(HInstruction* instr ATTRIBUTE_UNUSED) {
    return GetMaxCost();
  }

  /**
   * @brief Gets the rate of mispredict as a ratio.
   * @param instr The instruction to check.
   * @return Returns the mispredict rate as a ratio.
   */
  virtual std::pair<uint64_t, uint64_t> GetMispredictRate(HInstruction* instr ATTRIBUTE_UNUSED) {
    // Take assumption that we will always mispredict if this is not overridden.
    return std::make_pair(1, 1);
  }

  /**
   * @details Used to get the profit of doing the optimization. It must be the case that
   *  the cost is consistent in unit of measure as this method.
   * @see GetCost.
   * @param instr The instruction to check.
   * @return Returns the profit of speculating on current instruction.
   */
  virtual uint64_t GetProfit(HInstruction* instr ATTRIBUTE_UNUSED) {
    // The sane default is that this speculation does not bring any profit.
    return 0;
  }

  /**
   * @brief Used to obtain the preferred recovery method for the current instruction.
   * @details Passes may want different recovery methods depending on where in graph
   *  the instruction is. Additionally, passes are free to always request same recovery
   *  approach every time.
   * @param instr The instruction to check.
   * @return Returns the preferred recovery method.
   */
  virtual SpeculationRecoveryApproach GetRecoveryMethod(HInstruction* instr ATTRIBUTE_UNUSED) {
    // Although some optimizations may prefer a recovery method, we should simply
    // let the speculation pass decide which method to use based on benefit.
    return kRecoveryAny;
  }

  /**
   * @brief Used to obtain the preferred versioning approach for the current instruction.
   * @details Passes may want different versioning methods depending on where in graph
   *  the instruction is. Additionally, passes are free to always request same versioning
   *  approach every time.
   * @param instr The instruction to check.
   * @return Returns the preferred versioning method.
   */
  virtual VersioningApproach GetVersioningApproach(HInstruction* instr ATTRIBUTE_UNUSED) {
    return kVersioningAny;
  }

  /**
   * @brief Checks whether speculating on the given instruction supports recovery method.
   * @details This method exists because the speculation pass might decide it is more
   * worth it to pick another recovery scheme. However, we want to make sure that if
   * another scheme is chosen, it is actually supported.
   * @param recovery The recovery methodology recommended.
   * @param instr The instruction to check.
   */
  virtual bool SupportsRecoveryMethod(SpeculationRecoveryApproach recovery, HInstruction* instr) {
    DCHECK(instr != nullptr);
    if (recovery == kRecoveryDeopt) {
      // To do deoptimization, we must have an environment so we can transition to interpreter.
      return instr->HasEnvironment();
    }

    return true;
  }

  /**
   * @brief Used to check whether doing prediction here is worth it.
   * @details What we essentially check is that:
   *  profit * correct_predict_rate - cost - cost_of_chosen_recovery_path * mispredict_rate >= 0
   * @param instr The instruction to check.
   * @param recovery The recovery schemes planned to be used.
   * @param similar_candidates A vector of similar candidates that can use same prediction.
   */
  virtual bool IsPredictionWorthIt(HInstruction* instr,
                                   SpeculationRecoveryApproach recovery,
                                   std::vector<HInstruction*>* similar_candidates = nullptr);

  /**
   * @brief Used to determine whether two candidates might be able to use
   * same prediction.
   * @param instr The first instruction to compare.
   * @param instr2 The second instruction to compare.
   * @return Returns true if both can use same prediction.
   */
  virtual bool IsPredictionSame(HInstruction* instr, HInstruction* instr2) = 0;

  /**
   * @brief Used to record that speculative optimization was applied.
   * @param count The count of application - usually 1.
   */
  virtual void RecordSpeculation(size_t count = 1u) = 0;

  /**
   * @brief Used to record when a potential candidate is found.
   * @param count The count of found candidates - usually 1.
   */
  virtual void RecordFoundCandidate(size_t count = 1u) = 0;

  /**
   * @brief Used to get the cost of the recovery methodology.
   * @param recovery The recovery method.
   * @return Returns the cost of recovery.
   */
  int32_t GetCostOfRecoveryMethod(SpeculationRecoveryApproach recovery);

  /**
   * @brief Used to insert the speculation guard in the graph.
   * @param instr_guarded The instruction for which the guard is being created for.
   * @param instr_cursor The cursor before which the guard will be inserted.
   * @return Returns the guard if successfully inserted and null otherwise.
   */
  virtual HSpeculationGuard* InsertSpeculationGuard(HInstruction* instr_guarded,
                                                    HInstruction* instr_cursor) = 0;

  /**
   * @brief Used after speculation guard is inserted to actually cause speculation
   * replacement/motion to happen.
   * @param instr The instruction being speculated.
   * @param instr_copy The copy of instruction in the case where code versioning was used.
   * Can be null.
   * @param guard_inserted Whether this speculation was done via guard insertion.
   * @return Returns true if the speculation was properly generated.
   */
  virtual bool HandleSpeculation(HInstruction* instr,
                                 HInstruction* instr_copy,
                                 bool guard_inserted) = 0;

  // For deopt, put a high cost because we resume with interpreter.
  static constexpr uint32_t kCostOfDeopt = 100u;
  // Cost of 1 since we can do a single memory increment.
  static constexpr uint32_t kCostToCount = 1u;
  // When code versioning is done, it does not increase critical path. It is the same code
  // as before the speculation.
  static constexpr uint32_t kCostCodeVersioning = 0u;

  // Protected because it might be used in subclass.
  CompilerDriver* const compiler_driver_;
  const DexCompilationUnit& compilation_unit_;
  bool no_ordering_;

 private:
  /**
   * @brief Used to create groupings of similar candidates.
   * This function guarantee all candidates will be sorted by dependency
   * order.
   * @param candidates_grouped Map to put grouped instructions.
   * @param candidates The vector of HInstructions to be grouped.
   */
  void GroupCandidatesWithOrdering(CandidatesMap& candidates_grouped,
                       const std::vector<HInstruction*>* candidates);

  /**
   * @brief Used as helper to create deopt speculation technique.
   * @param candidate The instruction being speculated with deopt.
   * @return Returns true if successful.
   */
  bool HandleDeopt(HInstruction* candidate);

  /**
   * @brief Used as helper to create code versioning speculation technique.
   * @param candidate The instruction being speculated with deopt.
   * @param similar_candidates A vector of similar candidates that can use same prediction.
   * @param cloner The instruction cloner that can be used for duplication.
   * @param with_counting Whether counting of misspeculation should be done.
   * @param with_trap Whether the slow path should be a trap.
   * @param versioning The versioning approach to use.
   * @return Returns true if successful.
   */
  bool HandleCodeVersioning(HInstruction* candidate,
                            std::vector<HInstruction*>* similar_candidates,
                            HInstructionCloner& cloner,
                            bool with_counting = false,
                            bool with_trap = false, VersioningApproach versioning = kVersioningAny);

  /**
   * @brief Used as helper to create code versioning speculation technique.
   * @param instr The instruction being speculated with deopt.
   * @param cloner The instruction cloner that can be used for duplication.
   * @param guard The speculation guard instruction.
   * @param needs_trap Whether the slow path should be a trap.
   */
  void CodeVersioningHelper(HInstruction* instr,
                            HInstructionCloner& cloner,
                            HSpeculationGuard* guard,
                            bool needs_trap);

  DISALLOW_COPY_AND_ASSIGN(HSpeculationPass);
};

}  // namespace art

#endif  // ART_COMPILER_OPTIMIZING_EXTENSIONS_PASSES_SPECULATION_PASS_H_
