/*
 * Copyright (C) 2016 Intel Corporation
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

#include "devirtualization.h"
#include "ext_profiling.h"
#include "ext_utility.h"
#include "scoped_thread_state_change.h"

namespace art {

bool HDevirtualization::Gate() const {
  if (graph_->GetArtMethod() == nullptr) {
    // We need to have art_method_ to work.
    return false;
  }
  return HSpeculationPass::Gate();
}

HDevirtualization::~HDevirtualization() {
  if (kIsDebugBuild) {
    for (auto it : imprecise_predictions_) {
      // We must have at least one prediction if in list of predictions.
      DCHECK_GT(it.second.size(), 0u);
      // It must be the case that our imprecise predictions are not considered precise also.
      DCHECK(precise_prediction_.find(it.first) == precise_prediction_.end());
    }
  }
}

bool HDevirtualization::IsCandidate(HInstruction* instr) {
  // Have we already been devirtualized?
  if (instr->IsInvoke() && instr->AsInvoke()->IsDevirtualized()) {
    return false;
  }
  if (instr->IsInvokeVirtual() || instr->IsInvokeInterface()) {
    // If the invoke is already marked as intrinsic, we do not need to sharpen it.
    return !instr->AsInvoke()->IsIntrinsic();
  }

  return false;
}

bool HDevirtualization::HasPrediction(HInstruction* instr, bool update) {
  DCHECK(IsCandidate(instr));
  // First check if we have already checked this prediction before.
  if (precise_prediction_.find(instr) != precise_prediction_.end()) {
    return true;
  } else if (imprecise_predictions_.find(instr) != imprecise_predictions_.end()) {
    return true;
  }

  // Try resolving the target of this call.
  HInvoke* invoke = instr->AsInvoke();
  uint32_t method_index = invoke->GetDexMethodIndex();
  ScopedObjectAccess soa(Thread::Current());
  ClassLinker* class_linker = compilation_unit_.GetClassLinker();
  ArtMethod* resolved_method =
      compilation_unit_.GetDexCache().Get()->GetResolvedMethod(method_index,
                                                               class_linker->GetImagePointerSize());

  if (resolved_method != nullptr) {
    // Now that we have a resolved method, try to find a potential target if the type
    // is provably precise.
    ArtMethod* actual_method = FindVirtualOrInterfaceTarget(invoke, resolved_method);

    if (actual_method != nullptr) {
      if (update) {
        TypeHandle type;
        if (IsMethodOrDeclaringClassFinal(actual_method)) {
          type = handles_->NewHandle(actual_method->GetDeclaringClass());
        } else {
          // Type must have been recovered from RTI.
          HInstruction* receiver = invoke->InputAt(0);
          if (receiver->IsNullCheck()) {
            // RTP might have not propagated through null check - let us go one more level.
            receiver = receiver->InputAt(0);
          }
          type = receiver->GetReferenceTypeInfo().GetTypeHandle();
        }
        DCHECK(type.GetReference() != nullptr);

        precise_prediction_.Put(invoke, type);
        PRINT_PASS_OSTREAM_MESSAGE(this, "Found precise type " << PrettyDescriptor(type.Get()) <<
                                   " for " << invoke);
      }
      return true;
    } else {
      // We do not have a precise type based on analysis - well what about from profile?
      std::vector<TypeHandle> possible_targets;

      // When compiling non-image, we can check CHA results.
      if (Runtime::Current()->UseCHA()) {
        // Check if CHA hasn't been called before.
        if (no_prediction_from_cha_.find(instr) == no_prediction_from_cha_.end()) {
          possible_targets = FindTypesFromCHA(resolved_method);
          // For now we work only with one target.
          if (possible_targets.size() == 1u) {
            cha_prediction_.Put(invoke, possible_targets[0]);
          } else {
            no_prediction_from_cha_.insert(instr);
          }
        }
      }

      if (possible_targets.size() != 1u) {
        possible_targets = FindTypesFromProfile(invoke, graph_->GetArtMethod());
      }

      if (possible_targets.size() != 0u) {
        // Seems that we have potential targets - record this if needed.
        if (update) {
          imprecise_predictions_.Put(invoke, possible_targets);
          if (this->IsVerbose()) {
            std::string potential_types = "";
            for (auto it : possible_targets) {
              potential_types.append(PrettyDescriptor(it.Get()));
              potential_types.append(",");
            }
            PRINT_PASS_OSTREAM_MESSAGE(this, "Found imprecise types " <<
                                       potential_types << " for " << invoke);
          }
        }
        return true;
      }
    }
  }

  // Could not figure out a prediction for this.
  return false;
}

uint64_t HDevirtualization::GetMaxCost() {
  return kCostOfLoadClass + kCostOfDevirtCheck;
}

uint64_t HDevirtualization::GetCost(HInstruction* instr) {
  DCHECK(HasPrediction(instr, false));
  if (precise_prediction_.find(instr) != precise_prediction_.end()) {
    return 0u;
  } else {
    DCHECK_NE(imprecise_predictions_.count(instr), 0u);
    std::vector<TypeHandle>& types = imprecise_predictions_.find(instr)->second;
    ScopedObjectAccess soa(Thread::Current());
    mirror::Class* referrer_class = graph_->GetArtMethod()->GetDeclaringClass();
    uint64_t cost = 0u;
    for (auto it : types) {
      if (it.Get() == referrer_class) {
        cost += (kCostOfLoadReferrerClass + kCostOfDevirtCheck);
      } else {
        cost += GetMaxCost();
      }
    }
    return cost;
  }
}

std::pair<uint64_t, uint64_t> HDevirtualization::GetMispredictRate(HInstruction* instr) {
  DCHECK(HasPrediction(instr, false));

  if (precise_prediction_.find(instr) != precise_prediction_.end()) {
    return std::make_pair (0, 10);
  } else {
    DCHECK(imprecise_predictions_.find(instr) != imprecise_predictions_.end());
    size_t count = imprecise_predictions_.count(instr);
    // TODO Use the actual profile information to determine the likelihood of mispredict.
    if (count == 1) {
      // Since target is imprecise, we do not want to return that prediction is always true.
      return std::make_pair(1, 10);
    }
    return std::make_pair(count - 1, count);
  }
}

uint64_t HDevirtualization::GetProfit(HInstruction* instr) {
  DCHECK(HasPrediction(instr, false));
  // Since direct invokes have a bigger path length than virtual invokes,
  // we do not get profit simply from the replacement. We get the profit from
  // potentially inlining.

  // Since we potentially save on the copying, include the number of arguments
  // plus one extra for the return.
  const uint32_t num_arguments = instr->AsInvoke()->GetNumberOfArguments() + 1;

  return kCostOfVirtualInvokes + num_arguments;
}

std::vector<HDevirtualization::TypeHandle> HDevirtualization::FindTypesFromProfile(
    HInvoke* invoke, ArtMethod* caller_method) {
  std::vector<TypeHandle> results;
  if (use_exact_profiles_) {
    ProfilingInfo* prof_info = ExactProfiler::FindProfileForMethod(caller_method);
    if (prof_info != nullptr) {
      // We have something to look at.  Does it have any information for this invoke?
      PRINT_PASS_OSTREAM_MESSAGE(this, "prof_info = " << prof_info
                                       << ", dex_pc = 0x" << std::hex << invoke->GetDexPc());
      const InlineCache& ic = *prof_info->GetInlineCache(invoke->GetDexPc());
      if (ic.IsAOTMonomorphic()) {
        mirror::Class* invoke_type = ic.GetAOTMonomorphicType();
        PRINT_PASS_OSTREAM_MESSAGE(this, "Found (effectively) monomorphic type "
                                         << PrettyDescriptor(invoke_type)
                                         << " for " << invoke);
        results.push_back(handles_->NewHandle(ic.GetMonomorphicType()));
      } else if (ic.IsPolymorphic()) {
        for (size_t i = 0; i < InlineCache::kIndividualCacheSize; ++i) {
          mirror::Class* invoke_type = ic.GetTypeAt(i);
          if (invoke_type == nullptr) {
            break;
          }
          PRINT_PASS_OSTREAM_MESSAGE(this, "Found polymorphic type (" << i << ") "
                                           << PrettyDescriptor(invoke_type)
                                           << " for " << invoke);
          results.push_back(handles_->NewHandle(ic.GetTypeAt(i)));
        }
      }
    }
  }

  // Unable to find a way to narrow down the choices.
  // TODO When getting a new handle - do not forget to update "handles_".
  return results;
}

std::vector<HDevirtualization::TypeHandle> HDevirtualization::FindTypesFromCHA(
    ArtMethod* resolved_method) {
  ScopedObjectAccess soa(Thread::Current());
  std::string match_class;
  Handle<mirror::ClassLoader> class_loader(handles_->NewHandle(
      soa.Decode<mirror::ClassLoader*>(compilation_unit_.GetClassLoader())));
  std::vector<TypeHandle> possible_targets;

  // We will check CHA.
  CompilerDriver::CHAType result = compiler_driver_->CheckCHA(match_class,
      resolved_method, class_loader);
  PRINT_PASS_OSTREAM_MESSAGE(this, "CHA #target = " << result << " for "
                             << PrettyMethod(resolved_method));
  if (result == CompilerDriver::kCHANotAnalyzed) {
    MaybeRecordStat(MethodCompilationStat::kIntelCHANotAnalyzed);
  } else if (result == CompilerDriver::kCHAOneTarget) {
    // We have one target.
    MaybeRecordStat(MethodCompilationStat::kIntelCHAOneTarget);
    // The target is from itself or one of the children.
    ClassLinker* class_linker = compilation_unit_.GetClassLinker();
    mirror::Class* klass = class_linker->LookupClass(soa.Self(), match_class.c_str(),
        ComputeModifiedUtf8Hash(match_class.c_str()), class_loader.Get());
    // We won't accept an abstract class, because type guard will fail.
    if (klass != nullptr && klass->IsResolved() && !klass->IsAbstract()) {
      possible_targets.push_back(handles_->NewHandle(klass));
    }
  } else {
    // We have two or more targets.
    MaybeRecordStat(MethodCompilationStat::kIntelCHATwoOrMoreTargets);
  }

  return possible_targets;
}

HDevirtualization::TypeHandle HDevirtualization::GetPrimaryType(HInvoke* invoke) const {
  if (precise_prediction_.find(invoke) != precise_prediction_.end()) {
    return precise_prediction_.Get(invoke);
  } else {
    auto it = imprecise_predictions_.find(invoke);
    DCHECK(it != imprecise_predictions_.end());
    DCHECK_NE(it->second.size(), 0u);
    return it->second[0];
  }
}

bool HDevirtualization::IsPredictionSame(HInstruction* instr, HInstruction* instr2) {
  DCHECK(HasPrediction(instr, false));
  DCHECK(HasPrediction(instr2, false));
  HInvoke* invoke1 = instr->AsInvoke();
  HInvoke* invoke2 = instr2->AsInvoke();

  // Same instance means it should use the same prediction.
  bool same_instance = (invoke1->InputAt(0) == invoke2->InputAt(0));
  if (!same_instance) {
    // They must have the exact same 'this' pointer to be guarded by the same guard.
    return false;
  }

  // For predictions from CHA, we need to compare types predicted by CHA.
  if (Runtime::Current()->UseCHA()) {
    auto iter1 = cha_prediction_.find(instr);
    auto iter2 = cha_prediction_.find(instr2);
    if (iter1 != cha_prediction_.end() && iter2 != cha_prediction_.end()) {
      return iter1->second.Get() == iter2->second.Get();
    }
    if (iter1 != cha_prediction_.end() || iter2 != cha_prediction_.end()) {
      // Only one prediction is from CHA.  We don't know if they are the same.
      return false;
    }
  }

  // We have to be more careful for imprecise types.  The possible types
  // need to match too.
  auto instr_precise = precise_prediction_.find(instr);
  auto instr_imprecise = imprecise_predictions_.find(instr);
  auto instr2_precise = precise_prediction_.find(instr2);
  auto instr2_imprecise = imprecise_predictions_.find(instr2);
  if (instr_precise != precise_prediction_.end()) {
    // First type is precise.
    if (instr2_precise != precise_prediction_.end()) {
      // Both precise.  Are we checking for the same type?
      return instr_precise->second.Get() == instr2_precise->second.Get();
    }
    DCHECK(instr2_imprecise != imprecise_predictions_.end());
    const std::vector<TypeHandle>& types = instr2_imprecise->second;
    return types.size() == 1 && types[0].Get() == instr_precise->second.Get();
  }

  // First type is imprecise.
  if (instr2_precise != precise_prediction_.end()) {
    // Matching an imprecise with a precise.
    const std::vector<TypeHandle>& types = instr_imprecise->second;
    return types.size() == 1 && types[0].Get() == instr2_precise->second.Get();
  }

  // Both are imprecise.
  if (instr_imprecise != imprecise_predictions_.end() &&
      instr2_imprecise != imprecise_predictions_.end()) {
    const std::vector<TypeHandle>& instr_types = instr_imprecise->second;
    const std::vector<TypeHandle>& instr2_types = instr2_imprecise->second;
    if (instr_types.size() != instr2_types.size()) {
      return false;
    }

    // All types must match.
    for (size_t i = 0; i < instr_types.size(); i++) {
      if (instr_types[i].Get() != instr2_types[i].Get()) {
        return false;
      }
    }
  }

  return true;
}

HSpeculationGuard* HDevirtualization::InsertSpeculationGuard(HInstruction* instr_guarded,
                                                             HInstruction* instr_cursor) {
  HInvoke* invoke = instr_guarded->AsInvoke();
  // The object is always the first argument of instance invoke.
  HInstruction* object = invoke->InputAt(0);

  // Check that the type is accessible from current dex cache.
  ScopedObjectAccess soa(Thread::Current());
  TypeHandle type = GetPrimaryType(invoke);
  const DexFile& caller_dex_file = *compilation_unit_.GetDexFile();
  uint32_t class_index = FindClassIndexIn(type.Get(), caller_dex_file, compilation_unit_.GetDexCache());
  if (class_index == DexFile::kDexNoIndex) {
    // Seems we cannot find current type in the dex cache.
    PRINT_PASS_OSTREAM_MESSAGE(this, "Guard insertion failed because we cannot find " <<
                               PrettyDescriptor(type.Get()) << " in the dex cache for " <<
                               invoke);
    return nullptr;
  }

  // Load the class from the object.
  constexpr Primitive::Type loaded_type = Primitive::kPrimNot;
  ArtField* field =
      compilation_unit_.GetClassLinker()->GetClassRoot(ClassLinker::kJavaLangObject)->GetInstanceField(0);
  DCHECK_EQ(std::string(field->GetName()), "shadow$_klass_");
  HInstanceFieldGet* class_getter = new (graph_->GetArena()) HInstanceFieldGet(
      object,
      loaded_type,
      field->GetOffset(),
      field->IsVolatile(),
      field->GetDexFieldIndex(),
      field->GetDeclaringClass()->GetDexClassDefIndex(),
      *field->GetDexFile(),
      handles_->NewHandle(field->GetDexCache()),
      instr_guarded->GetDexPc());
  // The class field is essentially a final field.
  class_getter->SetSideEffects(SideEffects::None());
  TypeHandle class_type = handles_->NewHandle(
      compilation_unit_.GetClassLinker()->GetClassRoot(ClassLinker::kJavaLangClass));
  class_getter->SetReferenceTypeInfo(ReferenceTypeInfo::Create(class_type , /* is_exact */ true));

  // Now create a load class for the prediction.
  bool is_referrer = (type.Get() == graph_->GetArtMethod()->GetDeclaringClass());
  HLoadClass* prediction = new (graph_->GetArena()) HLoadClass(graph_->GetCurrentMethod(),
                                                               class_index,
                                                               *compilation_unit_.GetDexFile(),
                                                               is_referrer,
                                                               instr_guarded->GetDexPc(),
                                                               /* needs_access_check */ false,
                                                               /* is_in_dex_cache */ true);
  prediction->SetLoadedClassRTI(ReferenceTypeInfo::Create(type, /* is_exact */ true));
  prediction->SetReferenceTypeInfo(ReferenceTypeInfo::Create(class_type , /* is_exact */ true));

  HDevirtGuard* guard = new (graph_->GetArena()) HDevirtGuard(prediction, class_getter, invoke->GetDexPc());

  // Handle the insertion.
  HBasicBlock* insertion_bb = instr_cursor->GetBlock();
  DCHECK(insertion_bb != nullptr);
  insertion_bb->InsertInstructionBefore(class_getter, instr_cursor);
  insertion_bb->InsertInstructionAfter(prediction, class_getter);
  insertion_bb->InsertInstructionAfter(guard, prediction);

  return guard;
}

bool HDevirtualization::HandleSpeculation(HInstruction* instr,
                                          HInstruction* instr_copy,
                                          bool guard_inserted) {
  HInvoke* invoke = instr->AsInvoke();
  uint32_t method_index = 0u;

  // Find the target method - we know that the class is in dex cache
  // and therefore the method must be as well.
  {
    ScopedObjectAccess soa(Thread::Current());
    TypeHandle type = GetPrimaryType(invoke);
    ClassLinker* cl = Runtime::Current()->GetClassLinker();
    size_t pointer_size = cl->GetImagePointerSize();
    const DexFile& caller_dex_file = *compilation_unit_.GetDexFile();

    if (!guard_inserted || kIsDebugBuild) {
      // We repeat the same check as in "InsertSpeculationGuard" which checks that
      // the current class can be found in current dex file. The reason we do this
      // is that precise types do not require guard.
      uint32_t class_index = FindClassIndexIn(type.Get(), caller_dex_file,
                                              compilation_unit_.GetDexCache());
      if (kIsDebugBuild && guard_inserted) {
        CHECK_NE(class_index, DexFile::kDexNoIndex)
            << "Cannot find " << PrettyDescriptor(type.Get())
            << " in the dex cache for " << invoke;
      } else if (class_index == DexFile::kDexNoIndex) {
        PRINT_PASS_OSTREAM_MESSAGE(this, "Sharpening failed because we cannot find " <<
                                   PrettyDescriptor(type.Get()) << " in the dex cache for " <<
                                   invoke);
        return false;
      }
    }

    ArtMethod* resolved_method =
        compilation_unit_.GetDexCache().Get()->GetResolvedMethod(invoke->GetDexMethodIndex(),
                                                                 pointer_size);
    // We only sharpen for resolved invokes.
    DCHECK(resolved_method != nullptr);
    if (UNLIKELY(resolved_method == nullptr)) {
        PRINT_PASS_OSTREAM_MESSAGE(this, "Sharpening failed because resolved method is Null");
        return false;
    }

    ArtMethod* actual_method = resolved_method;
    if (!IsMethodOrDeclaringClassFinal(resolved_method)) {
      if (invoke->IsInvokeInterface()) {
        actual_method = type->FindVirtualMethodForInterface(resolved_method, pointer_size);
      } else {
        DCHECK(invoke->IsInvokeVirtual());
        actual_method = type->FindVirtualMethodForVirtual(resolved_method, pointer_size);
      }
    }

    if (actual_method == nullptr) {
      PRINT_PASS_OSTREAM_MESSAGE(this, "Sharpening failed because we cannot find " <<
                                 PrettyMethod(resolved_method) << " in the class " <<
                                 PrettyDescriptor(type.Get()) <<  " for " << invoke);
      return false;
    }

    method_index = FindMethodIndexIn(actual_method,
                                     caller_dex_file,
                                     invoke->GetDexMethodIndex());
    if (method_index == DexFile::kDexNoIndex) {
      PRINT_PASS_OSTREAM_MESSAGE(this, "Sharpening failed because we cannot find " <<
                                 PrettyMethod(resolved_method) << " in the caller's dex file "
                                 "for " << invoke);
      return false;
    }
  }

  MethodReference target_method(compilation_unit_.GetDexFile(), method_index);
  HInvokeStaticOrDirect::DispatchInfo dispatch_info = {
      HInvokeStaticOrDirect::MethodLoadKind::kDexCacheViaMethod,
      HInvokeStaticOrDirect::CodePtrLocation::kCallArtMethod,
      0u,
      0U
  };
  HInvokeStaticOrDirect* new_invoke =
      new (graph_->GetArena()) HInvokeStaticOrDirect(graph_->GetArena(),
                                                     invoke->GetNumberOfArguments(),
                                                     invoke->GetType(),
                                                     invoke->GetDexPc(),
                                                     method_index,
                                                     target_method,
                                                     dispatch_info,
                                                     invoke->GetOriginalInvokeType(),
                                                     InvokeType::kDirect,
                                                     HInvokeStaticOrDirect::ClinitCheckRequirement::kNone);
  for (size_t i = 0, e = invoke->InputCount(); i < e; ++i) {
    HInstruction* input = invoke->InputAt(i);
    new_invoke->SetArgumentAt(i, input);
  }
  // We need to add current method as input so we can get access to dex cache.
  // This use might be removed during call sharpening phase.
  if (HInvokeStaticOrDirect::NeedsCurrentMethodInput(new_invoke->GetMethodLoadKind())) {
    new_invoke->SetArgumentAt(invoke->InputCount(), graph_->GetCurrentMethod());
  }
  // Keep the type information from previous invoke.
  if (invoke->GetType() == Primitive::kPrimNot) {
    new_invoke->SetReferenceTypeInfo(invoke->GetReferenceTypeInfo());
  }
  // No need to copy intrinsic information - these should not be candidates.
  DCHECK(!invoke->IsIntrinsic());

  if (instr_copy != nullptr) {
    DCHECK(instr_copy->IsInvoke());
    instr_copy->AsInvoke()->SetDevirtualized();
  }

  invoke->GetBlock()->ReplaceAndRemoveInstructionWith(invoke, new_invoke);
  new_invoke->CopyEnvironmentFrom(invoke->GetEnvironment());
  return true;
}

SpeculationRecoveryApproach HDevirtualization::GetRecoveryMethod(HInstruction* instr) {
  if (precise_prediction_.find(instr) != precise_prediction_.end()) {
    return kRecoveryNotNeeded;
  }

  if (Runtime::Current()->UseCHA()) {
    // For CHA, we prefer code versioning, because the type guard is not perfect.
    // We want to avoid the cost of deopt, when the type guard fails.
    return kRecoveryCodeVersioning;
  }

  size_t prediction_count = imprecise_predictions_.find(instr)->second.size();
  if (prediction_count == 1 && Runtime::Current()->UseJitCompilation()) {
    // If the prediction is monomorphic call - we do not care about the overhead of deopt.
    return kRecoveryDeopt;
  }

  // For multiple predictions or not in JIT - use versioning.
  return kRecoveryCodeVersioning;
}

}  // namespace art
