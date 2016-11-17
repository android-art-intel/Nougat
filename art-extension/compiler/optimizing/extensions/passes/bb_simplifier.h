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

#ifndef ART_OPT_PASSES_BB_SIMPLIFIER_H_
#define ART_OPT_PASSES_BB_SIMPLIFIER_H_

#include "optimization_x86.h"

namespace art {

/**
 * @brief Simplifies CFG, removing unneeded branching.
 */
class HBBSimplifier : public HOptimization_X86 {
 public:
  explicit HBBSimplifier(HGraph* graph, OptimizingCompilerStats* stats = nullptr)
    : HOptimization_X86(graph, "bb_simplifier", stats) {}

  void Run() OVERRIDE;

 private:
  /**
    * @brief Try to eliminate meaningless diamond starting on the block.
    * @param block The block to simplify.
    * @param if_insn The IF instruction of the diamond.
    * @return True, if the simplification succeeded.
    */
  bool TrySimplifyIf(HBasicBlock* block, HIf* if_insn);

  /**
    * @brief Try to simplify a construction starting in current bb.
    * @param block The block to simplify.
    * @return True, if the simplification succeeded.
    */
  bool TrySimplify(HBasicBlock* block);
};

}  // namespace art

#endif  // ART_OPT_PASSES_BB_SIMPLIFIER_H_
