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

#include "commutative_trees_flipper.h"
#include "ext_utility.h"

namespace art {

uint32_t HCommutativeTreesFlipper::EstimateCost(HInstruction* insn) {
  DCHECK(insn != nullptr);
  auto op = insn->AsBinaryOperation();

  if (op == nullptr) {
    // Roughly assume that non-binary operations require 1 VR for operation.
    return 1u;
  }

  auto known = estimated_cost_.find(op);
  if (known != estimated_cost_.end()) {
    // We already know the cost of this operation.
    return known->second;
  }

  auto left = op->InputAt(0);
  auto right = op->InputAt(1);

  if (!op->IsCommutative()) {
    // Non-commutative operations cannot be flipped.
    auto cost = CalculationCost(left, right);
    estimated_cost_[op] = cost;
    return cost;
  } else {
    // There is a special case when we know that placing
    // Phi on left position is benefitable. It is:
    //   phi = Phi(a, b);
    //     ...
    //   b = x + phi
    // Here we know that phi is (most likely) in a register,
    // so making b like
    //   b = phi + x
    // may use this advantage.
    if (!left->IsPhi() && right->IsPhi()) {
      HPhi* phi = right->AsPhi();
      DCHECK(phi != nullptr);
      if (phi->GetBlock()->IsLoopHeader()
          && phi->InputAt(1) == op) {
        PRINT_PASS_OSTREAM_MESSAGE(this, "Flip " << op << " because of loop Phi pattern");
        auto cost = EstimateCost(left);
        op->ReplaceInput(right, 0);
        op->ReplaceInput(left, 1);
        MaybeRecordStat(kIntelCommutativeOperationFlipped);
        estimated_cost_[op] = cost;
        return cost;
      }
    }

    // We can try to flip the args.
    auto no_flip_cost = CalculationCost(left, right);
    auto flip_cost = CalculationCost(right, left);
    // Flip unless either of args is a Phi; be conservative for Phis.
    if (flip_cost < no_flip_cost
        && !left->IsPhi()
        && !right->IsPhi()) {
      PRINT_PASS_OSTREAM_MESSAGE(this, "Flip " << op << " (no flip cost = " << no_flip_cost
                                               << ", flip cost = " << flip_cost << ")");
      // Should never place a constant into the left position.
      DCHECK(!right->IsConstant());
      op->ReplaceInput(right, 0);
      op->ReplaceInput(left, 1);
      MaybeRecordStat(kIntelCommutativeOperationFlipped);
      estimated_cost_[op] = flip_cost;
      return flip_cost;
    } else {
      estimated_cost_[op] = no_flip_cost;
      return no_flip_cost;
    }
  }
}

uint32_t HCommutativeTreesFlipper::CalculationCost(HInstruction* left, HInstruction* right) {
  return std::max(EstimateCost(left), 1 + EstimateCost(right));
}

void HCommutativeTreesFlipper::Run() {
  auto graph = GetGraphX86();

  PRINT_PASS_OSTREAM_MESSAGE(this, "Start " << GetMethodName(graph));

  for (HReversePostOrderIterator b(*graph); !b.Done(); b.Advance()) {
    HBasicBlock* block = b.Current();
    for (HInstructionIterator i(block->GetInstructions()); !i.Done(); i.Advance()) {
      EstimateCost(i.Current());
    }
  }

  PRINT_PASS_OSTREAM_MESSAGE(this, "End " << GetMethodName(graph));
  estimated_cost_.clear();
}

}  // namespace art
