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

#include "cloning.h"
#include "ext_utility.h"

namespace art {

bool HInstructionCloner::AllOkay() const {
  if (kIsDebugBuild && all_cloned_okay_) {
    for (auto it : orig_to_clone_) {
      CHECK(it.first != nullptr);
      CHECK(it.second != nullptr);
      // The cloner should always create the same type unless the clone
      // was manually added. We check that now.
      if (manual_clones_.find(it.first) == manual_clones_.end()) {
        CHECK_EQ(it.first->GetKind(), it.second->GetKind()) << GetMethodName(GetGraph());
        CHECK_EQ(it.first->HasEnvironment(), it.second->HasEnvironment())
          << GetMethodName(GetGraph());
        if (it.first->CanBeMoved()) {
          if (!it.first->IsInvoke()
              || it.first->AsInvoke()->GetIntrinsic() != Intrinsics::kNone) {
            CHECK(it.first->InstructionDataEquals(it.second)) << it.first;
          } else {
            // TODO: Google implementation of InstructionDataEquals for invokes
            // compares intrinsics. We may have pure invokes that are movable,
            // but are not intrinsics. Maybe this needs to be changed in Google
            // code, but so far it is unclear what exactly should be
            // checked for pure methods and if we need this check at all.
            CHECK(it.first->IsInvokeStaticOrDirect());
            CHECK(it.second->IsInvokeStaticOrDirect());
            CHECK(it.first->AsInvokeStaticOrDirect()->IsPure());
            CHECK(it.second->AsInvokeStaticOrDirect()->IsPure());
          }
        }
      }
    }
  }

  return all_cloned_okay_;
}

void HInstructionCloner::GetInputsForUnary(HInstruction* instr, HInstruction** input_ptr) const {
  DCHECK(input_ptr != nullptr);
  DCHECK_EQ(instr->InputCount(), 1u);

  *input_ptr = instr->InputAt(0);

  if (use_cloned_inputs_) {
    // Look to see if this input has a clone.
    auto input_it = orig_to_clone_.find(*input_ptr);

    // If there is a clone, make it so the new inputs are from clone.
    if (input_it != orig_to_clone_.end()) {
      *input_ptr = input_it->second;
    }
  }
}

void HInstructionCloner::GetInputsForBinary(HInstruction* instr, HInstruction** lhs_ptr,
                                            HInstruction** rhs_ptr) const {
  DCHECK(lhs_ptr != nullptr);
  DCHECK(rhs_ptr != nullptr);
  DCHECK_EQ(instr->InputCount(), 2u);

  *lhs_ptr = instr->InputAt(0);
  *rhs_ptr = instr->InputAt(1);

  if (use_cloned_inputs_) {
    // Look to see if this input has a clone.
    auto lhs_it = orig_to_clone_.find(*lhs_ptr);
    auto rhs_it = orig_to_clone_.find(*rhs_ptr);

    // If there is a clone, make it so the new inputs are from clone.
    if (lhs_it != orig_to_clone_.end()) {
      *lhs_ptr = lhs_it->second;
    }
    if (rhs_it != orig_to_clone_.end()) {
      *rhs_ptr = rhs_it->second;
    }
  }
}

void HInstructionCloner::GetInputsForTernary(HInstruction* instr,
                                             HInstruction** input0_ptr,
                                             HInstruction** input1_ptr,
                                             HInstruction** input2_ptr) const {
  DCHECK(input0_ptr != nullptr);
  DCHECK(input1_ptr != nullptr);
  DCHECK(input2_ptr != nullptr);
  DCHECK_EQ(instr->InputCount(), 3u);

  *input0_ptr = instr->InputAt(0);
  *input1_ptr = instr->InputAt(1);
  *input2_ptr = instr->InputAt(2);

  if (use_cloned_inputs_) {
    // Look to see if this input has a clone.
    auto i0_it = orig_to_clone_.find(*input0_ptr);
    auto i1_it = orig_to_clone_.find(*input1_ptr);
    auto i2_it = orig_to_clone_.find(*input2_ptr);

    // If there is a clone, make it so the new inputs are from clone.
    if (i0_it != orig_to_clone_.end()) {
      *input0_ptr = i0_it->second;
    }
    if (i1_it != orig_to_clone_.end()) {
      *input1_ptr = i1_it->second;
    }
    if (i2_it != orig_to_clone_.end()) {
      *input2_ptr = i2_it->second;
    }
  }
}

void HInstructionCloner::GetInputsForQuaternary(HInstruction* instr,
                                                HInstruction** input0_ptr,
                                                HInstruction** input1_ptr,
                                                HInstruction** input2_ptr,
                                                HInstruction** input3_ptr) const {
  DCHECK(input0_ptr != nullptr);
  DCHECK(input1_ptr != nullptr);
  DCHECK(input2_ptr != nullptr);
  DCHECK(input3_ptr != nullptr);
  DCHECK_EQ(instr->InputCount(), 4u);

  *input0_ptr = instr->InputAt(0);
  *input1_ptr = instr->InputAt(1);
  *input2_ptr = instr->InputAt(2);
  *input3_ptr = instr->InputAt(3);

  if (use_cloned_inputs_) {
    // Look to see if this input has a clone.
    auto i0_it = orig_to_clone_.find(*input0_ptr);
    auto i1_it = orig_to_clone_.find(*input1_ptr);
    auto i2_it = orig_to_clone_.find(*input2_ptr);
    auto i3_it = orig_to_clone_.find(*input3_ptr);

    // If there is a clone, make it so the new inputs are from clone.
    if (i0_it != orig_to_clone_.end()) {
      *input0_ptr = i0_it->second;
    }
    if (i1_it != orig_to_clone_.end()) {
      *input1_ptr = i1_it->second;
    }
    if (i2_it != orig_to_clone_.end()) {
      *input2_ptr = i2_it->second;
    }
    if (i3_it != orig_to_clone_.end()) {
      *input3_ptr = i3_it->second;
    }
  }
}

void HInstructionCloner::GetInputsForMany(HInstruction* instr,
                                          std::vector<HInstruction*>& inputs) const {
  for (size_t i = 0, e = instr->InputCount(); i < e; ++i) {
    HInstruction* input = instr->InputAt(i);

    if (use_cloned_inputs_) {
      auto it = orig_to_clone_.find(input);
      if (it != orig_to_clone_.end()) {
        inputs.push_back(it->second);
      } else {
        inputs.push_back(input);
      }
    } else {
      inputs.push_back(input);
    }
  }
}

HEnvironment* HInstructionCloner::CloneEnvironment(HEnvironment* env, HInstruction* clone) {
  DCHECK_EQ(cloning_enabled_, true);

  HEnvironment* env_clone_base = nullptr;
  HEnvironment* env_clone;

  for (HEnvironment* env_clone_prev = nullptr;
       env != nullptr;
       env_clone_prev = env_clone, env = env->GetParent()) {
    env_clone = new (arena_) HEnvironment(arena_, env->Size(),
        env->GetDexFile(), env->GetMethodIdx(), env->GetDexPc(),
        env->GetInvokeType(), clone);

    if (env_clone_prev != nullptr) {
      env_clone_prev->SetParent(env_clone);
    } else {
      // This case is hit only if this is the first environment being cloned.
      // This means that the newly created environment is the base.
      env_clone_base = env_clone;
    }

    for (size_t i = 0; i < env->Size(); i++) {
      HInstruction* instruction = env->GetInstructionAt(i);

      if (use_cloned_inputs_) {
        // Now check to see if this input was cloned.
        auto clone_it = orig_to_clone_.find(instruction);
        if (clone_it != orig_to_clone_.end()) {
          instruction = clone_it->second;
        }
      }

      // Now link the inputs to the environment.
      env_clone->SetRawEnvAt(i, instruction);
      if (instruction != nullptr) {
        instruction->AddEnvUseAt(env_clone, i, arena_);
      }
    }
  }

  DCHECK(env_clone_base != nullptr);
  return env_clone_base;
}

void HInstructionCloner::CloneEnvironment(HInstruction* instr, HInstruction* clone) {
  if (instr->HasEnvironment()) {
    HEnvironment* env_clone = CloneEnvironment(instr->GetEnvironment(), clone);
    clone->SetRawEnvironment(env_clone);
  }
}

static void CopyReferenceType(HInstruction* instr, HInstruction* clone) {
  if (instr->GetType() == Primitive::kPrimNot) {
    clone->SetReferenceTypeInfo(instr->GetReferenceTypeInfo());
  }
}

static void CopyIntrinsicInformation(HInvoke* instr, HInvoke* clone) {
  if (instr->IsIntrinsic()) {
    // Determine the side effects of the intrinsic.
    SideEffects side_effects = instr->GetSideEffects();
    IntrinsicSideEffects intrinsic_side_effects = kNoSideEffects;
    if (side_effects.Includes(SideEffects::AllExceptGCDependency())) {
      intrinsic_side_effects = kAllSideEffects;
    } else if (side_effects.Includes(SideEffects::AllWrites())) {
      intrinsic_side_effects = kWriteSideEffects;
    } else if (side_effects.Includes(SideEffects::AllReads())) {
      intrinsic_side_effects = kReadSideEffects;
    }
    IntrinsicExceptions can_throw = instr->CanThrow() ? kCanThrow : kNoThrow;
    IntrinsicNeedsEnvironmentOrCache needs_env_or_dex_cache =
        instr->NeedsDexCacheOfDeclaringClass() || instr->NeedsEnvironment() ?
            kNeedsEnvironmentOrCache : kNoEnvironmentOrCache;

    clone->SetIntrinsic(instr->GetIntrinsic(),
                        needs_env_or_dex_cache,
                        intrinsic_side_effects,
                        can_throw);
  }
}

void HInstructionCloner::VisitAbove(HAbove* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HAbove* clone = new (arena_) HAbove(lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitAboveOrEqual(HAboveOrEqual* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HAboveOrEqual* clone = new (arena_) HAboveOrEqual(lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitAdd(HAdd* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HAdd* clone = new (arena_) HAdd(instr->GetResultType(), lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitAnd(HAnd* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HAnd* clone = new (arena_) HAnd(instr->GetResultType(), lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitArrayGet(HArrayGet* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* array, *index;
    GetInputsForBinary(instr, &array, &index);
    HArrayGet* clone = new (arena_) HArrayGet(array, index, instr->GetArrayType(),
                                              instr->GetDexPc(), instr->IsUnsigned(),
                                              instr->GetSideEffects());
    CopyReferenceType(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitArrayLength(HArrayLength* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HArrayLength* clone = new (arena_) HArrayLength(input, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitArraySet(HArraySet* instr) {
  if (cloning_enabled_) {
    HInstruction* array, *index, *value;
    GetInputsForTernary(instr, &array, &index, &value);
    HArraySet* clone = new (arena_) HArraySet(array, index, value,
        instr->GetComponentType(), instr->GetDexPc(), instr->GetSideEffects());

    if (instr->GetUseNonTemporalMove()) {
      clone->SetUseNonTemporalMove();
    }
    if (instr->StaticTypeOfArrayIsObjectArray()) {
      clone->SetStaticTypeOfArrayIsObjectArray();
    }
    if (!instr->NeedsTypeCheck()) {
      clone->ClearNeedsTypeCheck();
    }
    if (!instr->GetValueCanBeNull()) {
      clone->ClearValueCanBeNull();
    }

    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitBelow(HBelow* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HBelow* clone = new (arena_) HBelow(lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitBelowOrEqual(HBelowOrEqual* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HBelowOrEqual* clone = new (arena_) HBelowOrEqual(lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitBooleanNot(HBooleanNot* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HBooleanNot* clone = new (arena_) HBooleanNot(input, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitBoundsCheck(HBoundsCheck* instr) {
  if (cloning_enabled_) {
    HInstruction* index, *length;
    GetInputsForBinary(instr, &index, &length);
    HBoundsCheck* clone = new (arena_) HBoundsCheck(index, length, instr->GetDexPc());
    CopyReferenceType(instr, clone);
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitBoundType(HBoundType* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HBoundType* clone = new (arena_) HBoundType(input, instr->GetDexPc());
    clone->SetUpperBound(instr->GetUpperBound(), instr->GetUpperCanBeNull());
    clone->SetCanBeNull(instr->CanBeNull());
    CopyReferenceType(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitCheckCast(HCheckCast* instr) {
  if (cloning_enabled_) {
    HInstruction* object, *constant;
    GetInputsForBinary(instr, &object, &constant);
    DCHECK(constant->IsLoadClass());
    HCheckCast* clone = new (arena_) HCheckCast(object, constant->AsLoadClass(),
        instr->GetTypeCheckKind(), instr->GetDexPc());
    if (!instr->MustDoNullCheck()) {
      clone->ClearMustDoNullCheck();
    }
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitClassTableGet(HClassTableGet* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HClassTableGet* clone = new (arena_) HClassTableGet(input,
                                                        instr->GetType(),
                                                        instr->GetTableKind(),
                                                        instr->GetIndex(),
                                                        instr->GetDexPc());
    CopyReferenceType(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitClearException(HClearException* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HClearException* clone = new (arena_) HClearException(instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitClinitCheck(HClinitCheck* instr) {
  if (cloning_enabled_) {
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    DCHECK_EQ(input->IsLoadClass(), true);
    HClinitCheck* clone = new (arena_) HClinitCheck(input->AsLoadClass(), instr->GetDexPc());
    CopyReferenceType(instr, clone);
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitCompare(HCompare* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    DCHECK_EQ(lhs->GetType(), rhs->GetType());
    HCompare* clone = new (arena_) HCompare(lhs->GetType(),
                                            lhs,
                                            rhs,
                                            instr->GetBias(),
                                            instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitDeoptimize(HDeoptimize* instr) {
  if (cloning_enabled_) {
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HDeoptimize* clone = new (arena_) HDeoptimize(input, instr->GetDexPc());
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitDevirtGuard(HDevirtGuard* instr) {
  if (cloning_enabled_) {
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    DCHECK(lhs->IsLoadClass());
    HLoadClass* devirt_class = lhs->AsLoadClass();
    HDevirtGuard* clone = new (arena_) HDevirtGuard(devirt_class, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitDiv(HDiv* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HDiv* clone = new (arena_) HDiv(instr->GetResultType(), lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitDivZeroCheck(HDivZeroCheck* instr) {
  if (cloning_enabled_) {
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HDivZeroCheck* clone = new (arena_) HDivZeroCheck(input, instr->GetDexPc());
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitEqual(HEqual* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HEqual* clone = new (arena_) HEqual(lhs, rhs, instr->GetDexPc());
    clone->SetBias(instr->GetBias());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitGoto(HGoto* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HGoto* clone = new (arena_) HGoto(instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitGreaterThan(HGreaterThan* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HGreaterThan* clone = new (arena_) HGreaterThan(lhs, rhs, instr->GetDexPc());
    clone->SetBias(instr->GetBias());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitGreaterThanOrEqual(HGreaterThanOrEqual* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HGreaterThanOrEqual* clone = new (arena_) HGreaterThanOrEqual(lhs, rhs, instr->GetDexPc());
    clone->SetBias(instr->GetBias());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitIf(HIf* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HIf* clone = new (arena_) HIf(input, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitInstanceFieldGet(HInstanceFieldGet* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    const FieldInfo& fi = instr->GetFieldInfo();
    HInstanceFieldGet* clone =
        new (arena_) HInstanceFieldGet(input,
                                       instr->GetFieldType(),
                                       instr->GetFieldOffset(),
                                       instr->IsVolatile(),
                                       fi.GetFieldIndex(),
                                       fi.GetDeclaringClassDefIndex(),
                                       fi.GetDexFile(),
                                       fi.GetDexCache(),
                                       instr->GetDexPc());
    CopyReferenceType(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitInstanceFieldSet(HInstanceFieldSet* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* object, *value;
    GetInputsForBinary(instr, &object, &value);
    const FieldInfo& fi = instr->GetFieldInfo();
    HInstanceFieldSet* clone =
        new (arena_) HInstanceFieldSet(object,
                                       value,
                                       instr->GetFieldType(),
                                       instr->GetFieldOffset(),
                                       instr->IsVolatile(),
                                       fi.GetFieldIndex(),
                                       fi.GetDeclaringClassDefIndex(),
                                       fi.GetDexFile(),
                                       fi.GetDexCache(),
                                       instr->GetDexPc());
    if (!instr->GetValueCanBeNull()) {
      clone->ClearValueCanBeNull();
    }
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitInstanceOf(HInstanceOf* instr) {
  if (cloning_enabled_) {
    HInstruction* object, *constant;
    GetInputsForBinary(instr, &object, &constant);
    DCHECK(constant->IsLoadClass());
    HInstanceOf* clone = new (arena_) HInstanceOf(object, constant->AsLoadClass(),
        instr->GetTypeCheckKind(), instr->GetDexPc());
    if (!instr->MustDoNullCheck()) {
      clone->ClearMustDoNullCheck();
    }
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::FinishInvokeCloning(HInvoke* instr, HInvoke* clone) {
  CopyIntrinsicInformation(instr, clone);
  CopyReferenceType(instr, clone);

  std::vector<HInstruction*> inputs;
  GetInputsForMany(instr, inputs);
  DCHECK_EQ(instr->GetNumberOfArguments(), clone->GetNumberOfArguments());
  DCHECK_EQ(instr->InputCount(), clone->InputCount());
  for (size_t i = 0, e = instr->InputCount(); i < e; ++i) {
    clone->SetArgumentAt(i, inputs[i]);
  }

  CloneEnvironment(instr, clone);
  CommitClone(instr, clone);
}

void HInstructionCloner::VisitInvokeInterface(HInvokeInterface* instr) {
  if (cloning_enabled_) {
    HInvokeInterface* clone = new (arena_) HInvokeInterface(arena_,
                                                            instr->GetNumberOfArguments(),
                                                            instr->GetType(),
                                                            instr->GetDexPc(),
                                                            instr->GetDexMethodIndex(),
                                                            instr->GetImtIndex());
    FinishInvokeCloning(instr, clone);
  }
}

void HInstructionCloner::VisitInvokeStaticOrDirect(HInvokeStaticOrDirect* instr) {
  if (cloning_enabled_) {
    HInvokeStaticOrDirect* clone =
        new (arena_) HInvokeStaticOrDirect(arena_,
                                           instr->GetNumberOfArguments(),
                                           instr->GetType(),
                                           instr->GetDexPc(),
                                           instr->GetDexMethodIndex(),
                                           instr->GetTargetMethod(),
                                           instr->GetDispatchInfoUnchecked(),
                                           instr->GetOriginalInvokeType(),
                                           instr->GetOptimizedInvokeType(),
                                           instr->GetClinitCheckRequirement());
    clone->TrimInputCapacity(instr->InputCount());
    if (instr->IsPure()) {
      clone->SetPure();
      clone->SetSideEffects(instr->GetSideEffects());
    }
    FinishInvokeCloning(instr, clone);
  }
}

void HInstructionCloner::VisitInvokeVirtual(HInvokeVirtual* instr) {
  if (cloning_enabled_) {
    HInvokeVirtual* clone = new (arena_) HInvokeVirtual(arena_,
                                                        instr->GetNumberOfArguments(),
                                                        instr->GetType(),
                                                        instr->GetDexPc(),
                                                        instr->GetDexMethodIndex(),
                                                        instr->GetVTableIndex());
    FinishInvokeCloning(instr, clone);
  }
}

void HInstructionCloner::VisitInvokeUnresolved(HInvokeUnresolved* instr) {
  if (cloning_enabled_) {
    HInvokeUnresolved* clone = new (arena_) HInvokeUnresolved(arena_,
                                                              instr->GetNumberOfArguments(),
                                                              instr->GetType(),
                                                              instr->GetDexPc(),
                                                              instr->GetDexMethodIndex(),
                                                              instr->GetOriginalInvokeType());
    FinishInvokeCloning(instr, clone);
  }
}

void HInstructionCloner::VisitLessThan(HLessThan* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HLessThan* clone = new (arena_) HLessThan(lhs, rhs, instr->GetDexPc());
    clone->SetBias(instr->GetBias());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitLessThanOrEqual(HLessThanOrEqual* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HLessThanOrEqual* clone = new (arena_) HLessThanOrEqual(lhs, rhs, instr->GetDexPc());
    clone->SetBias(instr->GetBias());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitLoadClass(HLoadClass* instr) {
  if (cloning_enabled_) {
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    DCHECK(input->IsCurrentMethod());
    HLoadClass* clone =
        new (arena_) HLoadClass(input->AsCurrentMethod(),
                                instr->GetTypeIndex(),
                                instr->GetDexFile(),
                                instr->IsReferrersClass(),
                                instr->GetDexPc(),
                                instr->NeedsAccessCheck(),
                                instr->IsInDexCache());
    CopyReferenceType(instr, clone);

    if (instr->GetLoadedClassRTI().IsExact()) {
      clone->SetLoadedClassRTI(instr->GetLoadedClassRTI());
    }
    if (instr->MustGenerateClinitCheck()) {
      clone->SetMustGenerateClinitCheck(true);
    }

    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitLoadException(HLoadException* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HLoadException* clone = new (arena_) HLoadException(instr->GetDexPc());
    CopyReferenceType(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitLoadString(HLoadString* instr) {
  if (cloning_enabled_) {
    HLoadString* clone;
    HLoadString::LoadKind load_kind = instr->GetLoadKind();
    uint32_t string_index = (instr->HasStringReference(load_kind) ||
                            /* For slow paths. */ !instr->IsInDexCache()) ?
                            instr->GetStringIndex() : 0;

    switch (load_kind) {
      case HLoadString::LoadKind::kDexCacheViaMethod:
        HInstruction* input;
        GetInputsForUnary(instr, &input);
        DCHECK(input->IsCurrentMethod());
        clone = new (arena_) HLoadString(input->AsCurrentMethod(),
                                         string_index,
                                         instr->GetDexFile(),
                                         instr->GetDexPc());
        break;
      case HLoadString::LoadKind::kBootImageLinkTimeAddress:
      case HLoadString::LoadKind::kBootImageLinkTimePcRelative:
        clone = new (arena_) HLoadString(load_kind,
                                         string_index,
                                         instr->GetDexFile(),
                                         instr->GetDexPc());
        break;
      case HLoadString::LoadKind::kBootImageAddress:
      case HLoadString::LoadKind::kDexCacheAddress:
        clone = new (arena_) HLoadString(load_kind,
                                         string_index,
                                         instr->GetAddress(),
                                         instr->GetDexPc());
        break;
      case HLoadString::LoadKind::kDexCachePcRelative:
        clone = new (arena_) HLoadString(load_kind,
                                         string_index,
                                         instr->GetDexFile(),
                                         instr->GetDexCacheElementOffset(),
                                         instr->GetDexPc());
        break;
    }

    if (instr->IsInDexCache()) {
      clone->MarkInDexCache();
    }

    CopyReferenceType(instr, clone);
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitMemoryBarrier(HMemoryBarrier* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HMemoryBarrier* clone = new (arena_) HMemoryBarrier(instr->GetBarrierKind(),
                                                        instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitMonitorOperation(HMonitorOperation* instr) {
  if (cloning_enabled_) {
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HMonitorOperation::OperationKind kind = instr->IsEnter() ?
        HMonitorOperation::OperationKind::kEnter : HMonitorOperation::OperationKind::kExit;
    HMonitorOperation* clone = new (arena_) HMonitorOperation(input, kind, instr->GetDexPc());
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitMul(HMul* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HMul* clone = new (arena_) HMul(instr->GetResultType(), lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitNativeDebugInfo(HNativeDebugInfo* instr) {
  if (cloning_enabled_) {
    HNativeDebugInfo* clone = new (arena_) HNativeDebugInfo(instr->GetDexPc());
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitNeg(HNeg* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HNeg* clone = new (arena_) HNeg(instr->GetResultType(), input, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitNewArray(HNewArray* instr) {
  if (cloning_enabled_) {
    HInstruction* length, *curr_method;
    GetInputsForBinary(instr, &length, &curr_method);
    DCHECK(curr_method->IsCurrentMethod());
    HNewArray* clone = new (arena_) HNewArray(length,
                                              curr_method->AsCurrentMethod(),
                                              instr->GetDexPc(),
                                              instr->GetTypeIndex(),
                                              instr->GetDexFile(),
                                              instr->GetEntrypoint());
    CopyReferenceType(instr, clone);
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);

  }
}

void HInstructionCloner::VisitNewInstance(HNewInstance* instr) {
  if (cloning_enabled_) {
    HInstruction* cls, *curr_method;
    GetInputsForBinary(instr, &cls, &curr_method);
    DCHECK(curr_method->IsCurrentMethod());
    HNewInstance* clone = new (arena_) HNewInstance(cls,
                                                    curr_method->AsCurrentMethod(),
                                                    instr->GetDexPc(),
                                                    instr->GetTypeIndex(),
                                                    instr->GetDexFile(),
                                                    instr->CanThrow(),
                                                    instr->IsFinalizable(),
                                                    instr->GetEntrypoint());
    CopyReferenceType(instr, clone);
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitNot(HNot* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HNot* clone = new (arena_) HNot(instr->GetResultType(), input, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitNotEqual(HNotEqual* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HNotEqual* clone = new (arena_) HNotEqual(lhs, rhs, instr->GetDexPc());
    clone->SetBias(instr->GetBias());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitNullCheck(HNullCheck* instr) {
  if (cloning_enabled_) {
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HNullCheck* clone = new (arena_) HNullCheck(input, instr->GetDexPc());
    CopyReferenceType(instr, clone);
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitOr(HOr* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HOr* clone = new (arena_) HOr(instr->GetResultType(), lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitPackedSwitch(HPackedSwitch* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HPackedSwitch* clone = new (arena_) HPackedSwitch(instr->GetStartValue(),
                                                      instr->GetNumEntries(),
                                                      input,
                                                      instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitPhi(HPhi* phi) {
  if (cloning_enabled_) {
    DCHECK(!phi->HasEnvironment());
    HPhi* clone = new (arena_) HPhi(arena_, phi->GetRegNumber(), 0, phi->GetType());

    // Now copy the inputs while making sure to use input from clone (if one exists).
    for (size_t input_idx = 0u; input_idx != phi->InputCount(); input_idx++) {
      HInstruction* input = phi->InputAt(input_idx);
      auto it = orig_to_clone_.find(input);
      if (use_cloned_inputs_ && it != orig_to_clone_.end()) {
        clone->AddInputNoUseUpdate(it->second);
      } else {
        clone->AddInputNoUseUpdate(input);
      }
    }

    clone->SetCanBeNull(phi->CanBeNull());
    if (phi->IsDead()) {
      clone->SetDead();
    }

    CopyReferenceType(phi, clone);
    CommitClone(phi, clone);
  }
}

void HInstructionCloner::VisitSelect(HSelect* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* condition, *true_val, *false_val;
    GetInputsForTernary(instr, &false_val, &true_val, &condition);
    HSelect* clone = new (arena_) HSelect(condition,
                                          true_val,
                                          false_val,
                                          instr->GetDexPc());
    CopyReferenceType(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitRem(HRem* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HRem* clone = new (arena_) HRem(instr->GetResultType(), lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitReturn(HReturn* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HReturn* clone = new (arena_) HReturn(input, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitReturnVoid(HReturnVoid* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HReturnVoid* clone = new (arena_) HReturnVoid(instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitRor(HRor* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* value, *distance;
    GetInputsForBinary(instr, &value, &distance);
    HRor* clone = new (arena_) HRor(instr->GetResultType(), value, distance);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitShl(HShl* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HShl* clone = new (arena_) HShl(instr->GetResultType(), lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitShr(HShr* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HShr* clone = new (arena_) HShr(instr->GetResultType(), lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitStaticFieldGet(HStaticFieldGet* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    const FieldInfo& fi = instr->GetFieldInfo();
    HStaticFieldGet* clone =
        new (arena_) HStaticFieldGet(input,
                                     instr->GetFieldType(),
                                     instr->GetFieldOffset(),
                                     instr->IsVolatile(),
                                     fi.GetFieldIndex(),
                                     fi.GetDeclaringClassDefIndex(),
                                     fi.GetDexFile(),
                                     fi.GetDexCache(),
                                     instr->GetDexPc());
    CopyReferenceType(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitStaticFieldSet(HStaticFieldSet* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* cls, *value;
    GetInputsForBinary(instr, &cls, &value);
    const FieldInfo& fi = instr->GetFieldInfo();
    HStaticFieldSet* clone =
        new (arena_) HStaticFieldSet(cls,
                                     value,
                                     instr->GetFieldType(),
                                     instr->GetFieldOffset(),
                                     instr->IsVolatile(),
                                     fi.GetFieldIndex(),
                                     fi.GetDeclaringClassDefIndex(),
                                     fi.GetDexFile(),
                                     fi.GetDexCache(),
                                     instr->GetDexPc());
    if (!instr->GetValueCanBeNull()) {
      clone->ClearValueCanBeNull();
    }
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitSub(HSub* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HSub* clone = new (arena_) HSub(instr->GetResultType(), lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitSuspend(HSuspend* instr) {
  if (cloning_enabled_) {
    HSuspend* clone = new (arena_) HSuspend(instr->GetDexPc());
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitSuspendCheck(HSuspendCheck* instr) {
  if (cloning_enabled_) {
    HSuspendCheck* clone = new (arena_) HSuspendCheck(instr->GetDexPc());
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}


void HInstructionCloner::VisitTestSuspend(HTestSuspend* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HTestSuspend* clone = new (arena_) HTestSuspend(instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitThrow(HThrow* instr) {
  if (cloning_enabled_) {
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HThrow* clone = new (arena_) HThrow(input, instr->GetDexPc());
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitTryBoundary(HTryBoundary* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HTryBoundary::BoundaryKind boundary_kind = instr->IsEntry() ?
        HTryBoundary::BoundaryKind::kEntry : HTryBoundary::BoundaryKind::kExit;
    HTryBoundary* clone = new (arena_) HTryBoundary(boundary_kind, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitTypeConversion(HTypeConversion* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    if (instr->GetResultType() != input->GetType()) {
      HTypeConversion* clone = new (arena_) HTypeConversion(instr->GetResultType(), input,
          instr->GetDexPc());
      CopyReferenceType(instr, clone);
      CommitClone(instr, clone);
    } else {
      CopyReferenceType(instr, input);
      CommitClone(instr, input);
      if (kIsDebugBuild) {
        // Add this to manual clones since we elided the redundant type cast.
        manual_clones_.insert(instr);
      }
    }

  }
}

void HInstructionCloner::VisitUShr(HUShr* instr) {
  if (cloning_enabled_) {
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HUShr* clone = new (arena_) HUShr(instr->GetResultType(), lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitUnresolvedInstanceFieldGet(HUnresolvedInstanceFieldGet* instr) {
  if (cloning_enabled_) {
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    HUnresolvedInstanceFieldGet* clone =
        new (arena_) HUnresolvedInstanceFieldGet(input,
                                                 instr->GetFieldType(),
                                                 instr->GetFieldIndex(),
                                                 instr->GetDexPc());
    CopyReferenceType(instr, clone);
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitUnresolvedInstanceFieldSet(HUnresolvedInstanceFieldSet* instr) {
  if (cloning_enabled_) {
    HInstruction* object, *value;
    GetInputsForBinary(instr, &object, &value);
    HUnresolvedInstanceFieldSet* clone =
        new (arena_) HUnresolvedInstanceFieldSet(object,
                                                 value,
                                                 instr->GetFieldType(),
                                                 instr->GetFieldIndex(),
                                                 instr->GetDexPc());
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitUnresolvedStaticFieldGet(HUnresolvedStaticFieldGet* instr) {
  if (cloning_enabled_) {
    HUnresolvedStaticFieldGet* clone =
        new (arena_) HUnresolvedStaticFieldGet(instr->GetFieldType(),
                                               instr->GetFieldIndex(),
                                               instr->GetDexPc());
    CopyReferenceType(instr, clone);
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitUnresolvedStaticFieldSet(HUnresolvedStaticFieldSet* instr) {
  if (cloning_enabled_) {
    HInstruction* value;
    GetInputsForUnary(instr, &value);
    HUnresolvedStaticFieldSet* clone =
        new (arena_) HUnresolvedStaticFieldSet(value,
                                               instr->GetFieldType(),
                                               instr->GetFieldIndex(),
                                               instr->GetDexPc());
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitXor(HXor* instr) {
  if (cloning_enabled_) {
    HInstruction* lhs, *rhs;
    GetInputsForBinary(instr, &lhs, &rhs);
    HXor* clone = new (arena_) HXor(instr->GetResultType(), lhs, rhs, instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitX86SelectValue(HX86SelectValue* instr) {
  if (cloning_enabled_) {
    HInstruction* input_0, *input_1, *input_2, *input_3;
    GetInputsForQuaternary(instr, &input_0, &input_1, &input_2, &input_3);
    HX86SelectValue* clone =
      new (arena_) HX86SelectValue(instr->GetType(),
                                   instr->GetCondition(),
                                   instr->GetOppositeCondition(),
                                   input_0, input_1, input_2, input_3,
                                   instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitX86LoadFromConstantTable(HX86LoadFromConstantTable* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* method_base, *constant;
    GetInputsForBinary(instr, &method_base, &constant);
    DCHECK(method_base->IsX86ComputeBaseMethodAddress());
    DCHECK(constant->IsConstant());
    HX86LoadFromConstantTable* clone = new (arena_) HX86LoadFromConstantTable(
        method_base->AsX86ComputeBaseMethodAddress(),
        constant->AsConstant());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitX86FPNeg(HX86FPNeg* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input, *method_base;
    GetInputsForBinary(instr, &input, &method_base);
    DCHECK(method_base->IsX86ComputeBaseMethodAddress());
    HX86FPNeg* clone = new (arena_) HX86FPNeg(instr->GetType(),
                                              input,
                                              method_base->AsX86ComputeBaseMethodAddress(),
                                              instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitX86PackedSwitch(HX86PackedSwitch* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input, *method_base;
    GetInputsForBinary(instr, &input, &method_base);
    DCHECK(method_base->IsX86ComputeBaseMethodAddress());
    HX86PackedSwitch* clone = new (arena_) HX86PackedSwitch(
        instr->GetStartValue(),
        instr->GetNumEntries(),
        input,
        method_base->AsX86ComputeBaseMethodAddress(),
        instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitX86BoundsCheckMemory(HX86BoundsCheckMemory* instr) {
  if (cloning_enabled_) {
    HInstruction* index, *array;
    GetInputsForBinary(instr, &index, &array);
    HX86BoundsCheckMemory* clone = new (arena_) HX86BoundsCheckMemory(index,
                                                                      array,
                                                                      instr->GetDexPc());
    CopyReferenceType(instr, clone);
    CloneEnvironment(instr, clone);
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitX86IncrementExecutionCount(HX86IncrementExecutionCount* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* input;
    GetInputsForUnary(instr, &input);
    DCHECK(input->IsX86ReturnExecutionCountTable());
    HX86IncrementExecutionCount* clone =
        new (arena_) HX86IncrementExecutionCount(instr->GetBlockNumber(),
                                                 input->AsX86ReturnExecutionCountTable(),
                                                 instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

void HInstructionCloner::VisitX86ProfileInvoke(HX86ProfileInvoke* instr) {
  if (cloning_enabled_) {
    DCHECK(!instr->HasEnvironment());
    HInstruction* current_method, *object;
    GetInputsForBinary(instr, &current_method, &object);
    DCHECK(current_method->IsCurrentMethod());
    HX86ProfileInvoke* clone =
        new (arena_) HX86ProfileInvoke(instr->GetIndex(),
                                       current_method->AsCurrentMethod(),
                                       object,
                                       instr->GetDexPc());
    CommitClone(instr, clone);
  }
}

}  // namespace art
