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

#ifndef VENDOR_INTEL_ART_EXTENSION_OPT_INFRASTRUCTURE_BOUND_INFORMATION_H_
#define VENDOR_INTEL_ART_EXTENSION_OPT_INFRASTRUCTURE_BOUND_INFORMATION_H_

#include <cinttypes>
#include <iosfwd>

namespace art {

// Forward declarations.
class HConstant;
class HInductionVariable;

/**
 * @brief This class is used to keep bound information.
 * @see ComputeBoundInformation which outlines the algorithm used to compute this.
 * @details This is not guaranteed to be filled completely.
 * - loop_biv may be null.
 * - comparison_condition is only valid if is_simple_count_up or is_simple_count_down are true.
 * - num_iterations, biv_start_value, and biv_end_value are only valid if knowIterations is true.
 */
class HLoopBoundInformation {
 public:
  HLoopBoundInformation() :
    loop_biv_(nullptr), constant_bound_(nullptr),
    is_simple_count_up_(false), is_simple_count_down_(false),
    comparison_condition_(0), num_iterations_(-1) {}

  HInductionVariable* GetLoopBIV() const { return loop_biv_; }

  void SetLoopBIV(HInductionVariable* biv) { loop_biv_ = biv; }

  HConstant* GetConstantBound() const { return constant_bound_; }

  void SetConstantBound(HConstant* constant_bound) { constant_bound_ = constant_bound; }

  bool IsSimpleCountUp() const { return is_simple_count_up_; }

  void SetSimpleCountUp(bool b = true) { is_simple_count_up_ = b; }

  bool IsSimpleCountDown() const { return is_simple_count_down_; }

  void SetSimpleCountDown(bool b = true) { is_simple_count_down_ = b; }

  int GetComparisonCondition() const { return comparison_condition_; }

  void SetComparisonCondition(int cond) { comparison_condition_ = cond; }

  int64_t GetNumIterations() const { return num_iterations_; }

  void SetNumIterations(int64_t num_iterations) { num_iterations_ = num_iterations; }

  int64_t GetIntegralBIVStartValue() const;

  void SetIntegralBIVStartValue(int64_t start_value);

  int64_t GetIntegralBIVEndValue() const;

  void SetIntegralBIVEndValue(int64_t end_value);

  double GetFPBIVStartValue() const;

  void SetFPBIVStartValue(double start_value);

  double GetFPBIVEndValue() const;

  void SetFPBIVEndValue(double end_value);

  void Dump(std::ostream& os);

  bool IsFP() const;

  bool IsInteger() const;

  bool IsLong() const;

  bool IsFloat() const;

  bool IsDouble() const;

  bool IsWide() const;

 private:
  /**
   * @brief Used to store the loop's BIV.
   * @details In order for this to exist, the loop must have only one backedge and exit.
   * Additionally this BIV must be used in the condition that iterates the loop.
   */
  HInductionVariable* loop_biv_;

  /**
   * @brief The constant of the loop bound.
   */
  HConstant* constant_bound_;

  /**
   * @brief Whether the loop is a simple count up loop.
   * @details The IV must increment in positive manner, the loop condition must use BIV
   * and constant/invariant.
   */
  bool is_simple_count_up_;

  /**
   * @brief Whether the loop is a simple count down loop.
   * @details The IV must increment in negative manner, the loop condition must use BIV
   * and constant/invariant.
   */
  bool is_simple_count_down_;

  /**
   * @brief Contains the comparison condition as an OP_IF bytecode.
   * @details Only valid if is_simple_count_up or is_simple_count_down are true.
   */
  int comparison_condition_;

  /**
   * @brief The number of iterations through the loop.
   * @details If this value is -1, then number of iterations is not known.
   */
  int64_t num_iterations_;

  typedef union {
    int64_t integral = 0;
    double fp;
  } Border;

  /**
   * @brief The start value of the induction variable.
   */
  Border biv_start_value_ = Border();

  /**
   * @brief The end value of the induction variable.
   */
  Border biv_end_value_ = Border();
};
}  // namespace art

#endif  // VENDOR_INTEL_ART_EXTENSION_OPT_INFRASTRUCTURE_BOUND_INFORMATION_H_
