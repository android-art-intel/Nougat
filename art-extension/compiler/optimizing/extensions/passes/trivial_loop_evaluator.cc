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
#include "pass_option.h"
#include "trivial_loop_evaluator.h"

namespace art {

class TLEVisitor : public HGraphVisitor {
 public:
  union Value {
    explicit Value() { i = 0; }
    explicit Value(int8_t _i) { i = static_cast<int32_t>(_i); }
    explicit Value(int16_t _i) { i = static_cast<int32_t>(_i); }
    explicit Value(int32_t _i) { i = _i; }
    explicit Value(int64_t _l) { l = _l; }
    explicit Value(float _f) { f = _f; }
    explicit Value(double _d) { d = _d; }
    int32_t i;
    int64_t l;
    float f;
    double d;
  };

  explicit TLEVisitor(HGraph_X86* graph, HOptimization_X86* opt) :
      HGraphVisitor(graph),
      pred_index_(0),
      is_error_(false),
      next_bb_(nullptr),
      values_(std::less<HInstruction*>(), graph->GetArena()->Adapter()),
      phi_values_(std::less<HInstruction*>(), graph->GetArena()->Adapter()),
      opt_(opt) {}

#define NOTHING_IF_ERROR if (is_error_) return
#define SWITCH_FOR_CAST_TYPES(instr, \
                         condition, \
                         bool_case, \
                         byte_case, \
                         short_case, \
                         int_case, \
                         long_case, \
                         float_case, \
                         double_case) \
        do { switch (condition) { \
          case Primitive::kPrimBoolean: bool_case; break; \
          case Primitive::kPrimByte: byte_case; break; \
          case Primitive::kPrimShort: short_case; break; \
          case Primitive::kPrimInt: int_case; break; \
          case Primitive::kPrimLong: long_case; break; \
          case Primitive::kPrimFloat: float_case; break; \
          case Primitive::kPrimDouble: double_case; break; \
          default: SetError(instr); \
        }} while (false)

#define SWITCH_FOR_JAVA_TYPES(instr, \
                         condition, \
                         bool_case, \
                         int_case, \
                         long_case, \
                         float_case, \
                         double_case) \
        do { switch (condition) { \
          case Primitive::kPrimBoolean: bool_case; break; \
          case Primitive::kPrimByte: \
          case Primitive::kPrimShort: \
          case Primitive::kPrimInt: int_case; break; \
          case Primitive::kPrimLong: long_case; break; \
          case Primitive::kPrimFloat: float_case; break; \
          case Primitive::kPrimDouble: double_case; break; \
          default: SetError(instr); \
        }} while (false)

  void VisitBasicBlock(HBasicBlock* block) {
    for (HInstructionIterator it(block->GetPhis()); !it.Done(); it.Advance()) {
      it.Current()->Accept(this);
    }
    // Update phis.
    for (auto it : phi_values_) {
      values_.Overwrite(it.first, it.second);
    }
    phi_values_.clear();
    for (HInstructionIterator it(block->GetInstructions()); !it.Done(); it.Advance()) {
      it.Current()->Accept(this);
    }
  }

  void VisitInstruction(HInstruction* instruction) OVERRIDE {
    NOTHING_IF_ERROR;
    // If this instruction does not have a visitor then we do not support it.
    SetError(instruction);
  }

  void VisitSuspendCheck(HSuspendCheck* instr ATTRIBUTE_UNUSED) OVERRIDE {
    // We can ignore suspend checks for this optimization.
  }
  void VisitGoto(HGoto* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    next_bb_ = instr->GetSuccessor();
  }

  void VisitIf(HIf* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    next_bb_ =  GetValue(instr->InputAt(0)).i == 1 ?
               instr->IfTrueSuccessor() :
               instr->IfFalseSuccessor();
  }

  void VisitPhi(HPhi* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    Value value = GetValue(instr->InputAt(pred_index_));
    NOTHING_IF_ERROR;
    // We need to store right values to update all phis in parallel.
    phi_values_.Overwrite(instr, value);
  }

#define INTEGRAL_TO_FP_CONV(vmin, vmax, cast, value) \
  (std::isnan(value) ? 0 : (value >= vmax ? vmax : (value <= vmin ? vmin : static_cast<cast>(value))))
  void VisitTypeConversion(HTypeConversion* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    HInstruction* input = instr->InputAt(0);
    Primitive::Type in_type = instr->GetInputType();
    Primitive::Type out_type = instr->GetResultType();

    Value in_value = GetValue(input);
    NOTHING_IF_ERROR;

    SWITCH_FOR_JAVA_TYPES(instr, in_type,
      // bool case.
      SetError(instr),
      // int case.
      SWITCH_FOR_CAST_TYPES(instr, out_type,
        SetError(instr),
        values_.Overwrite(instr, Value(static_cast<int8_t>(in_value.i))),
        values_.Overwrite(instr, Value(static_cast<int16_t>(in_value.i))),
        values_.Overwrite(instr, Value(static_cast<int32_t>(in_value.i))),
        values_.Overwrite(instr, Value(static_cast<int64_t>(in_value.i))),
        values_.Overwrite(instr, Value(static_cast<float>(in_value.i))),
        values_.Overwrite(instr, Value(static_cast<double>(in_value.i)))),
      // long case.
      SWITCH_FOR_CAST_TYPES(instr, out_type,
        SetError(instr),
        values_.Overwrite(instr, Value(static_cast<int8_t>(in_value.l))),
        values_.Overwrite(instr, Value(static_cast<int16_t>(in_value.l))),
        values_.Overwrite(instr, Value(static_cast<int32_t>(in_value.l))),
        SetError(instr),
        values_.Overwrite(instr, Value(static_cast<float>(in_value.l))),
        values_.Overwrite(instr, Value(static_cast<double>(in_value.l)))),
      // float case.
      SWITCH_FOR_CAST_TYPES(instr, out_type,
        SetError(instr),
        values_.Overwrite(instr, Value(INTEGRAL_TO_FP_CONV(std::numeric_limits<int8_t>::min(),
                                                           std::numeric_limits<int8_t>::max(),
                                                           int8_t,
                                                           in_value.f))),
        values_.Overwrite(instr, Value(INTEGRAL_TO_FP_CONV(std::numeric_limits<int16_t>::min(),
                                                           std::numeric_limits<int16_t>::max(),
                                                           int16_t,
                                                           in_value.f))),
        values_.Overwrite(instr, Value(INTEGRAL_TO_FP_CONV(std::numeric_limits<int32_t>::min(),
                                                           std::numeric_limits<int32_t>::max(),
                                                           int32_t,
                                                           in_value.f))),
        values_.Overwrite(instr, Value(INTEGRAL_TO_FP_CONV(std::numeric_limits<int64_t>::min(),
                                                           std::numeric_limits<int64_t>::max(),
                                                           int64_t,
                                                           in_value.f))),
        SetError(instr),
        values_.Overwrite(instr, Value(static_cast<double>(in_value.f)))),
      // double case.
      SWITCH_FOR_CAST_TYPES(instr, out_type,
        values_.Overwrite(instr, Value(INTEGRAL_TO_FP_CONV(0, 1, bool, in_value.d))),
        values_.Overwrite(instr, Value(INTEGRAL_TO_FP_CONV(std::numeric_limits<int8_t>::min(),
                                                           std::numeric_limits<int8_t>::max(),
                                                           int8_t,
                                                           in_value.d))),
        values_.Overwrite(instr, Value(INTEGRAL_TO_FP_CONV(std::numeric_limits<int16_t>::min(),
                                                           std::numeric_limits<int16_t>::max(),
                                                           int16_t,
                                                           in_value.d))),
        values_.Overwrite(instr, Value(INTEGRAL_TO_FP_CONV(std::numeric_limits<int32_t>::min(),
                                                           std::numeric_limits<int32_t>::max(),
                                                           int32_t,
                                                           in_value.d))),
        values_.Overwrite(instr, Value(INTEGRAL_TO_FP_CONV(std::numeric_limits<int64_t>::min(),
                                                           std::numeric_limits<int64_t>::max(),
                                                           int64_t,
                                                           in_value.d))),
        values_.Overwrite(instr, Value(static_cast<float>(in_value.d))),
        SetError(instr)));
  }
#undef INTEGRAL_TO_FP_CONV

  template <typename T> static bool FpEqual(T x, T y) {
    return (((x) > (y)) ?
           (((x) - (y)) < std::numeric_limits<T>::epsilon()) :
           (((y) - (x)) < std::numeric_limits<T>::epsilon()));
  }

  template <typename E, typename T> static uint32_t Compare(E eq, T x, T y) {
    return eq(x, y) ? 0 : (x > y ? 1 : -1);
  }

  int32_t Compare(HInstruction* instr,
                  HInstruction* left,
                  HInstruction* right,
                  ComparisonBias bias) {
    DCHECK_EQ(right->GetType(), right->GetType());

    Value left_value = GetValue(left);
    Value right_value = GetValue(right);
    if (is_error_) {
      return 0;
    }
    int32_t res = 0;
    SWITCH_FOR_JAVA_TYPES(instr, left->GetType(),
      SetError(instr),
      res = left_value.i == right_value.i ? 0 : (left_value.i > right_value.i ? 1 : -1),
      res = left_value.l == right_value.l ? 0 : (left_value.l > right_value.l ? 1 : -1),
      res = (isnan(left_value.f) || isnan(right_value.f))
            ? (bias == ComparisonBias::kGtBias ? 1 : -1)
            : FpEqual(left_value.f, right_value.f) ? 0 : (left_value.f > right_value.f ? 1 : -1),
      res = (isnan(left_value.d) || isnan(right_value.d))
            ? (bias == ComparisonBias::kGtBias ? 1 : -1)
            : (FpEqual(left_value.d, right_value.d) ? 0 : (left_value.d > right_value.d ? 1 : -1)));
    return res;
  }

  void VisitCompare(HCompare* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    int32_t res = Compare(instr, instr->GetLeft(), instr->GetRight(), instr->GetBias());
    NOTHING_IF_ERROR;
    values_.Overwrite(instr, Value(res));
  }

  void VisitEqual(HEqual* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    values_.Overwrite(instr,
      Value(Compare(instr, instr->GetLeft(), instr->GetRight(), instr->GetBias()) == 0 ? 1 : 0));
  }
  void VisitNotEqual(HNotEqual* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    values_.Overwrite(instr,
      Value(Compare(instr, instr->GetLeft(), instr->GetRight(), instr->GetBias()) != 0 ? 1 : 0));
  }
  void VisitLessThan(HLessThan* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    values_.Overwrite(instr,
      Value(Compare(instr, instr->GetLeft(), instr->GetRight(), instr->GetBias()) < 0 ? 1 : 0));
  }
  void VisitLessThanOrEqual(HLessThanOrEqual* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    values_.Overwrite(instr,
      Value(Compare(instr, instr->GetLeft(), instr->GetRight(), instr->GetBias()) <= 0 ? 1 : 0));
  }
  void VisitGreaterThan(HGreaterThan* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    values_.Overwrite(instr,
      Value(Compare(instr, instr->GetLeft(), instr->GetRight(), instr->GetBias()) > 0 ? 1 : 0));
  }
  void VisitGreaterThanOrEqual(HGreaterThanOrEqual* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    values_.Overwrite(instr,
      Value(Compare(instr, instr->GetLeft(), instr->GetRight(), instr->GetBias()) >= 0 ? 1 : 0));
  }

  template <typename F> void VisitUnsignedComparision(HCondition* instr, F comparator) {
    HInstruction* left = instr->GetLeft();
    HInstruction* right = instr->GetRight();
    DCHECK_EQ(left->GetType(), right->GetType());

    Value left_value = GetValue(left);
    Value right_value = GetValue(right);
    NOTHING_IF_ERROR;

    bool res = true;
    SWITCH_FOR_JAVA_TYPES(instr, left->GetType(),
      SetError(instr),
      res = comparator(static_cast<uint32_t>(left_value.i), static_cast<uint32_t>(right_value.i)),
      res = comparator(static_cast<uint64_t>(left_value.l), static_cast<uint64_t>(right_value.l)),
      SetError(instr),
      SetError(instr));
    values_.Overwrite(instr, Value(res ? 1 : 0));
  }

  void VisitBelow(HBelow* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    VisitUnsignedComparision(instr, [] (uint64_t x, uint64_t y) -> bool { return x < y; });
  }
  void VisitBelowOrEqual(HBelowOrEqual* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    VisitUnsignedComparision(instr, [] (uint64_t x, uint64_t y) -> bool { return x <= y; });
  }
  void VisitAbove(HAbove* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    VisitUnsignedComparision(instr, [] (uint64_t x, uint64_t y) -> bool { return x > y; });
  }
  void VisitAboveOrEqual(HAboveOrEqual* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    VisitUnsignedComparision(instr, [] (uint64_t x, uint64_t y) -> bool { return x >= y; });
  }

  void VisitNeg(HNeg* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    Value val = GetValue(instr->GetInput());
    NOTHING_IF_ERROR;

    SWITCH_FOR_JAVA_TYPES(instr, instr->GetInput()->GetType(),
      SetError(instr),
      values_.Overwrite(instr, Value(-val.i)),
      values_.Overwrite(instr, Value(-val.l)),
      values_.Overwrite(instr, Value(-val.f)),
      values_.Overwrite(instr, Value(-val.d)));
  }

  void VisitAdd(HAdd* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    HInstruction* left = instr->GetLeft();
    HInstruction* right = instr->GetRight();
    DCHECK_EQ(right->GetType(), right->GetType());

    Value left_value = GetValue(left);
    Value right_value = GetValue(right);
    NOTHING_IF_ERROR;

    SWITCH_FOR_JAVA_TYPES(instr, left->GetType(),
      SetError(instr),
      values_.Overwrite(instr, Value(left_value.i + right_value.i)),
      values_.Overwrite(instr, Value(left_value.l + right_value.l)),
      values_.Overwrite(instr, Value(left_value.f + right_value.f)),
      values_.Overwrite(instr, Value(left_value.d + right_value.d)));
  }

  void VisitSub(HSub* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    HInstruction* left = instr->GetLeft();
    HInstruction* right = instr->GetRight();
    DCHECK_EQ(right->GetType(), right->GetType());

    Value left_value = GetValue(left);
    Value right_value = GetValue(right);
    NOTHING_IF_ERROR;

    SWITCH_FOR_JAVA_TYPES(instr, left->GetType(),
      SetError(instr),
      values_.Overwrite(instr, Value(left_value.i - right_value.i)),
      values_.Overwrite(instr, Value(left_value.l - right_value.l)),
      values_.Overwrite(instr, Value(left_value.f - right_value.f)),
      values_.Overwrite(instr, Value(left_value.d - right_value.d)));
  }

  void VisitMul(HMul* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    HInstruction* left = instr->GetLeft();
    HInstruction* right = instr->GetRight();
    DCHECK_EQ(right->GetType(), right->GetType());

    Value left_value = GetValue(left);
    Value right_value = GetValue(right);
    NOTHING_IF_ERROR;

    SWITCH_FOR_JAVA_TYPES(instr, left->GetType(),
      SetError(instr),
      values_.Overwrite(instr, Value(left_value.i * right_value.i)),
      values_.Overwrite(instr, Value(left_value.l * right_value.l)),
      values_.Overwrite(instr, Value(left_value.f * right_value.f)),
      values_.Overwrite(instr, Value(left_value.d * right_value.d)));
  }

  void VisitDiv(HDiv* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    HInstruction* left = instr->GetLeft();
    HInstruction* right = instr->GetRight();
    DCHECK_EQ(right->GetType(), right->GetType());

    Value left_value = GetValue(left);
    Value right_value = GetValue(right);
    NOTHING_IF_ERROR;

    SWITCH_FOR_JAVA_TYPES(instr, left->GetType(),
      SetError(instr),
      values_.Overwrite(instr, right_value.i == 0 ? SetError(instr) :
                               Value(left_value.i / right_value.i)),
      values_.Overwrite(instr, right_value.l == 0 ? SetError(instr) :
                               Value(left_value.l / right_value.l)),
      values_.Overwrite(instr, Value(left_value.f / right_value.f)),
      values_.Overwrite(instr, Value(left_value.d / right_value.d)));
  }

  template <typename T, typename F> T FpRem(T x, T y, F f) {
    if (std::isnan(x) || std::isnan(y) ||
        std::isinf(x) || FpEqual(y, static_cast<T>(0))) {
      return std::numeric_limits<T>::quiet_NaN();
    }

    if (std::isinf(y)) {
      return x;
    }

    if (FpEqual(x, static_cast<T>(0))) {
      return x;
    }

    return f(x, y);
  }

  void VisitRem(HRem* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    HInstruction* left = instr->GetLeft();
    HInstruction* right = instr->GetRight();
    DCHECK_EQ(right->GetType(), right->GetType());

    Value left_value = GetValue(left);
    Value right_value = GetValue(right);
    NOTHING_IF_ERROR;

    SWITCH_FOR_JAVA_TYPES(instr, left->GetType(),
      SetError(instr),
      values_.Overwrite(instr, right_value.i == 0 ? SetError(instr) :
                               Value(right_value.i == -1 ? 0 :
                                     left_value.i % right_value.i)),
      values_.Overwrite(instr, right_value.l == 0 ? SetError(instr) :
                               Value(right_value.l == -1 ? 0 :
                                     left_value.l % right_value.l)),
      values_.Overwrite(instr,
        Value(FpRem(left_value.f, right_value.f, [] (float x, float y) ->
              float { return std::fmodf(x, y); }))),
      values_.Overwrite(instr,
       Value(FpRem(left_value.d, right_value.d, [] (double x, double y) ->
             double { return std::fmod(x, y); }))));
  }

  int32_t ComputeShiftCount(HBinaryOperation* shift_instr) {
    int32_t shift_count = 0;
    Primitive::Type type = shift_instr->GetLeft()->GetType();
    if (type == Primitive::kPrimLong) {
      shift_count = GetValue(shift_instr->GetRight()).l & kMaxLongShiftDistance;
    } else {
      shift_count = GetValue(shift_instr->GetRight()).i & kMaxIntShiftDistance;
    }
    return shift_count;
  }

  void VisitShl(HShl* instr) OVERRIDE {
    NOTHING_IF_ERROR;

    Value left_value = GetValue(instr->GetLeft());
    NOTHING_IF_ERROR;

    int32_t shift_count = ComputeShiftCount(instr);
    NOTHING_IF_ERROR;

    SWITCH_FOR_JAVA_TYPES(instr, instr->GetLeft()->GetType(),
      SetError(instr),
      values_.Overwrite(instr, Value(left_value.i << shift_count)),
      values_.Overwrite(instr, Value(left_value.l << shift_count)),
      SetError(instr),
      SetError(instr));
  }

  void VisitShr(HShr* instr) OVERRIDE {
    NOTHING_IF_ERROR;

    Value left_value = GetValue(instr->GetLeft());
    NOTHING_IF_ERROR;

    int32_t shift_count = ComputeShiftCount(instr);
    NOTHING_IF_ERROR;

    SWITCH_FOR_JAVA_TYPES(instr, instr->GetLeft()->GetType(),
      SetError(instr),
      values_.Overwrite(instr, Value(left_value.i >> shift_count)),
      values_.Overwrite(instr, Value(left_value.l >> shift_count)),
      SetError(instr),
      SetError(instr));
  }

  void VisitUShr(HUShr* instr) OVERRIDE {
    NOTHING_IF_ERROR;

    Value left_value = GetValue(instr->GetLeft());
    NOTHING_IF_ERROR;

    int32_t shift_count = ComputeShiftCount(instr);
    NOTHING_IF_ERROR;

    SWITCH_FOR_JAVA_TYPES(instr, instr->GetLeft()->GetType(),
      SetError(instr),
      values_.Overwrite(instr,
        Value(static_cast<int32_t>(static_cast<uint32_t>(left_value.i) >> shift_count))),
      values_.Overwrite(instr,
        Value(static_cast<int64_t>(static_cast<uint64_t>(left_value.l) >> shift_count))),
      SetError(instr),
      SetError(instr));
  }

  void VisitAnd(HAnd* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    SWITCH_FOR_JAVA_TYPES(instr, instr->GetType(),
      values_.Overwrite(instr,
        Value(static_cast<bool>(GetValueAsLong(instr->GetLeft()) & GetValueAsLong(instr->GetRight())))),
      values_.Overwrite(instr,
        Value(static_cast<int32_t>(GetValueAsLong(instr->GetLeft()) & GetValueAsLong(instr->GetRight())))),
      values_.Overwrite(instr,
        Value(GetValueAsLong(instr->GetLeft()) & GetValueAsLong(instr->GetRight()))),
      SetError(instr),
      SetError(instr));
  }

  void VisitOr(HOr* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    SWITCH_FOR_JAVA_TYPES(instr, instr->GetType(),
      values_.Overwrite(instr,
        Value(static_cast<bool>(GetValueAsLong(instr->GetLeft()) | GetValueAsLong(instr->GetRight())))),
      values_.Overwrite(instr,
        Value(static_cast<int32_t>(GetValueAsLong(instr->GetLeft()) | GetValueAsLong(instr->GetRight())))),
      values_.Overwrite(instr,
        Value(GetValueAsLong(instr->GetLeft()) | GetValueAsLong(instr->GetRight()))),
      SetError(instr),
      SetError(instr));
  }

  void VisitXor(HXor* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    SWITCH_FOR_JAVA_TYPES(instr, instr->GetType(),
      values_.Overwrite(instr,
        Value(static_cast<bool>(GetValueAsLong(instr->GetLeft()) ^ GetValueAsLong(instr->GetRight())))),
      values_.Overwrite(instr,
        Value(static_cast<int32_t>(GetValueAsLong(instr->GetLeft()) ^ GetValueAsLong(instr->GetRight())))),
      values_.Overwrite(instr,
        Value(GetValueAsLong(instr->GetLeft()) ^ GetValueAsLong(instr->GetRight()))),
      SetError(instr),
      SetError(instr));
  }

  void VisitNot(HNot* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    SWITCH_FOR_JAVA_TYPES(instr, instr->GetType(),
      values_.Overwrite(instr, Value(~GetValue(instr->GetInput()).i)),
      values_.Overwrite(instr, Value(~GetValue(instr->GetInput()).i)),
      values_.Overwrite(instr, Value(~GetValue(instr->GetInput()).l)),
      SetError(instr),
      SetError(instr));
  }

  void VisitBooleanNot(HBooleanNot* instr) OVERRIDE {
    NOTHING_IF_ERROR;
    SWITCH_FOR_JAVA_TYPES(instr, instr->GetType(),
      values_.Overwrite(instr, Value(GetValue(instr->GetInput()).i == 0 ? 1 : 0)),
      values_.Overwrite(instr, Value(GetValue(instr->GetInput()).i == 0 ? 1 : 0)),
      SetError(instr),
      SetError(instr),
      SetError(instr));
  }

  HBasicBlock* GetNextBasicBlock() { return next_bb_; }

  bool IsError() { return is_error_; }

  void setPredecessorIndex(int idx) {
    pred_index_ = idx;
  }

  int64_t GetValueAsLong(HInstruction* instr) {
    Value v = GetValue(instr);
    SWITCH_FOR_JAVA_TYPES(instr, instr->GetType(),
      return static_cast<int64_t>(v.i),
      return static_cast<int64_t>(v.i),
      return static_cast<int64_t>(v.l),
      return static_cast<int64_t>(v.f),
      return static_cast<int64_t>(v.d));
    return 0;
  }

  Value GetValue(HInstruction* instr) {
    Value v;
    if (instr->IsConstant()) {
      SWITCH_FOR_JAVA_TYPES(instr, instr->GetType(),
        v.i = instr->AsIntConstant()->GetValue(),
        v.i = instr->AsIntConstant()->GetValue(),
        v.l = instr->AsLongConstant()->GetValue(),
        v.f = instr->AsFloatConstant()->GetValue(),
        v.d = instr->AsDoubleConstant()->GetValue());
    } else {
      auto value_it = values_.find(instr);
      if (value_it != values_.end()) {
        return value_it->second;
      } else {
        PRINT_PASS_OSTREAM_MESSAGE(opt_,
          "Input \"" << instr << "\" is not constant.");
        SetError(instr);
      }
    }
    return v;
  }

  HConstant* GetConstant(HGraph* graph, HInstruction* instr, Value v) {
    SWITCH_FOR_JAVA_TYPES(instr, instr->GetType(),
      return graph->GetConstant(Primitive::kPrimBoolean, v.i != 0 ? 1 : 0),
      return graph->GetIntConstant(v.i),
      return graph->GetLongConstant(v.l),
      return graph->GetFloatConstant(v.f),
      return graph->GetDoubleConstant(v.d));
    return nullptr;
  }

#undef NOTHING_IF_ERROR
#undef SWITCH_FOR_CAST_TYPES
#undef SWITCH_FOR_JAVA_TYPES

  Value SetError(HInstruction* instr) {
    is_error_ = true;
    PRINT_PASS_OSTREAM_MESSAGE(opt_, "TLE could not handle " << instr);
    return Value(0);
  }

  ArenaSafeMap<HInstruction*, Value> GetConstants() const {
    return values_;
  }

 private:
  int pred_index_;
  bool is_error_;
  HBasicBlock* next_bb_;
  ArenaSafeMap<HInstruction*, Value> values_;
  ArenaSafeMap<HInstruction*, Value> phi_values_;
  HOptimization_X86* opt_;
};

void TrivialLoopEvaluator::Run() {
  // Build the list of loops belonging to the CFG.
  HGraph_X86* graph = GetGraphX86();
  HLoopInformation_X86* loop_start = graph->GetLoopInformation();
  bool graph_updated = false;
  PRINT_PASS_OSTREAM_MESSAGE(this, "TrivialLoopEvaluator: Begin " << GetMethodName(graph));

  // For each inner loop in the graph, we will try to apply TLE.
  for (HOnlyInnerLoopIterator it(loop_start); !it.Done(); it.Advance()) {
    HLoopInformation_X86* loop = it.Current();

    // First, we need to check that the loop is a valid candidate for TLE.
    if (LoopGate(loop) == false) {
      continue;
    }

    // This container will hold all the evaluated values of the loop.
    TLEVisitor visitor(graph, this);

    // Then, we will evaluate the loop if possible.
    if (!EvaluateLoop(loop, visitor)) {
      continue;
    }

    // The last step is to write back the values to the registers using the evaluated values.
    UpdateRegisters(loop, visitor);

    // At this point, we have updated the graph at least once while executing this method.
    graph_updated = true;

    // Finally, we want to remove the now-useless loop from the graph.
    if (!loop->RemoveFromGraph()) {
      LOG(FATAL) << "TrivialLoopEvaluator: Could not remove loop from the graph.";
    } else {
      MaybeRecordStat(MethodCompilationStat::kIntelRemoveTrivialLoops);
      PRINT_PASS_OSTREAM_MESSAGE(this, "TrivialLoopEvaluator: Loop \"" << loop << "\" of method \""
                << GetMethodName(graph) << "\" has been statically evaluated by TLE.");
    }
  }

  if (graph_updated) {
    graph->RebuildDomination();
  }
  PRINT_PASS_OSTREAM_MESSAGE(this, "TrivialLoopEvaluator: End " << GetMethodName(graph));
}

bool TrivialLoopEvaluator::EvaluateLoop(HLoopInformation_X86* loop, TLEVisitor& visitor) {
  const unsigned int loop_iterations = loop->GetNumIterations(loop->GetHeader());

  // For each iteration of the loop, execute its sequence of instructions.
  uint64_t current_iter = 0;
  HBasicBlock* header = loop->GetHeader();
  HBasicBlock* current_block = header;

  while (loop->Contains(*current_block)) {
    visitor.VisitBasicBlock(current_block);

    if (visitor.IsError()) {
      return false;
    }

    HBasicBlock* bb = visitor.GetNextBasicBlock();

    // For iteration count checker.
    if (current_block == header) {
      current_iter++;
      DCHECK_LE(current_iter, loop_iterations);
    }

    // Set predecessor index to handle Phi nodes in the next bb.
    visitor.setPredecessorIndex(bb->GetPredecessorIndexOf(current_block));

    current_block = bb;
  }
  // Paranoid check: we must have evaluated all our iterations. Otherwise, it means either bound
  // computation is broken, or we did not evaluate the loop properly.
  DCHECK_EQ(current_iter, loop_iterations);

  return true;
}

void TrivialLoopEvaluator::UpdateRegisters(HLoopInformation_X86* loop, TLEVisitor& visitor) {
  DCHECK(loop != nullptr);

  // We want to find all the users of the values we need to write back.
  // Then, we replace the corresponding input by the HConstant.
  for (auto value : visitor.GetConstants()) {
    HInstruction* insn = value.first;
    TLEVisitor::Value constant = value.second;
    HConstant* constant_node = nullptr;

    // Go through each of the instruction's users.
    for (HAllUseIterator it(insn); !it.Done(); it.Advance()) {
      HInstruction* user = it.Current();

      // We do not care about users in the loop (we will kill them anyway).
      if (loop->Contains(*user->GetBlock())) {
        continue;
      }

      if (constant_node == nullptr) {
        constant_node = visitor.GetConstant(graph_, insn, constant);
        DCHECK(constant_node != nullptr);
      }
      it.ReplaceInput(constant_node);
    }
  }
}

bool TrivialLoopEvaluator::LoopGate(HLoopInformation_X86* loop) const {
  DCHECK(loop != nullptr);

  unsigned int num_blocks = loop->NumberOfBlocks();
  // Loops are not normalized. Therefore, we want to skip the first BB.
  if (num_blocks > 2) {
    PRINT_PASS_OSTREAM_MESSAGE(this, "Cannot apply TLE: loop has " << num_blocks << " blocks.");
    return false;
  }

  // We currently support only innermost loops.
  if (!loop->IsInner()) {
    PRINT_PASS_OSTREAM_MESSAGE(this, "Loop is not innermost.");
    return false;
  }

  // The loop should not side exit because it would make the evaluation harder.
  if (!loop->HasOneExitEdge()) {
    PRINT_PASS_OSTREAM_MESSAGE(this, "Loop has more than one exit block.");
    return false;
  }

  // The loop must have a known number of iterations in order to evaluate it.
  if (!loop->HasKnownNumIterations()) {
    PRINT_PASS_OSTREAM_MESSAGE(this, "Loop does not have a known number of iterations.");
    return false;
  }

  // The iteration count must be smaller than the threshold we fixed in order to
  // prevent increasing compile time too much.
  static PassOption<int64_t> eval_max_iter(this, driver_, "LoopEvalMaxIter", kDefaultLoopEvalMaxIter);
  if (loop->GetNumIterations(loop->GetHeader()) > eval_max_iter.GetValue()) {
    PRINT_PASS_OSTREAM_MESSAGE(this,
        "Loop has too many iterations (max supported : " << eval_max_iter.GetValue() << ").");
    return false;
  }

  return true;
}

}  // namespace art.
