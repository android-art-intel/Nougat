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

#ifndef ART_COMPILER_OPTIMIZING_EXTENSIONS_PASSES_PHI_CLEANUP_H_
#define ART_COMPILER_OPTIMIZING_EXTENSIONS_PASSES_PHI_CLEANUP_H_

#include "optimization_x86.h"

namespace art {

/**
 * @brief This is an optimization pass that removes some unneeded phis.
 */
class HPhiCleanup : public HOptimization_X86 {
 public:
  explicit HPhiCleanup(HGraph* graph,
                       const char* pass_name = kPhiCleanupPassName,
                       OptimizingCompilerStats* stats = nullptr)
    : HOptimization_X86(graph, pass_name, stats) {}

  void Run() OVERRIDE;

 private:
  static constexpr const char* kPhiCleanupPassName = "phi_cleanup";

  /**
    * @brief Checks whether the optimization should be applied.
    * @details In theory, we can apply it to any method. But to save
    *          the compile time, we limit the scope of Phi Cleanup to
    *          methods with loops only.
    * @return true, if Phi Cleanup should be applied.
    */
  bool Gate() const;

  /**
    * @brief Performs phi cleanup.
    */
  void CleanUpPhis();

  /**
   * @brief Checks whether a Phi has the same instruction on all its inputs.
   * @param phi The Phi to check.
   * @return true, if the Phi has the same instruction on all inputs.
   */
  bool AllInputsSame(HPhi* phi) const;

  /**
   * @brief Recursively checks that all users of the instruction are from the same clique.
   * @details We may consider whole use chain of a Phi a clique if none of the instructions
   *          in this chain has side effects, env or is a control flow. There can be other phis
   *          in this clique. The clique elements may be removed all together.
   * @param to_check The instruction to check.
   * @param seen_insns The set of Phis and instructions that we have already checked.
   * @param candidates The set of candidates for the same clique.
   * @return true, if phi has only users from the same clique.
   */
  bool RemoveCliqueHelper(HInstruction* to_check,
                          std::unordered_set<HInstruction*>& seen_insns,
                          std::unordered_set<HInstruction*>& candidates);

  /**
   * @brief Checks if a Phi and its clique may be removed and removes it,
   *        if possible.
   * @param phi The Phi to check.
   * @param seen_insns The set of Phis that we have already considered.
   * @return true, if the Phi and its clique were successfully removed.
   */
  bool RemoveClique(HPhi* phi, std::unordered_set<HInstruction*>& seen_insns);
};

}  // namespace art

#endif  // ART_COMPILER_OPTIMIZING_EXTENSIONS_PASSES_PHI_CLEANUP_H_
