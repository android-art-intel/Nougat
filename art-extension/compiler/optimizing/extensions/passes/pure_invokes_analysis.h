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
 */

#ifndef ART_COMPILER_OPTIMIZING_PURE_INVOKES_ANALYSIS_H_
#define ART_COMPILER_OPTIMIZING_PURE_INVOKES_ANALYSIS_H_

#include "driver/compiler_driver.h"
#include "optimization_x86.h"

namespace art {

class HPureInvokesAnalysis : public HOptimization_X86 {
 public:
  HPureInvokesAnalysis(HGraph* graph,
                       OptimizingCompilerStats* stats = nullptr)
  : HOptimization_X86(graph, kPureInvokesAnalysisPassName, stats) {}

  void Run() OVERRIDE;

  static constexpr const char* kPureInvokesAnalysisPassName = "pure_invokes_analysis";

 private:
  /**
    * @brief Stores info about method purity and its side effects.
    */
  struct MethodPurityInfo {
    const bool is_pure;
    const SideEffects se;
  };

  /**
   * @brief Finds candidates for pure invokes hoisting.
   * @details This method makes primary part of work. It emilinates pure
   *          invokes for which we have proved that it is legal, and also
   *          accumulates those that can potentially be hoisted to the vector.
   * @return true, if we found at least one candidate for hoisting.
   */
  void ProcessPureInvokes();

  /**
   * @brief Checks whether the result of the instuction can be null.
   * @param insn The instuction to check.
   * @return true, if we proved that insn cannot return null.
   */
  bool CanReturnNull(HInstruction* insn);

  /**
   * @brief Checks whether the call is an invoke of pure method.
   * @param call The call to check.
   * @return true, if the method in this call is pure.
   */
  bool IsPureMethodInvoke(HInvokeStaticOrDirect* call);

  /**
   * @brief Pessimistically checks whether the invoke can return null.
   * @param call The invoke.
   * @return false, if we proved that the call never returns null.
   *         true, otherwise.
   */
  bool IsInvokeThatCanReturnNull(HInvokeStaticOrDirect* call);

  // Method reference -> known answer for this method.
  SafeMap<const MethodReference, MethodPurityInfo, MethodReferenceComparator> pure_invokes_;
  SafeMap<const MethodReference, bool, MethodReferenceComparator> null_invokes_;

  // Presets of side effects of the invokes we consider pure.
  static const SideEffects se_none_;
  static const SideEffects se_memory_read_;
  static const SideEffects se_memory_read_and_alloc_;

  /**
    * @brief Specifies side effects of pure methods with given names.
    */
  struct PureMethodSignature {
    const char* method_name;
    const SideEffects se;
  };

  // The whitelist contains known pure methods.
  static PureMethodSignature whitelist_[];
  // The signatures of methods that never return null.
  static const char* never_returns_null_[];

  DISALLOW_COPY_AND_ASSIGN(HPureInvokesAnalysis);
};

}  // namespace art

#endif  // ART_COMPILER_OPTIMIZING_PURE_INVOKES_ANALYSIS_H_
