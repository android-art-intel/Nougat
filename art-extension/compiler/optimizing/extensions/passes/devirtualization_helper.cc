/*
 * Copyright (C) 2014 The Android Open Source Project
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

//! This file is used for implementations of devirtualization helper methods that were
//! adapted from files written by Google. Hence - the original license header is kept.

#include "devirtualization.h"

namespace art {

bool HDevirtualization::IsMethodOrDeclaringClassFinal(ArtMethod* method)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  return method->IsFinal() || method->GetDeclaringClass()->IsFinal();
}

ArtMethod* HDevirtualization::FindVirtualOrInterfaceTarget(HInvoke* invoke,
                                                           ArtMethod* resolved_method)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  if (IsMethodOrDeclaringClassFinal(resolved_method)) {
    // No need to lookup further, the resolved method will be the target.
    return resolved_method;
  }

  HInstruction* receiver = invoke->InputAt(0);
  if (receiver->IsNullCheck()) {
    // Find the actual receiver to check type.
    receiver = receiver->InputAt(0);
  }
  ReferenceTypeInfo info = receiver->GetReferenceTypeInfo();
  DCHECK(info.IsValid()) << "Invalid RTI for " << receiver->DebugName();
  if (!info.IsExact()) {
    // We currently only support inlining with known receivers.
    // TODO: Remove this check, we should be able to inline final methods
    // on unknown receivers.
    return nullptr;
  } else if (info.GetTypeHandle()->IsInterface()) {
    // Statically knowing that the receiver has an interface type cannot
    // help us find what is the target method.
    return nullptr;
  } else if (!resolved_method->GetDeclaringClass()->IsAssignableFrom(info.GetTypeHandle().Get())) {
    // The method that we're trying to call is not in the receiver's class or super classes.
    return nullptr;
  }

  ClassLinker* cl = Runtime::Current()->GetClassLinker();
  size_t pointer_size = cl->GetImagePointerSize();
  if (invoke->IsInvokeInterface()) {
    resolved_method = info.GetTypeHandle()->FindVirtualMethodForInterface(
        resolved_method, pointer_size);
  } else {
    DCHECK(invoke->IsInvokeVirtual());
    resolved_method = info.GetTypeHandle()->FindVirtualMethodForVirtual(
        resolved_method, pointer_size);
  }

  if (resolved_method == nullptr) {
    // The information we had on the receiver was not enough to find
    // the target method. Since we check above the exact type of the receiver,
    // the only reason this can happen is an IncompatibleClassChangeError.
    return nullptr;
  } else if (!resolved_method->IsInvokable()) {
    // The information we had on the receiver was not enough to find
    // the target method. Since we check above the exact type of the receiver,
    // the only reason this can happen is an IncompatibleClassChangeError.
    return nullptr;
  } else if (IsMethodOrDeclaringClassFinal(resolved_method)) {
    // A final method has to be the target method.
    return resolved_method;
  } else if (info.IsExact()) {
    // If we found a method and the receiver's concrete type is statically
    // known, we know for sure the target.
    return resolved_method;
  } else {
    // Even if we did find a method, the receiver type was not enough to
    // statically find the runtime target.
    return nullptr;
  }
}

uint32_t HDevirtualization::FindClassIndexIn(mirror::Class* cls, const DexFile& dex_file,
                                             Handle<mirror::DexCache> dex_cache)
                                                 SHARED_REQUIRES(Locks::mutator_lock_) {
  uint32_t index = DexFile::kDexNoIndex;
  if (cls->GetDexCache() == nullptr) {
    DCHECK(cls->IsArrayClass()) << PrettyClass(cls);
    index = cls->FindTypeIndexInOtherDexFile(dex_file);
  } else if (cls->GetDexTypeIndex() == DexFile::kDexNoIndex16) {
    DCHECK(cls->IsProxyClass()) << PrettyClass(cls);
    // TODO: deal with proxy classes.
  } else if (IsSameDexFile(cls->GetDexFile(), dex_file)) {
    index = cls->GetDexTypeIndex();
    // Update the dex cache to ensure the class is in. The generated code will
    // consider it is. We make it safe by updating the dex cache, as other
    // dex files might also load the class, and there is no guarantee the dex
    // cache of the dex file of the class will be updated.
    if (dex_cache->GetResolvedType(index) == nullptr) {
      dex_cache->SetResolvedType(index, cls);
    }
  } else {
    index = cls->FindTypeIndexInOtherDexFile(dex_file);
    // We cannot guarantee the entry in the dex cache will resolve to the same class,
    // as there may be different class loaders. So only return the index if it's
    // the right class in the dex cache already.
    if (index != DexFile::kDexNoIndex && dex_cache->GetResolvedType(index) != cls) {
      index = DexFile::kDexNoIndex;
    }
  }

  return index;
}

uint32_t HDevirtualization::FindMethodIndexIn(ArtMethod* method, const DexFile& dex_file,
                                              uint32_t referrer_index)
                                                  SHARED_REQUIRES(Locks::mutator_lock_) {
  if (IsSameDexFile(*method->GetDexFile(), dex_file)) {
    return method->GetDexMethodIndex();
  } else {
    return method->FindDexMethodIndexInOtherDexFile(dex_file, referrer_index);
  }
}

}  // namespace art
