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

#include "constant_calculation_sinking.h"
#include "ext_utility.h"
#include "loop_iterators.h"

namespace art {

/*
 * @brief Delete phi and all its users.
 * @param phi Phi node to delete.
 */
void HConstantCalculationSinking::DeletePhiAndUsers(HPhi* phi) const {
  DCHECK(phi != nullptr);

  std::vector<HInstruction*> to_remove;

  // Add all phi users to list.
  const HUseList<HInstruction*>& uses = phi->GetUses();
  for (auto it = uses.begin(), end = uses.end(); it != end; ++it) {
    HInstruction* user = it->GetUser();
    if (phi != user) {
      to_remove.push_back(user);
    }
  }

  // Remove phi.
  RemoveAsUser(phi);
  RemoveFromEnvironmentUsers(phi);
  phi->GetBlock()->RemovePhi(phi, false);

  // Remove all phi users.
  std::vector<HInstruction*>::const_iterator it;
  for (it = to_remove.begin(); it != to_remove.end(); it++) {
    HInstruction* insn = *it;
    HBasicBlock* block = insn->GetBlock();
    DCHECK(insn != nullptr);
    RemoveAsUser(insn);
    RemoveFromEnvironmentUsers(insn);
    block->RemoveInstruction(insn, false);
  }
}

/*
 * @brief check the instruction has no users over the loop.
 * @param candidate instruction we want to check.
 * @param loop_info loop we're working on.
 * @param phi phi node relates to this instruction.
 * @return returns true if instruction has no users in the loop.
 */
bool HConstantCalculationSinking::HasNoDependenciesWithinLoop(HInstruction* candidate,
         HLoopInformation_X86* loop_info, HInstruction* phi) const {
  const HUseList<HInstruction*>& uses = candidate->GetUses();
  for (auto it = uses.begin(), end = uses.end(); it != end; ++it) {
    HInstruction* user = it->GetUser();

    if (user != phi && loop_info->Contains(*user->GetBlock())) {
      return false;
    }
  }

  // Now we want to ensure that our candidate or phi is not included
  // in environment of in-loop instruction which can throw and catcher
  // inside of our method uses the VR corresponding to our Phi
  // or instruction. Otherwise it means that we will eliminate the VR
  // change inside the loop (and correct it after) but we can jump
  // to catch handler which needs this VR up to date.
  if (loop_info->CheckForCatchBlockUsage(candidate) ||
      loop_info->CheckForCatchBlockUsage(phi)) {
    PRINT_PASS_MESSAGE(this, "Phi %d or Instruction %d has VR uses in catch block",
                       phi->GetId(), candidate->GetId());
    return false;
  }

  return true;
}

/*
 * @brief Fill accumulator_list with only constants able to be sunk.
 * @param loop_info loop we're working on.
 * @param choosen_iv the loop's main basic IV that should be ignored.
 * @param accumulator_list the calculation of the accumulators to be considered by the pass.
 */
void HConstantCalculationSinking::FillAccumulator(HLoopInformation_X86* loop_info,
         std::vector<AccumulatorAssociation>& accumulator_list) const {
  // Determine if it is a linear function.
  // Check it has no dependencies.
  // Add to the accumulator_list.

  // Go through each phi.
  for (HInstructionIterator it(loop_info->GetHeader()->GetPhis()); !it.Done(); it.Advance()) {
    AccumulatorAssociation accum_assoc;

    HPhi* phi = it.Current()->AsPhi();

    size_t input_count = phi->InputCount();
    // For now accept only PHI nodes that have two uses and one define.
    if (input_count != 2) {
      continue;
    }

    HInstruction* candidate = nullptr;

    // Check phi has only one user in loop and save the user found.
    uint32_t phi_users_in_loop = 0;
    bool phi_has_outside_loop_uses = false;
    const HUseList<HInstruction*>& uses = phi->GetUses();
    for (auto it2 = uses.begin(), end2 = uses.end();
         it2 != end2  && (phi_users_in_loop < 2) ; ++it2) {
      HInstruction* user = it2->GetUser();

      if (!loop_info->Contains(*(user->GetBlock()))) {
        phi_has_outside_loop_uses = true;
      } else {
        phi_users_in_loop++;
        if (loop_info->ExecutedPerIteration(user)) {
          candidate = user;
        }
      }
    }
    if (phi_users_in_loop != 1) {
      continue;
    }

    // We must ensure that candidate is actually input of our Phi node.
    // We should compare against the second input because the first one
    // comes from pre-header always. We also checked the Phi has only two inputs.
    if (candidate == nullptr || candidate != phi->InputAt(1)) {
      continue;
    }

    // Check selected user has no users in the current loop.
    if (!HasNoDependenciesWithinLoop(candidate, loop_info, phi)) {
      continue;
    }

    // Check operation is supported.
    switch (candidate->GetKind()) {
      case HInstruction::kAdd:
      case HInstruction::kMul:
      case HInstruction::kDiv:
      case HInstruction::kSub:
      case HInstruction::kRem:
        break;
      default:
        continue;
    }

    // Get the constant the IV increments on.
    HBinaryOperation* binary = candidate->AsBinaryOperation();
    DCHECK(binary != nullptr);
    HConstant* constant_candidate = binary->GetConstantRight();
    if (constant_candidate == nullptr) {
      continue;
    }

    // Get the initial constant instruction.
    HInstruction* initial_constant_candidate = nullptr;
    initial_constant_candidate = (phi->InputAt(0) == candidate) ?
      phi->InputAt(1) : phi->InputAt(0);
    DCHECK(initial_constant_candidate != nullptr);
    if (!initial_constant_candidate->IsConstant()) {
      if (constant_candidate->IsIntConstant() || constant_candidate->IsLongConstant() ||
          constant_candidate->IsFloatConstant() || constant_candidate->IsDoubleConstant()) {
        initial_constant_candidate = nullptr;
      } else {
        continue;
      }
    }

    // Someone will use Phi node outside the loop, we should check whether
    // accumulation has the same value as Phi node because we will use
    // accumulation as replacment for Phi node.
    if (phi_has_outside_loop_uses) {
      // If accumulation is used between phi node (header) and if block
      // then they differ.
      HBasicBlock* exit_block = loop_info->GetExitBlock(true);
      DCHECK(exit_block != nullptr);
      if (candidate->GetBlock()->Dominates(exit_block)) {
        continue;
      }
    }

    // Fill the AccumulatorAssociation structure with data received and
    // push it into the list.
    accum_assoc.constant = constant_candidate;
    accum_assoc.linear_instruction = candidate;
    accum_assoc.phi = phi;
    accum_assoc.initial_constant = initial_constant_candidate;

    accumulator_list.push_back(accum_assoc);

    PRINT_PASS_MESSAGE(this, "Reg %d has been added to the sink list", phi->GetId());
  }
}

/**
 * @brief Find the original value of a variable before entering a loop.
 * @param accum_assoc the AccumulatorAssociation for the calculation.
 * @param seen_base_twice if the base mir have seen by twice.
 * @return if the original value was found.
 *
 * @details This method is responsible for finding the initial value of
 *  the variable var in the code below.
 *
 * float var = 42.0;
 * for (int i = 0; i < 42; i++) {
 *   // We are looking from here and want to find 42.0 as the initial value.
 *   var += 2.0f;
 * }.
 */
bool HConstantCalculationSinking::FindOrigin(
        AccumulatorAssociation& accum_assoc, bool& seen_base_twice) const {
  std::set<HInstruction*> visited;
  std::queue<HInstruction*> still_to_do;
  HInstruction* candidate = accum_assoc.initial_constant;
  HInstruction* elem = nullptr;
  HInstruction* base_phi = accum_assoc.phi;
  HInstruction* base = accum_assoc.linear_instruction;
  HBasicBlock* base_bb = base->GetBlock();

  if (base_bb == nullptr) {
    return false;
  }
  // If we have the constant operand ready, no need to search for it again.
  if (candidate != nullptr && candidate->IsConstant()) {
    return candidate->IsFloatConstant() || candidate->IsDoubleConstant();
  }

  // Mark base as not met yet.
  bool seen_base = false;
  seen_base_twice = false;

  // Push base.
  still_to_do.push(base_phi);

  // Continue until we have the list empty.
  while (still_to_do.empty() == false) {
    // Retrieve and remove the front of the queue.
    elem = still_to_do.front();
    still_to_do.pop();

    if (elem == nullptr) {
      continue;
    }

    // Skip if visited already.
    if (visited.find(elem) != visited.end()) {
      continue;
    }
    // when we see the accumulation.
    if (elem == base) {
      // It is an accumulation, so we must see the base only once.
      if (seen_base == false) {
        seen_base = true;
        continue;
      } else {
        // We see it second time, so most probably there is an outer
        // loop and our use here is actually a merged of possible
        // constant and value computed on the previous iteration.
        // So finally it is not a constant.
        seen_base_twice = true;
        continue;
      }
    }

    // Add to visited before doing anything else.
    visited.insert(elem);

    // If it is a phi, push all uses in the queue.
    if (elem->IsPhi()) {
      HPhi* phi_elem = elem->AsPhi();
      for (size_t i = 0; i < phi_elem->InputCount(); i++) {
        HInstruction* insn = phi_elem->InputAt(i);
        still_to_do.push(insn);
        if (visited.find(insn) != visited.end() && insn->IsPhi() && (insn == base_phi)) {
            seen_base_twice = true;
        }
      }
    } else {
      // We found something that is not a phi-node.
      // Did we already find something before?
      if (candidate != nullptr) {
        return false;
      } else {
        HBasicBlock* elem_bb = elem->GetBlock();

        // We found a candidate, does it actually dominate the base?
        if (!elem_bb->Dominates(base_bb)) {
            return false;
        }
        // Now set the candidate.
        candidate = elem;
      }
    }
  }
  if (candidate != nullptr && (candidate->IsFloatConstant() || candidate->IsDoubleConstant())) {
    accum_assoc.initial_constant = candidate;
    return true;
  } else {
    return false;
  }
}

/**
 * @brief Evaluate an float operation.
 * @param instruction_kind the type of calculation we need to perform.
 * @param iterations the number of iterations for the considered loop.
 * @param accum_assoc the AccumulatorAssociation for the calculation.
 * @param is_double is the integer calculation in double type.
 * @param seen_base_twice if the base mir was seen twice during walk up.
 * @return Returns true if calculation is handled.
 */
bool HConstantCalculationSinking::EvaluateFloatOperation(
        const HInstruction::InstructionKind& instruction_kind,
        int64_t iterations, AccumulatorAssociation& accum_assoc,
        bool is_double, bool seen_base_twice) const {
  /*
   * For floating-point calculations, we need to simulate it at compile time.
   *   This method just has two versions depending if we are in double mode or
   *     float mode.
   * At the end, we check if it was deemed safe.
   */
  int64_t max = kMaximumEvaluationIterationsForFP;

  // Set the right number of iterations.
  max = (max > iterations) ? iterations : max;

  if (max <= 0) {
    return false;
  }

  bool overflowed = false;

  if (is_double) {
    // In double, we need to get the original value as well.
    double value = accum_assoc.initial_constant->AsDoubleConstant()->GetValue();
    const double operand_value = accum_assoc.constant->AsDoubleConstant()->GetValue();

    // Handle the remainder separately.
    if (instruction_kind == HInstruction::kRem) {
      accum_assoc.const_instruction = graph_->GetDoubleConstant(operand_value);
      return true;
    } else {
      // For all others, we must do a loop.
      for (int i = 0; i < max; i++) {
        switch (instruction_kind) {
          case HInstruction::kDiv:
            value /= operand_value;
            break;
          case HInstruction::kMul:
            value *= operand_value;
            break;
          case HInstruction::kAdd:
            value += operand_value;
            break;
          case HInstruction::kSub:
            value -= operand_value;
            break;
          default:
            DCHECK(false);
            break;
        }
        if (isinf(value) || value == 0.0) {
          if (value != 0.0) {
            overflowed = true;
          }
          // For Mul and Div we should be careful in terms of sign.
          if (instruction_kind == HInstruction::kMul || instruction_kind == HInstruction::kDiv) {
            if (signbit(operand_value)) {
              // Do not allow constant sinking for the accumulation case where value
              // computed from the previous iteration of outer loop is used when the operand
              // value is negative due to the terms of sign depends on the iteration numbers
              // of outer loop.
              if (seen_base_twice == true) {
                return false;
              }
              const int remaining_iteration = iterations - i - 1;
              if ((remaining_iteration % 2) != 0) {
                value = -value;
              }
            }
            break;
          } else {
            if (value != 0.0) {
              break;
            }
          }
        }
      }
    }
    accum_assoc.const_instruction = graph_->GetDoubleConstant(value);
    accum_assoc.is_zero = (value == 0.0);
    accum_assoc.is_inf = isinf(value);
  } else {
    // In float, we need to get the original value as well.
    float value = accum_assoc.initial_constant->AsFloatConstant()->GetValue();
    const float operand_value = accum_assoc.constant->AsFloatConstant()->GetValue();

    // Handle the remainder separately.
    if (instruction_kind == HInstruction::kRem) {
      accum_assoc.const_instruction = graph_->GetFloatConstant(operand_value);
      return true;
    } else {
      // For all others, we must do a loop.
      for (int i = 0; i < max; i++) {
        switch (instruction_kind) {
          case HInstruction::kDiv:
            value /= operand_value;
            break;
          case HInstruction::kMul:
            value *= operand_value;
            break;
          case HInstruction::kAdd:
            value += operand_value;
            break;
          case HInstruction::kSub:
            value -= operand_value;
            break;
          default:
            DCHECK(false);
            break;
        }
        if (isinf(value) || value == 0.0f) {
          if (value != 0.0f) {
            overflowed = true;
          }
          // For Mul and Div we should be careful in terms of sign.
          if (instruction_kind == HInstruction::kMul || instruction_kind == HInstruction::kDiv) {
            if (signbit(operand_value)) {
              // Do not allow constant sinking for the accumulation case where value
              // computed from the previous iteration of outer loop is used when the operand
              // value is negative due to the terms of sign depends on the iteration numbers
              // of outer loop.
              if (seen_base_twice == true) {
                return false;
              }
              const int remaining_iteration = iterations - i - 1;
              if ((remaining_iteration % 2) != 0) {
                value = -value;
              }
            }
            break;
          } else {
            if (value != 0.0f) {
              break;
            }
          }
        }
      }
    }
    accum_assoc.const_instruction = graph_->GetFloatConstant(value);
    accum_assoc.is_zero = (value == 0.0f);
    accum_assoc.is_inf = isinf(value);
  }

  // Calculation is safe if:
  //  - It was a remainder.
  //  - It overflowed (went to infinity).
  //  - The calculation is now nil (for multiplication and division).
  //  - We executed all the iterations.
  bool is_safe = (instruction_kind == HInstruction::kRem) || overflowed ||
                 (((instruction_kind == HInstruction::kMul) ||
                   (instruction_kind == HInstruction::kDiv)) && accum_assoc.is_zero) ||
                 iterations == max;

  // If it is safe, it is because the calculation is finalized.
  if (is_safe) {
    accum_assoc.finalized_calculation = true;
  }
  return is_safe;
}

/**
 * @brief Is the calculation safe when considering floating point?
 * @param instruction_kind the type of calculation we need to perform.
 * @param accum_assoc the AccumulatorAssociation for the calculation.
 * @return whether or not the calculation is deemed safe for floating-point.
 */
bool HConstantCalculationSinking::SafeForFloatOperation(
        const HInstruction::InstructionKind& instruction_kind,
        AccumulatorAssociation& accum_assoc) const {
  // It is safe for remainder.
  if (instruction_kind == HInstruction::kRem) {
    return true;
  }

  // If it is 0, for multiplication/division it is safe.
  if (accum_assoc.is_zero &&
      (instruction_kind == HInstruction::kMul || instruction_kind == HInstruction::kDiv)) {
    return true;
  }

  // Is it infinity? If so, it is also safe.
  if (accum_assoc.is_inf) {
    return true;
  }

  // Otherwise, is it in a nested loop?
  // We don't want to try to look at if it is in a nested loop,
  // That could make this a non-legal transformation.
  HInstruction* inst = accum_assoc.linear_instruction;
  HBasicBlock* block = inst->GetBlock();
  if (block == nullptr) {
    return false;
  }
  HLoopInformation_X86* loop_info = LOOPINFO_TO_LOOPINFO_X86(block->GetLoopInformation());
  return (loop_info != nullptr) && (loop_info->GetParent() == nullptr);
}

/**
 * @brief Handle the calculation.
 * @param instruction_kind the type of calculation we need to perform.
 * @param iterations the number of iterations for the considered loop.
 * @param accum_assoc the AccumulatorAssociation for the calculation.
 * @return Returns true if calculation is handled.
 */
bool HConstantCalculationSinking::HandleCalculation(
        const HInstruction::InstructionKind& instruction_kind,
        int64_t iterations,
        AccumulatorAssociation& accum_assoc) const {
  if ((accum_assoc.constant->IsIntConstant() ||
       accum_assoc.constant->IsLongConstant())) {
    return EvaluateIntegerOperation(instruction_kind, iterations, accum_assoc);
  }
  if ((accum_assoc.constant->IsFloatConstant() ||
       accum_assoc.constant->IsDoubleConstant())) {
    bool is_double = (accum_assoc.constant->IsDoubleConstant());
    bool seen_base_twice = false;

    // For floating point calculations, we require a bit more information: the initial value.
    bool success = FindOrigin(accum_assoc, seen_base_twice);

    // If we failed, then bail here.
    if (success == false) {
      return false;
    }
    success = EvaluateFloatOperation(instruction_kind, iterations, accum_assoc, is_double, seen_base_twice);
    // If we failed, then bail here.
    if (success == false) {
      return false;
    }

    // Final step before accepting this: is this loop nested and did we get something else than infinity?
    //    If so, there is a risk this is reused and that makes it not safe.
    return SafeForFloatOperation(instruction_kind, accum_assoc);
  }
  return false;
}

/**
 * @brief Evaluate an integer operation.
 * @param instruction_kind the type of calculation we need to perform.
 * @param iterations the number of iterations for the considered loop.
 * @param accum_assoc the AccumulatorAssociation for the calculation.
 * @return Returns true if calculation is handled.
 */
bool HConstantCalculationSinking::EvaluateIntegerOperation(
        const HInstruction::InstructionKind& instruction_kind,
        int64_t iterations,
        AccumulatorAssociation& accum_assoc) const {
  /*
   * Entry point to handle addition, subtraction, multiplication, and division.
   *  For the first two, it is easy, return the operand * iterations.
   *  For the other two, see below.
   *
   * The whole point of this code is actually to calculate the power value and see
   *  if it would overflow or not.
   *
   * Now if it overflows to 0, that is great actually:
   *   For division, it means that the result would be 0 because we divided by
   *     something greater than our maximum possible value.
   *   For multiplication, it means that the result would also be 0.
   *
   * If it overflows not to 0, it means:
   *   For division, the result is still 0, we are dividing by something really big.
   *   For multiplication, we are fine, we just keep the lower bits.
   *
   * Note: if the operand is even, this will always overflow after 64 iterations
   *   for 64 bits because:
   *
   *   Suppose we have X, an even number:  X = Y * 2
   *   Which means: X ^ iterations = (Y * 2) ^ iterations
   *                               = (Y ^ iterations) * (2 ^ iterations).
   *
   *   Because we have 2 at a power, we will be effectively be shifting by that amount.
   *   Thus on 64-bit, it will take at most 64 iterations to have overflowed to 0.
   */
  accum_assoc.const_instruction = nullptr;
  union constant_operand {
    int32_t constant_operand_32;
    int64_t constant_operand_64;
  } constant_operand;

  union result {
    int32_t result_32;
    int64_t result_64;
  } result;

  bool is_long = false;

  switch (accum_assoc.constant->GetKind()) {
    case HInstruction::kIntConstant:
      constant_operand.constant_operand_32 = accum_assoc.constant->AsIntConstant()->GetValue();
      break;
    case HInstruction::kLongConstant:
      is_long = true;
      constant_operand.constant_operand_64 = accum_assoc.constant->AsLongConstant()->GetValue();
      break;
    default:
      return false;
  }

  switch (instruction_kind) {
    case HInstruction::kAdd:
    case HInstruction::kSub:
      if (!is_long) {
        result.result_32 = iterations * constant_operand.constant_operand_32;
        accum_assoc.const_instruction = graph_->GetIntConstant(result.result_32);
        return true;
      } else {
        result.result_64 = iterations * constant_operand.constant_operand_64;
        accum_assoc.const_instruction = graph_->GetLongConstant(result.result_64);
        return true;
      }
    case HInstruction::kRem:
      if (!is_long) {
        result.result_32 = constant_operand.constant_operand_32;
        if (result.result_32 != 0) {
          accum_assoc.const_instruction = graph_->GetIntConstant(result.result_32);
          return true;
        }
      } else {
        result.result_64 = constant_operand.constant_operand_64;
        if (result.result_64 != 0) {
          accum_assoc.const_instruction = graph_->GetLongConstant(result.result_64);
          return true;
        }
      }
      break;
    default:
      break;
  }

  constexpr int32_t int32_min_value = std::numeric_limits<int32_t>::min();
  constexpr int32_t int32_max_value = std::numeric_limits<int32_t>::max();
  constexpr int64_t int64_min_value = std::numeric_limits<int64_t>::min();
  constexpr int64_t int64_max_value = std::numeric_limits<int64_t>::max();
  int64_t max = kMaximumEvaluationIterations;

  // Set the right number of iterations.
  max = std::min(max, iterations);
  if (max <= 0) {
    return false;
  }

  // Set calculation value to 1 (fake initial value and tested against 0 later).
  if (is_long) {
    result.result_64 = 1;
  } else {
    result.result_32 = 1;
  }
  bool overflowed = false;

  // We need to know how many iterations can we make with our value of operand
  // so that we don't exceed representable range.
  int32_t allowed_iterations = 0;

  if ((!is_long && constant_operand.constant_operand_32 == int32_min_value) ||
      (is_long && constant_operand.constant_operand_64 == int64_min_value)) {
    // We reach the minimum possible value for 1 iteration.
    allowed_iterations = 1;
  } else {
    int64_t abs_operand;
    if (is_long) {
      abs_operand = std::abs(constant_operand.constant_operand_64);
    } else {
      abs_operand = std::abs(constant_operand.constant_operand_32);
    }

    if (abs_operand == 0 || abs_operand == 1) {
      // We can know result without doing any iterations.
      allowed_iterations = iterations;
    } else {
      // For all others we need to evaluate allowed iteration count carefully.
      int64_t max_representable = (is_long ? int64_max_value : int32_max_value);

      while (max_representable >= abs_operand) {
        allowed_iterations++;
        max_representable /= abs_operand;
      }

      // When operand is negative, we can possibly reach min value (abs of which is
      // greater than max value). Check only even iter counts (to ensure that after
      // next iteration result is negative and avoid unneeded checks).
      // In this case we should possibly allow one more iteration.
      if (is_long) {
        if ((constant_operand.constant_operand_64 < 0) && (allowed_iterations % 2 == 0)) {
          int64_t temp = 1;
          for (int32_t i = 0; i < allowed_iterations; i++) {
            temp *= constant_operand.constant_operand_64;
          }
          if (temp * constant_operand.constant_operand_64 == int64_min_value) {
            allowed_iterations++;
          }
        }
      } else {
        if ((constant_operand.constant_operand_32 < 0) && (allowed_iterations % 2 == 0)) {
          int32_t temp = 1;
          for (int32_t i = 0; i < allowed_iterations; i++) {
            temp *= constant_operand.constant_operand_32;
          }
          if (temp * constant_operand.constant_operand_32 == int32_min_value) {
            allowed_iterations++;
          }
        }
      }
    }
  }

  if (allowed_iterations < max) {
    overflowed = true;
  }

  // We still want to calculate where the overflow is going to,
  // in case it goes to zero.
  if (is_long) {
    int64_t value = 1;
    if (constant_operand.constant_operand_64 == 0) {
      value = 0;
    } else if (constant_operand.constant_operand_64 == 1) {
      value = 1;
    } else if (constant_operand.constant_operand_64 == -1) {
      value = (iterations % 2 == 0) ? value : -value;
    } else {
      for (int64_t i = 0; i < max; i++) {
        value *= constant_operand.constant_operand_64;
      }
    }
    result.result_64 = value;
  } else {
    int32_t value = 1;
    if (constant_operand.constant_operand_32 == 0) {
      value = 0;
    } else if (constant_operand.constant_operand_32 == 1) {
      value = 1;
    } else if (constant_operand.constant_operand_32 == -1) {
      value = (iterations % 2 == 0) ? value : -value;
    } else {
      for (int64_t i = 0; i < max; i++) {
        value *= constant_operand.constant_operand_32;
      }
    }
    result.result_32 = value;
  }

  // If we overflowed and we have a division, then value is 0 and we are done.
  if ((overflowed == true) && (instruction_kind == HInstruction::kDiv)) {
    accum_assoc.finalized_calculation = true;
    if (is_long) {
      result.result_64 = 0;
    } else {
      result.result_32 = 0;
    }
  } else {
    if (((is_long && result.result_64 == 0) || (!is_long && result.result_32 == 0)) && (instruction_kind == HInstruction::kMul)) {
      // If the result is nil, then we finalized the result.
      accum_assoc.finalized_calculation = true;
    }
  }
  if (is_long) {
    accum_assoc.const_instruction = graph_->GetLongConstant(result.result_64);
  } else {
    accum_assoc.const_instruction = graph_->GetIntConstant(result.result_32);
  }

  // Otherwise, it is fine as long as we did enough iterations or the value is 0.
  return ((is_long && result.result_64 == 0) || (!is_long && result.result_32 == 0) || overflowed == false);
}

/**
 * @brief Perform the constant calculation: walk through the list and determine if we
 * can sink the calculation.
 * @param accumulator_list the AccumulatorAssociation list to consider for sinking.
 * @param loop_info info about the loop we're working on.
 * @param to_sink the list of constants to sink from the loop.
 */
void HConstantCalculationSinking::DoConstantCalculation(
        const std::vector<AccumulatorAssociation>& accumulator_list,
        HLoopInformation_X86* loop_info,
        std::vector<AccumulatorAssociation>& to_sink) const {
  // Iterate through the expression candidates.
  std::vector<AccumulatorAssociation>::const_iterator accu_iter;
  for (accu_iter = accumulator_list.begin(); accu_iter != accumulator_list.end(); accu_iter++) {
    AccumulatorAssociation accum_assoc = *accu_iter;
    DCHECK(accum_assoc.linear_instruction != nullptr);

    HInstruction* linear_instruction = accum_assoc.linear_instruction;
    int64_t num_iterations = loop_info->GetNumIterations(linear_instruction->GetBlock());

    HInstruction::InstructionKind instruction_kind = linear_instruction->GetKind();
    switch (instruction_kind) {
      case HInstruction::kRem:
      case HInstruction::kDiv: {
        if (accum_assoc.constant->IsArithmeticZero()) {
          continue;
        }
      }
      FALLTHROUGH_INTENDED;
      case HInstruction::kAdd:
      case HInstruction::kSub:
      case HInstruction::kMul: {
        bool result = HandleCalculation(instruction_kind, num_iterations, accum_assoc);
        if (result) {
          to_sink.push_back(accum_assoc);
        }
        break;
      }
      default:
        break;
    }
  }
}

HInductionVariable* HConstantCalculationSinking::GetBasicInductionVariable(
        HLoopInformation_X86* loop_info) const {
  HLoopBoundInformation bound_info = loop_info->GetBoundInformation();

  return bound_info.GetLoopBIV();
}

static void UpdateAllUsersOutsideLoop(HLoopInformation_X86* loop_info,
                                      HInstruction* inst,
                                      HInstruction* to_replace) {
  for (HAllUseIterator use_it(inst); !use_it.Done(); use_it.Advance()) {
    HInstruction* user = use_it.Current();
    if (!loop_info->Contains(*user->GetBlock())) {
      use_it.ReplaceInput(to_replace);
    } else {
      if (use_it.IsEnv()) {
        use_it.ReplaceInput(nullptr);
      }
    }
  }
}

void HConstantCalculationSinking::DoConstantSinking(const std::vector<AccumulatorAssociation>& to_sink,
        HLoopInformation_X86* loop_info, std::vector<AccumulatorAssociation>& to_remove) const {
  std::vector<AccumulatorAssociation>::const_iterator sink_iter;
  // Get the Exit Basic Block.
  HBasicBlock* exit_bb = loop_info->GetExitBlock(true);
  DCHECK(exit_bb != nullptr);

  for (sink_iter = to_sink.begin(); sink_iter != to_sink.end(); sink_iter++) {
    AccumulatorAssociation accum_assoc = *sink_iter;
    HInstruction* phi = accum_assoc.phi;
    HInstruction* constant = accum_assoc.const_instruction;
    HInstruction* inst = accum_assoc.linear_instruction;
    HInstruction* to_replace = constant;

    DCHECK(phi != nullptr);

    if (constant != nullptr) {
      if (accum_assoc.finalized_calculation == false) {
        // Create new instruction.
        HInstruction *initial_instr = nullptr;
        initial_instr = (phi->InputAt(0) == inst) ? phi->InputAt(1) : phi->InputAt(0);
        Primitive::Type instruction_type = accum_assoc.linear_instruction->GetType();
        switch (accum_assoc.linear_instruction->GetKind()) {
          case HInstruction::kAdd:
            to_replace = new (graph_->GetArena()) HAdd(instruction_type, initial_instr,
                                                       constant, inst->GetDexPc());
            break;
          case HInstruction::kSub:
            to_replace = new (graph_->GetArena()) HSub(instruction_type, initial_instr,
                                                       constant, inst->GetDexPc());
            break;
          case HInstruction::kMul:
            to_replace = new (graph_->GetArena()) HMul(instruction_type, initial_instr,
                                                       constant, inst->GetDexPc());
            break;
          case HInstruction::kRem:
            to_replace = new (graph_->GetArena()) HRem(instruction_type, initial_instr,
                                                       constant, inst->GetDexPc());
            break;
          case HInstruction::kDiv:
            to_replace = new (graph_->GetArena()) HDiv(instruction_type, initial_instr,
                                                       constant, inst->GetDexPc());
            break;
          default:
            PRINT_PASS_MESSAGE(this, "Operation is not supported");
            continue;
        }
        DCHECK(to_replace != nullptr);

        // Put new instruction as the first instruction in the exit block.
        exit_bb->InsertInstructionBefore(to_replace, exit_bb->GetFirstInstruction());
      }

      // Replace calculation result with constant or new instruction.
      UpdateAllUsersOutsideLoop(loop_info, phi, to_replace);
      UpdateAllUsersOutsideLoop(loop_info, inst, to_replace);

      to_remove.push_back(accum_assoc);
      PRINT_PASS_MESSAGE(this, "Constant has been sunk for the loop with head bb %d",
                         loop_info->GetHeader()->GetBlockId());
      MaybeRecordStat(MethodCompilationStat::kIntelCCS);
    }
  }
}

bool HConstantCalculationSinking::IsLoopGoodForCCS(HLoopInformation_X86* loop_info) const {
  DCHECK(loop_info->IsInner());

  // Loop should be countable.
  if (!loop_info->HasKnownNumIterations()) {
    return false;
  }

  // Not guaranteed that all iterations will be executed.
  if (loop_info->IsIrreducible()) {
    return false;
  }

  return true;
}

/**
 * @brief Remove unnecessary instructions after sinking.
 * @param to_remove the AccumulatorAssociation list to consider for dead code removing.
 */
void HConstantCalculationSinking::RemoveDeadCode(
        const std::vector<AccumulatorAssociation>& to_remove) const {
  if (to_remove.empty()) {
    return;
  }

  std::vector<AccumulatorAssociation>::const_iterator it;
  for (it = to_remove.begin(); it != to_remove.end(); it++) {
    HPhi* phi = (*it).phi;
    DeletePhiAndUsers(phi);
  }
}

/**
 * @brief Handle a loop for the sinking of an constant operation.
 * @param loop_info info about the loop we're working on.
 */
void HConstantCalculationSinking::HandleLoop(HLoopInformation_X86* loop_info) const {
  std::vector<AccumulatorAssociation> accumulator_list;
  std::vector<AccumulatorAssociation> to_remove;
  std::vector<AccumulatorAssociation> to_sink;

  // Step 0: Check with the gate that we can try to optimize this loop.
  if (!IsLoopGoodForCCS(loop_info)) {
    PRINT_PASS_MESSAGE(this, "Loop with head bb %d is not good for CCS",
                       loop_info->GetHeader()->GetBlockId());
    return;
  }

  // Step 1: Get loop's main IV.
  HInductionVariable* choosen_iv = GetBasicInductionVariable(loop_info);
  if (choosen_iv == nullptr) {
    return;
  }

  // Step 2: Find the accumulators that are eligible.
  FillAccumulator(loop_info, accumulator_list);

  // Step 3: Do constant calculation on our accumulator.
  DoConstantCalculation(accumulator_list, loop_info, to_sink);

  // Step 4: Sink the constant operations.
  DoConstantSinking(to_sink, loop_info, to_remove);

  // Step 5: Remove dead instructions.
  RemoveDeadCode(to_remove);

  PRINT_PASS_MESSAGE(this, "Finished to sink constant operation for loop with head bb #%d",
                     loop_info->GetHeader()->GetBlockId());
}

void HConstantCalculationSinking::Run() {
  HGraph_X86* graph = GetGraphX86();
  HLoopInformation_X86* loop_info = graph->GetLoopInformation();

  // CCS will affect debugging the VR values.
  if (graph_->IsDebuggable()) {
    PRINT_PASS_OSTREAM_MESSAGE(this, "Skip debuggable method "
                                     << GetMethodName(graph_));
    return;
  }

  PRINT_PASS_OSTREAM_MESSAGE(this, "Try to optimize loop in method " <<
                             GetMethodName(graph_));

  // Walk through all inner loops.
  for (HOnlyInnerLoopIterator it_loop(loop_info); !it_loop.Done(); it_loop.Advance()) {
    HLoopInformation_X86* current = it_loop.Current();
    HandleLoop(current);
  }
}

}  // namespace art
