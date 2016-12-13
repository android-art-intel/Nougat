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

#ifndef ART_OPT_PASSES_COMMUTATIVE_TREES_FLIPPER_H_
#define ART_OPT_PASSES_COMMUTATIVE_TREES_FLIPPER_H_

#include "optimization_x86.h"

namespace art {

/**
 * @brief Flips trees of commutative operations to help the register allocation.
 * @details From the point of view of simple allocators, expression like
 *          (a + (b + (c + d))) may take more registers than (((a + b) + c) + d).
 *          This pass flips the binary operation to ensure the best estimation of
 *          abstract registers needed for calculation of expressions.
 */
class HCommutativeTreesFlipper : public HOptimization_X86 {
 public:
  explicit HCommutativeTreesFlipper(HGraph* graph,
                                    OptimizingCompilerStats* stats = nullptr)
    : HOptimization_X86(graph, kCommutativeTreesFlipperPassName, stats) {}

  static constexpr const char* kCommutativeTreesFlipperPassName = "commutative_trees_flipper";

  void Run() OVERRIDE;

 private:
  /**
   * @brief Estimates how many abstract regs is required for calculation of instruction.
   * @details In our heuristics, we assume that any non-binary operations takes 1 reg,
   *          any binary operation with args (left, right) takes max(cost(left), 1 + cost(right))
   *          abstract regs, because we need to store the result of left when calculating right.
   *          In case if it is profitable to flip operands of a binary operation to reduce its
   *          cost estimation, this function flips them and remembers the optimal cost.
   * @param insn The instruction that we want to estimate.
   * @return The optimal estimated number of abstract registers needed for calculation.
   */
  uint32_t EstimateCost(HInstruction* insn);

  /**
   * @brief Calculates cost of a binary operation with args (left, right)
   *        using formula max(cost(left), 1 + cost(right)).
   * @param left The 1st argument of the binary operation.
   * @param right The 2nd argument of the binary operation.
   * @return The estimation of number of regs for calculation 'as is'.
   */
  uint32_t CalculationCost(HInstruction* left, HInstruction* right);

  // Memorizes pre-calculated costs of binary operations.
  std::unordered_map<HBinaryOperation*, uint32_t> estimated_cost_;
};

}  // namespace art

#endif  // ART_OPT_PASSES_COMMUTATIVE_TREES_FLIPPER_H_
