/*
 * Copyright (C) 2015 The Android Open Source Project
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
 * Modified by Intel Corporation
 */

#ifndef ART_COMPILER_OPTIMIZING_NODES_X86_H_
#define ART_COMPILER_OPTIMIZING_NODES_X86_H_

namespace art {

// Compute the address of the method for X86 Constant area support.
class HX86ComputeBaseMethodAddress : public HExpression<0> {
 public:
  // Treat the value as an int32_t, but it is really a 32 bit native pointer.
  HX86ComputeBaseMethodAddress()
      : HExpression(Primitive::kPrimInt, SideEffects::None(), kNoDexPc) {}

  DECLARE_INSTRUCTION(X86ComputeBaseMethodAddress);

 private:
  DISALLOW_COPY_AND_ASSIGN(HX86ComputeBaseMethodAddress);
};

// Load a constant value from the constant table.
class HX86LoadFromConstantTable : public HExpression<2> {
 public:
  HX86LoadFromConstantTable(HX86ComputeBaseMethodAddress* method_base,
                            HConstant* constant)
      : HExpression(constant->GetType(), SideEffects::None(), kNoDexPc) {
    SetRawInputAt(0, method_base);
    SetRawInputAt(1, constant);
  }

  HX86ComputeBaseMethodAddress* GetBaseMethodAddress() const {
    return InputAt(0)->AsX86ComputeBaseMethodAddress();
  }

  HConstant* GetConstant() const {
    return InputAt(1)->AsConstant();
  }

  DECLARE_INSTRUCTION(X86LoadFromConstantTable);

 private:
  DISALLOW_COPY_AND_ASSIGN(HX86LoadFromConstantTable);
};

// Version of HNeg with access to the constant table for FP types.
class HX86FPNeg : public HExpression<2> {
 public:
  HX86FPNeg(Primitive::Type result_type,
            HInstruction* input,
            HX86ComputeBaseMethodAddress* method_base,
            uint32_t dex_pc)
      : HExpression(result_type, SideEffects::None(), dex_pc) {
    DCHECK(Primitive::IsFloatingPointType(result_type));
    SetRawInputAt(0, input);
    SetRawInputAt(1, method_base);
  }

  DECLARE_INSTRUCTION(X86FPNeg);

 private:
  DISALLOW_COPY_AND_ASSIGN(HX86FPNeg);
};

// X86 version of HPackedSwitch that holds a pointer to the base method address.
class HX86PackedSwitch : public HTemplateInstruction<2> {
 public:
  HX86PackedSwitch(int32_t start_value,
                   int32_t num_entries,
                   HInstruction* input,
                   HX86ComputeBaseMethodAddress* method_base,
                   uint32_t dex_pc)
    : HTemplateInstruction(SideEffects::None(), dex_pc),
      start_value_(start_value),
      num_entries_(num_entries) {
    SetRawInputAt(0, input);
    SetRawInputAt(1, method_base);
  }

  bool IsControlFlow() const OVERRIDE { return true; }

  int32_t GetStartValue() const { return start_value_; }

  int32_t GetNumEntries() const { return num_entries_; }

  HX86ComputeBaseMethodAddress* GetBaseMethodAddress() const {
    return InputAt(1)->AsX86ComputeBaseMethodAddress();
  }

  HBasicBlock* GetDefaultBlock() const {
    // Last entry is the default block.
    return GetBlock()->GetSuccessors()[num_entries_];
  }

  DECLARE_INSTRUCTION(X86PackedSwitch);

 private:
  const int32_t start_value_;
  const int32_t num_entries_;

  DISALLOW_COPY_AND_ASSIGN(HX86PackedSwitch);
};

// X86/X86-64 version of HBoundsCheck that checks length in array descriptor.
class HX86BoundsCheckMemory : public HExpression<2> {
 public:
  HX86BoundsCheckMemory(HInstruction* index, HInstruction* array, uint32_t dex_pc)
      : HExpression(index->GetType(), SideEffects::CanTriggerGC(), dex_pc) {
    DCHECK_EQ(array->GetType(), Primitive::kPrimNot);
    DCHECK(Primitive::IsIntegralType(index->GetType()));
    SetRawInputAt(0, index);
    SetRawInputAt(1, array);
  }

  bool CanBeMoved() const OVERRIDE { return true; }
  bool InstructionDataEquals(HInstruction* other ATTRIBUTE_UNUSED) const OVERRIDE {
    return true;
  }

  bool CanDoImplicitNullCheckOn(HInstruction* obj) const OVERRIDE {
    return obj == InputAt(1);
  }

  bool NeedsEnvironment() const OVERRIDE { return true; }

  bool CanBeSubstitutedWithItsInput() const OVERRIDE { return true; }

  bool CanThrow() const OVERRIDE { return true; }

  virtual size_t GetBaseInputIndex() const OVERRIDE { return 1; }

  HInstruction* GetIndex() const { return InputAt(0); }

  HInstruction* GetArray() const { return InputAt(1); }

  DECLARE_INSTRUCTION(X86BoundsCheckMemory);

 private:
  DISALLOW_COPY_AND_ASSIGN(HX86BoundsCheckMemory);
};

class HInstructionRHSMemory : public HTemplateInstruction<3> {
 public:
  size_t InputCount() const OVERRIDE { return input_count_; }

  DECLARE_ABSTRACT_INSTRUCTION(InstructionRHSMemory);

  size_t GetOffset() const { return offset_; }

  Primitive::Type GetType() const OVERRIDE { return type_; }

  bool CanDoImplicitNullCheckOn(HInstruction* obj) const OVERRIDE {
    // We can do an implicit check if we don't have an index and the offset is small.
    return obj == InputAt(1) && from_static_ == false &&
           offset_ < kPageSize && input_count_ == 2;
  }

  void SetFromStatic() { from_static_ = true; }

  virtual size_t GetBaseInputIndex() const { return 1; }

  bool InstructionDataEquals(HInstruction* other) const OVERRIDE {
    HInstructionRHSMemory* other_rhs = other->AsInstructionRHSMemory();
    return GetOffset() == other_rhs->GetOffset()
           && from_static_ == other_rhs->from_static_;
  }

  size_t ComputeHashCode() const OVERRIDE {
    return (HInstruction::ComputeHashCode() << 7) | GetOffset();
  }

 protected:
  HInstructionRHSMemory(Primitive::Type type,
                        HInstruction* lhs,
                        HInstruction* base,
                        HInstruction* index,
                        size_t offset,
                        uint32_t dex_pc = kNoDexPc)
    : HTemplateInstruction<3>(SideEffects::FieldReadOfType(type, false), dex_pc),
      type_(type),
      from_static_(false),
      offset_(offset),
      input_count_(index == nullptr ? 2 : 3) {
      if (index != nullptr) {
        SetRawInputAt(2, index);
      }
      SetRawInputAt(0, lhs);
      SetRawInputAt(1, base);
    }

 private:
  const Primitive::Type type_;
  bool from_static_;
  const size_t offset_;
  uint32_t input_count_;

  DISALLOW_COPY_AND_ASSIGN(HInstructionRHSMemory);
};

class HAddRHSMemory : public HInstructionRHSMemory {
 public:
  HAddRHSMemory(Primitive::Type type,
                HInstruction* lhs,
                HInstruction* base,
                HInstruction* index,
                size_t offset)
      : HInstructionRHSMemory(type, lhs, base, index, offset) {}

  DECLARE_INSTRUCTION(AddRHSMemory);

 private:
  DISALLOW_COPY_AND_ASSIGN(HAddRHSMemory);
};

class HSubRHSMemory : public HInstructionRHSMemory {
 public:
  HSubRHSMemory(Primitive::Type type,
                HInstruction* lhs,
                HInstruction* base,
                HInstruction* index,
                size_t offset)
      : HInstructionRHSMemory(type, lhs, base, index, offset) {}

  DECLARE_INSTRUCTION(SubRHSMemory);

 private:
  DISALLOW_COPY_AND_ASSIGN(HSubRHSMemory);
};

class HMulRHSMemory : public HInstructionRHSMemory {
 public:
  HMulRHSMemory(Primitive::Type type,
                HInstruction* lhs,
                HInstruction* base,
                HInstruction* index,
                size_t offset)
      : HInstructionRHSMemory(type, lhs, base, index, offset) {}

  DECLARE_INSTRUCTION(MulRHSMemory);

 private:
  DISALLOW_COPY_AND_ASSIGN(HMulRHSMemory);
};

class HDivRHSMemory : public HInstructionRHSMemory {
 public:
  HDivRHSMemory(Primitive::Type type,
                HInstruction* lhs,
                HInstruction* base,
                HInstruction* index,
                size_t offset)
      : HInstructionRHSMemory(type, lhs, base, index, offset) {}

  DECLARE_INSTRUCTION(DivRHSMemory);

 private:
  DISALLOW_COPY_AND_ASSIGN(HDivRHSMemory);
};

class HInstructionLHSMemory : public HTemplateInstruction<3> {
 public:
  size_t InputCount() const OVERRIDE { return input_count_; }

  void SetArgumentAt(size_t index, HInstruction* argument) {
    SetRawInputAt(index, argument);
  }

  DECLARE_ABSTRACT_INSTRUCTION(InstructionLHSMemory);

  size_t GetOffset() const { return offset_; }

  Primitive::Type GetType() const OVERRIDE { return type_; }

  bool CanDoImplicitNullCheckOn(HInstruction* obj) const OVERRIDE {
    // We can do an implicit check if we don't have an index and the offset is small.
    return obj == InputAt(0) && from_static_ == false &&
           offset_ < kPageSize && input_count_ == 2;
  }

  void SetFromStatic() { from_static_ = true; }

  HInstruction* GetBase() const { return InputAt(0); }

  HInstruction* GetRHS() const { return InputAt(InputCount() - 1); }

  HInstruction* GetIndex() const {
    return InputCount() == 2 ? nullptr : InputAt(1);
  }

 protected:
  HInstructionLHSMemory(HInstructionRHSMemory* rhs_op,
                        HInstruction* rhs,
                        uint32_t dex_pc = kNoDexPc)
    : HTemplateInstruction<3>(SideEffects::None(), dex_pc),
      type_(rhs_op->GetType()),
      from_static_(false),
      offset_(rhs_op->GetOffset()) {
    input_count_ = rhs_op->InputCount();
    SetRawInputAt(0, rhs_op->InputAt(1));
    if (input_count_ == 2) {
      SetRawInputAt(1, rhs);
    } else {
      SetRawInputAt(1, rhs_op->InputAt(2));
      SetRawInputAt(2, rhs);
    }
  }

 private:
  const Primitive::Type type_;
  bool from_static_;
  const size_t offset_;
  uint32_t input_count_;

  DISALLOW_COPY_AND_ASSIGN(HInstructionLHSMemory);
};

class HAddLHSMemory : public HInstructionLHSMemory {
 public:
  HAddLHSMemory(HInstructionRHSMemory* rhs_op,
                HInstruction* rhs,
                uint32_t dex_pc)
      : HInstructionLHSMemory(rhs_op, rhs, dex_pc) {}

  DECLARE_INSTRUCTION(AddLHSMemory);

 private:
  DISALLOW_COPY_AND_ASSIGN(HAddLHSMemory);
};

// Return the address of the execution counters for this method.
class HX86ReturnExecutionCountTable : public HExpression<1> {
 public:
  HX86ReturnExecutionCountTable(bool is_64bit,
                                HCurrentMethod* current_method,
                                uint32_t dex_pc = kNoDexPc)
      : HExpression(is_64bit ? Primitive::kPrimLong : Primitive::kPrimInt,
                    SideEffects::None(),
                    dex_pc) {
    SetRawInputAt(0, current_method);
  }

  DECLARE_INSTRUCTION(X86ReturnExecutionCountTable);

 private:
  DISALLOW_COPY_AND_ASSIGN(HX86ReturnExecutionCountTable);
};

// Increment the counter for the given block number.
class HX86IncrementExecutionCount : public HExpression<1> {
 public:
  HX86IncrementExecutionCount(uint32_t block_number,
                              HX86ReturnExecutionCountTable* count_table,
                              uint32_t dex_pc = kNoDexPc)
      : HExpression(Primitive::kPrimVoid, SideEffects::None(), dex_pc),
        block_number_(block_number) {
    SetRawInputAt(0, count_table);
  }

  uint32_t GetBlockNumber() const { return block_number_; }

  DECLARE_INSTRUCTION(X86IncrementExecutionCount);

 private:
  const uint32_t block_number_;

  DISALLOW_COPY_AND_ASSIGN(HX86IncrementExecutionCount);
};

class HSpeculationGuard : public HCondition {
 public:
  explicit HSpeculationGuard(HInstruction* left,
                             HInstruction* right,
                             uint32_t dex_pc = kNoDexPc) :
      HCondition(left, right, dex_pc) {
  }
  virtual ~HSpeculationGuard() {}

  // Speculative guards can be moved anywhere in the code as long
  // as they are still guarding the desired control path.
  virtual bool CanBeMoved() const { return true; }

  virtual HConstant* Evaluate(HIntConstant* x ATTRIBUTE_UNUSED,
                              HIntConstant* y ATTRIBUTE_UNUSED) const OVERRIDE {
    VLOG(compiler) << DebugName() << " is not defined for the (int, int) case.";
    return nullptr;
  }
  virtual HConstant* Evaluate(HLongConstant* x ATTRIBUTE_UNUSED,
                              HLongConstant* y ATTRIBUTE_UNUSED) const OVERRIDE {
    VLOG(compiler) << DebugName() << " is not defined for the (long, long) case.";
    return nullptr;
  }
  virtual HConstant* Evaluate(HFloatConstant* x ATTRIBUTE_UNUSED,
                              HFloatConstant* y ATTRIBUTE_UNUSED) const OVERRIDE {
    VLOG(compiler) << DebugName() << " is not defined for the (float, float) case.";
    return nullptr;
  }
  virtual HConstant* Evaluate(HDoubleConstant* x ATTRIBUTE_UNUSED,
                              HDoubleConstant* y ATTRIBUTE_UNUSED) const OVERRIDE {
    VLOG(compiler) << DebugName() << " is not defined for the (double, double) case.";
    return nullptr;
  }

  virtual IfCondition GetConditionForFailedGuard() const = 0;

  virtual IfCondition GetCondition() const OVERRIDE {
    // In general, we want the true case to failed guard. We do this
    // so that the "fallthrough" case is the success of guard.
    return GetConditionForFailedGuard();
  }

  virtual IfCondition GetOppositeCondition() const OVERRIDE {
    switch (GetCondition()) {
      case kCondEQ: return kCondNE;
      case kCondNE: return kCondEQ;
      case kCondLT: return kCondGE;
      case kCondLE: return kCondGT;
      case kCondGT: return kCondLE;
      case kCondGE: return kCondLT;
      default: UNIMPLEMENTED(FATAL) << "Unhandled condition.";
    }
    // This is unreachable.
    return kCondEQ;
  }

  bool IsControlFlow() const OVERRIDE { return false; }

  DECLARE_INSTRUCTION(SpeculationGuard);

 private:
  DISALLOW_COPY_AND_ASSIGN(HSpeculationGuard);
};

class HDevirtGuard : public HSpeculationGuard {
 public:
  HDevirtGuard(HLoadClass* predicted_class,
               HInstruction* object_class,
               uint32_t dex_pc = kNoDexPc) :
      HSpeculationGuard(predicted_class, object_class, dex_pc) {
  }

  HInstruction* GetInstance() {
    return InputAt(1);
  }

  HLoadClass* GetPredictedClass() {
    return InputAt(0)->AsLoadClass();
  }

  IfCondition GetConditionForFailedGuard() const OVERRIDE {
    // If the classes are not equal, this means we failed the guard.
    return kCondNE;
  }

  DECLARE_INSTRUCTION(DevirtGuard);

 private:
  DISALLOW_COPY_AND_ASSIGN(HDevirtGuard);
};

class HTrap : public HTemplateInstruction<0> {
 public:
  explicit HTrap(uint32_t dex_pc = kNoDexPc)
      : HTemplateInstruction(SideEffects::None(), dex_pc) {}

  bool IsControlFlow() const OVERRIDE { return true; }

  DECLARE_INSTRUCTION(Trap);

 private:
  DISALLOW_COPY_AND_ASSIGN(HTrap);
};

// Remember the invoke target class for a virtual/interface invoke.
class HX86ProfileInvoke : public HExpression<2> {
 public:
  HX86ProfileInvoke(uint32_t index,
                    HCurrentMethod* current_method,
                    HInstruction* object,
                    uint32_t dex_pc)
      : HExpression(Primitive::kPrimVoid, SideEffects::AllWritesAndReads(), dex_pc),
        index_(index) {
    SetRawInputAt(0, current_method);
    SetRawInputAt(1, object);
  }

  uint32_t GetIndex() const { return index_; }

  DECLARE_INSTRUCTION(X86ProfileInvoke);

 private:
  const uint32_t index_;

  DISALLOW_COPY_AND_ASSIGN(HX86ProfileInvoke);
};

// Fictive switch between normal and OSR entries.
class HOsrFork : public HTemplateInstruction<0> {
 public:
  explicit HOsrFork(uint32_t dex_pc = kNoDexPc)
    : HTemplateInstruction(SideEffects::None(), dex_pc) {}

  bool IsControlFlow() const OVERRIDE { return true; }

  HBasicBlock* GetNormalPath() const {
    return GetBlock()->GetSuccessors()[0];
  }

  HBasicBlock* GetOsrPath() const {
    return GetBlock()->GetSuccessors()[1];
  }

  DECLARE_INSTRUCTION(OsrFork);

 private:
  DISALLOW_COPY_AND_ASSIGN(HOsrFork);
};

// Fictive jump from OSR mode.
class HOsrJump : public HTemplateInstruction<0> {
 public:
  explicit HOsrJump(uint32_t dex_pc = kNoDexPc)
    : HTemplateInstruction(SideEffects::None(), dex_pc) {}

  bool IsControlFlow() const OVERRIDE { return true; }

  DECLARE_INSTRUCTION(OsrJump);

 private:
  DISALLOW_COPY_AND_ASSIGN(HOsrJump);
};

class HOsrFictiveValue : public HExpression<0> {
 public:
  HOsrFictiveValue(Primitive::Type parameter_type)
      : HExpression(parameter_type, SideEffects::None(), kNoDexPc) { }

  bool CanBeMoved() const OVERRIDE { return false; }

  DECLARE_INSTRUCTION(OsrFictiveValue);

 private:

  DISALLOW_COPY_AND_ASSIGN(HOsrFictiveValue);
};

class HOsrEntryPoint : public HTemplateInstruction<0> {
 public:
  explicit HOsrEntryPoint(uint32_t dex_pc = kNoDexPc)
      : HTemplateInstruction(SideEffects::CanTriggerGC(), dex_pc) {}

  bool CanBeMoved() const OVERRIDE { return false; }

  DECLARE_INSTRUCTION(OsrEntryPoint);

 private:
  DISALLOW_COPY_AND_ASSIGN(HOsrEntryPoint);
};

}  // namespace art

#endif  // ART_COMPILER_OPTIMIZING_NODES_X86_H_
