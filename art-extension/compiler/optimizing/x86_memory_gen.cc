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

#include "x86_memory_gen.h"
#include "code_generator.h"
#include "ext_alias.h"
#include "mirror/array-inl.h"

namespace art {
namespace x86 {

/**
 * Replace instructions with memory operand forms.
 */
class MemoryOperandVisitor : public HGraphVisitor {
 public:
  MemoryOperandVisitor(HGraph* graph, bool do_implicit_null_checks)
      : HGraphVisitor(graph), do_implicit_null_checks_(do_implicit_null_checks) {}

 private:
  bool BinaryFP(HBinaryOperation* bin) {
    if (Primitive::IsFloatingPointType(bin->GetResultType()) &&
        (bin->InputAt(0)->IsConstant() || bin->InputAt(1)->IsConstant()
         || bin->InputAt(0)->IsX86LoadFromConstantTable()
         || bin->InputAt(1)->IsX86LoadFromConstantTable())) {
      return true;
    }
    return false;
  }

  void VisitAdd(HAdd* add) OVERRIDE {
    VisitMathOp(add, false);
  }

  void VisitSub(HSub* sub) OVERRIDE {
    VisitMathOp(sub, false);
  }

  void VisitMul(HMul* mul) OVERRIDE {
    VisitMathOp(mul, false);
  }

  void VisitDiv(HDiv* div) OVERRIDE {
    VisitMathOp(div, true);
  }

  void VisitMathOp(HBinaryOperation* bin_op, bool float_only) {
    // Does this operation have a FP constant?
    if (BinaryFP(bin_op)) {
      // Nothing more that we can do.
      return;
    }

    // Can we convert to a HInstructionRHSMemory variant?
    HInstruction* rhs = bin_op->GetRight();
    HInstruction* lhs = bin_op->GetLeft();
    if (TryMemoryOperation(bin_op, lhs, rhs, float_only)) {
      // We did it.
      return;
    }

    // We didn't use the RHS. Can we use the LHS?
    if (bin_op->IsCommutative()) {
      TryMemoryOperation(bin_op, rhs, lhs, float_only);
    }
  }

  bool IsSafeToReplaceWithMemOp(HInstruction* rhs, HBinaryOperation* bin_op) {
    // There is a case when we can't convert an op to mem variant.
    // Let's say we have the following instructions:
    // a = ArrayGet b, c
    // e = ArraySet b, c, f
    // g = Mul a, f
    // h = ArraySet b, c, g
    //
    // If we convert it into
    // e = ArraySet b, c, f
    // g = MulRHSMemory b, c, f
    // h = ArraySet b, c, g
    // then we violate flow dependency.
    // There is a similar case, when he have FieldGet/FieldSet
    // instead of ArrayGet/ArraySet.

    if (rhs->GetBlock() != bin_op->GetBlock()) {
      // For analysis simplification we want to need
      // rhs and binary op to be in the same block.
      return false;
    }

    for (HInstruction* instruction = rhs->GetNext();
         instruction != nullptr && instruction != bin_op;
         instruction = instruction->GetNext()) {
      switch (instruction->GetKind()) {
        case HInstruction::kArrayGet:
        case HInstruction::kInstanceFieldGet:
        case HInstruction::kStaticFieldGet:
          // A read is harmless, even if it is to the same address.
          break;
        default:
          if (alias_checker_.Alias(instruction, rhs) != AliasCheck::kNoAlias) {
            // This instruction can alias with the load.  Don't generate an
            // operation from memory.
            return false;
          }
          break;
      }
    }
    // No instruction between rhs and bin_op aliased with rhs.  It is safe to
    // combine the instructions into one.
    return true;
  }

  bool TryMemoryOperation(HBinaryOperation* bin_op,
                          HInstruction* lhs,
                          HInstruction* rhs,
                          bool float_only) {
    if (!rhs->HasOnlyOneNonEnvironmentUse()) {
      return false;
    }
    HInstruction::InstructionKind mem_kind = rhs->GetKind();
    switch (mem_kind) {
      case HInstruction::kInstanceFieldGet:
      case HInstruction::kStaticFieldGet: {
        const FieldInfo& field_info = (mem_kind == HInstruction::kStaticFieldGet) ?
                                        rhs->AsStaticFieldGet()->GetFieldInfo() :
                                        rhs->AsInstanceFieldGet()->GetFieldInfo();
        if (field_info.IsVolatile()) {
          // Ignore volatiles.
          break;
        }

        if (!IsSafeToReplaceWithMemOp(rhs, bin_op)) {
          // Conversion into memory operation is not permitted.
          break;
        }

        Primitive::Type field_type = field_info.GetFieldType();
        uint32_t offset = field_info.GetFieldOffset().Uint32Value();

        HInstructionRHSMemory* new_insn = nullptr;
        HInstruction* base = nullptr;
        switch (field_type) {
          case Primitive::kPrimInt:
            if (!float_only) {
              base = rhs->InputAt(0);
              new_insn = GetRHSMemory(bin_op, rhs, lhs, base, nullptr, offset);
            }
            break;
          case Primitive::kPrimFloat:
          case Primitive::kPrimDouble:
            base = rhs->InputAt(0);
            new_insn = GetRHSMemory(bin_op, rhs, lhs, base, nullptr, offset);
            break;
          default:
            // Unsupported type.
            break;
        }
        if (new_insn) {
          if (mem_kind == HInstruction::kStaticFieldGet) {
            new_insn->SetFromStatic();
          }
          bin_op->GetBlock()->ReplaceAndRemoveInstructionWith(bin_op, new_insn);
          rhs->GetBlock()->RemoveInstruction(rhs);
          DCHECK(base != nullptr);
          if (rhs->HasEnvironment()) {
            new_insn->CopyEnvironmentFrom(rhs->GetEnvironment());
          }
          return true;
        }
        break;
      }
      case HInstruction::kArrayGet: {
        if (!IsSafeToReplaceWithMemOp(rhs, bin_op)) {
          // Conversion into memory operation is not permitted.
          break;
        }

        HArrayGet* get = rhs->AsArrayGet();
        if (get == nullptr) { // Paranoid.
          return false;
        }

        Primitive::Type type = get->GetArrayType();
        uint32_t data_offset = GetArrayOffset(type);
        HInstruction* new_insn = nullptr;
        HInstruction* base = nullptr;

        switch (type) {
          case Primitive::kPrimInt:
            if (!float_only) {
              base = get->GetArray();
              new_insn = GetRHSMemory(bin_op, get, lhs, base, get->GetIndex(), data_offset);
            }
            break;
          case Primitive::kPrimFloat:
          case Primitive::kPrimDouble:
            base = get->GetArray();
            new_insn = GetRHSMemory(bin_op, get, lhs, base, get->GetIndex(), data_offset);
            break;
          default:
            // Unsupported type.
            break;
        }
        if (new_insn) {
          bin_op->GetBlock()->ReplaceAndRemoveInstructionWith(bin_op, new_insn);
          rhs->GetBlock()->RemoveInstruction(rhs);
          DCHECK(base != nullptr);
          if (rhs->HasEnvironment()) {
            new_insn->CopyEnvironmentFrom(rhs->GetEnvironment());
          }
          return true;
        }
        break;
      }
      default:
        break;
    }

    // Failed to convert to a memory operation.
    return false;
  }

  uint32_t GetArrayOffset(Primitive::Type type) {
    switch (type) {
      case Primitive::kPrimInt:
        return art::mirror::Array::DataOffset(sizeof(int32_t)).Uint32Value();
      case Primitive::kPrimFloat:
        return art::mirror::Array::DataOffset(sizeof(float)).Uint32Value();
      case Primitive::kPrimDouble:
        return art::mirror::Array::DataOffset(sizeof(double)).Uint32Value();
      default:
        // Unsupported type.
        return 0;
    }
  }

  void TryConvertConstantIndexToOffset(Primitive::Type type,
                                       HInstruction*& index,
                                       uint32_t& data_offset) {
    HInstruction* temp_index = index;
    if (temp_index->IsBoundsCheck()) {
      temp_index = temp_index->InputAt(0);
    }
    if (!temp_index->IsIntConstant()) {
      return;
    }
    int32_t index_value = temp_index->AsIntConstant()->GetValue();

    // Scale the index by the type.
    switch (type) {
      case Primitive::kPrimInt:
        index_value *= sizeof(int32_t);
        break;
      case Primitive::kPrimFloat:
        index_value *= sizeof(float);
        break;
      case Primitive::kPrimDouble:
        index_value *= sizeof(double);
        break;
      default:
        break;
    }

    // Replace the index with adjusted offset.
    index = nullptr;
    data_offset += index_value;
  }

  HInstructionRHSMemory* GetRHSMemory(HBinaryOperation* bin_op, HInstruction* get,
                                      HInstruction* lhs, HInstruction* base,
                                      HInstruction* index, uint32_t data_offset) {
    ArenaAllocator* arena = GetGraph()->GetArena();
    if (index != nullptr) {
      TryConvertConstantIndexToOffset(bin_op->GetType(), index, data_offset);
    }

    // Is this valid to do a null check on?
    if (base->IsNullCheck() && get->IsInstanceFieldGet() && data_offset >= kPageSize) {
      // We can't use an implicit null check. Just use the original instruction.
      return nullptr;
    }

    HInstructionRHSMemory* result = nullptr;
    switch (bin_op->GetKind()) {
      case HInstruction::kAdd:
        result = new (arena) HAddRHSMemory(bin_op->GetType(), lhs, base, index, data_offset);
        break;
      case HInstruction::kSub:
        result = new (arena) HSubRHSMemory(bin_op->GetType(), lhs, base, index, data_offset);
        break;
      case HInstruction::kMul:
        result = new (arena) HMulRHSMemory(bin_op->GetType(), lhs, base, index, data_offset);
        break;
      case HInstruction::kDiv:
        result = new (arena) HDivRHSMemory(bin_op->GetType(), lhs, base, index, data_offset);
        break;
      default:
        LOG(FATAL) << "Unexpected type " << bin_op->GetType();
        break;
    }

    return result;
  }

  void TryForLHSMemory(HInstruction* instruction) {
    HInstruction* previous = instruction->GetPrevious();
    if (previous) {
      HInstructionRHSMemory* rhs_mem = previous->AsInstructionRHSMemory();
      if (rhs_mem) {
        TryMemoryOperation(instruction, rhs_mem);
      }
    }
  }

  void HandleFieldSet(HInstruction* insn) {
    // Can we combine this with a preceding RHSMemory operation?
    TryForLHSMemory(insn);
  }

  void VisitStaticFieldSet(HStaticFieldSet* instruction) OVERRIDE {
    HandleFieldSet(instruction);
  }

  void VisitInstanceFieldSet(HInstanceFieldSet* instruction) OVERRIDE {
    HandleFieldSet(instruction);
  }

  void VisitArraySet(HArraySet* insn) OVERRIDE {
    // Can we combine this with a preceding RHSMemory operation?
    TryForLHSMemory(insn);
  }

  void TryMemoryOperation(HInstruction* instruction, HInstructionRHSMemory* rhs_mem_op) {
    // Does this instruction use the result of the rhs_mem_op?
    HInstruction* input = instruction->InputAt(instruction->InputCount() - 1);
    if (input != rhs_mem_op) {
      // The result of the RHS mem op is not the input to the Set.
      return;
    }

    // We can't support FP operations to memory.
    if (Primitive::IsFloatingPointType(rhs_mem_op->InputAt(0)->GetType())) {
      return;
    }

    // We only support add to memory.
    if (!rhs_mem_op->IsAddRHSMemory()) {
      return;
    }

    // Are these instructions compatible?
    if (HInstanceFieldSet* i_set = instruction->AsInstanceFieldSet()) {
      // The the memory operation can't have an index.
      if (rhs_mem_op->InputCount() != 2) {
        return;
      }
      // We need to match on base and offset.
      if (i_set->InputAt(0) != rhs_mem_op->InputAt(1) ||
          i_set->GetFieldOffset().Uint32Value() != rhs_mem_op->GetOffset()) {
        return;
      }
    } else if (HStaticFieldSet* a_set = instruction->AsStaticFieldSet()) {
      // The the memory operation can't have an index.
      if (rhs_mem_op->InputCount() != 2) {
        return;
      }
      // We need to match on base and offset.
      if (a_set->InputAt(0) != rhs_mem_op->InputAt(1) ||
          a_set->GetFieldOffset().Uint32Value() != rhs_mem_op->GetOffset()) {
        return;
      }
    } else {
      DCHECK(instruction->IsArraySet());
      // Does the memory operation have an index?
      if (rhs_mem_op->InputCount() != 3) {
        return;
      }
      // We need to match on base and index.
      HArraySet* set = instruction->AsArraySet();
      if (UNLIKELY(set == nullptr)) {
        return;
      }
      if (set->GetArray() != rhs_mem_op->InputAt(1) ||
          set->GetIndex() != rhs_mem_op->InputAt(2)) {
        return;
      }
    }

    HInstruction* new_rhs = rhs_mem_op->InputAt(0);
    ArenaAllocator* arena = GetGraph()->GetArena();
    HInstructionLHSMemory* new_insn =
        new (arena) HAddLHSMemory(rhs_mem_op, new_rhs, rhs_mem_op->GetDexPc());

    // Go ahead and do the replacement.
    HBasicBlock* block = instruction->GetBlock();
    block->ReplaceAndRemoveInstructionWith(instruction, new_insn);

    // Remove the old RHS memory op if there is no other use of it.
    if (!rhs_mem_op->HasUses()) {
      if (rhs_mem_op->HasEnvironment()) {
        new_insn->CopyEnvironmentFrom(rhs_mem_op->GetEnvironment());
      }
      block->RemoveInstruction(rhs_mem_op);
    }
  }

  void VisitBoundsCheck(HBoundsCheck* check) OVERRIDE {
    // Replace the length by the array itself, so that we can do compares to memory.
    HArrayLength* array_len = check->InputAt(1)->AsArrayLength();

    // We only want to replace an ArrayLength.
    if (array_len == nullptr) {
      return;
    }

    HInstruction* array = array_len->InputAt(0);
    DCHECK_EQ(array->GetType(), Primitive::kPrimNot);

    // Don't apply this optimization when the array is nullptr.
    HInstruction* array_base = array;
    while (array_base->IsNullCheck() || array_base->IsBoundType()) {
      array_base = array_base->InputAt(0);
    }
    if (array_base->IsConstant()) {
      return;
    }

    // Is there a null check that could be an implicit check?
    if (array->IsNullCheck() && do_implicit_null_checks_) {
      // The ArrayLen may generate the implicit null check.  Can the
      // bounds check do so as well?
      if (array_len->GetNextDisregardingMoves() != check) {
        // No, it won't.  Leave as is.
        return;
      }
    }

    // Can we remove the ArrayLength?
    if (array_len->HasOnlyOneNonEnvironmentUse()) {
      HX86BoundsCheckMemory* new_check =
        new (GetGraph()->GetArena()) HX86BoundsCheckMemory(check->InputAt(0), array, check->GetDexPc());
      check->GetBlock()->InsertInstructionBefore(new_check, check);
      check->ReplaceWith(new_check);
      DCHECK(check->GetEnvironment() != nullptr);
      new_check->CopyEnvironmentFrom(check->GetEnvironment());
      check->GetBlock()->RemoveInstruction(check);
      array_len->GetBlock()->RemoveInstruction(array_len);
    }
  }

  bool do_implicit_null_checks_;

  // Alias checker for RHSMemory generation.
  AliasCheck alias_checker_;
};

X86MemoryOperandGeneration::X86MemoryOperandGeneration(HGraph* graph,
                                                       OptimizingCompilerStats* stats,
                                                       CodeGenerator* codegen)
    : HOptimization(graph, kPassNameX86MemOpGen, stats),
      do_implicit_null_checks_(codegen->GetCompilerOptions().GetImplicitNullChecks()) {
}

void X86MemoryOperandGeneration::Run() {
  MemoryOperandVisitor visitor(graph_, do_implicit_null_checks_);
  visitor.VisitInsertionOrder();
}

}  // namespace x86
}  // namespace art
