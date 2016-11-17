/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include "art_method-inl.h"
#include "callee_save_frame.h"
#include "entrypoints/runtime_asm_entrypoints.h"
#include "ext_profiling.h"
#include "instrumentation.h"
#include "mirror/object-inl.h"
#include "runtime.h"
#include "thread-inl.h"

namespace art {

extern "C" const void* artInstrumentationMethodEntryFromCode(ArtMethod* method,
                                                             mirror::Object* this_object,
                                                             Thread* self,
                                                             uintptr_t lr)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  // Instrumentation changes the stack. Thus, when exiting, the stack cannot be verified, so skip
  // that part.
  ScopedQuickEntrypointChecks sqec(self, kIsDebugBuild, false);
  instrumentation::Instrumentation* instrumentation = Runtime::Current()->GetInstrumentation();
  const void* result;
  if (instrumentation->IsDeoptimized(method)) {
    result = GetQuickToInterpreterBridge();
  } else {
    result = instrumentation->GetQuickCodeFor(method, sizeof(void*));
    DCHECK(!Runtime::Current()->GetClassLinker()->IsQuickToInterpreterBridge(result));
  }
  bool interpreter_entry = (result == GetQuickToInterpreterBridge());
  instrumentation->PushInstrumentationStackFrame(self, method->IsStatic() ? nullptr : this_object,
                                                 method, lr, interpreter_entry);
  CHECK(result != nullptr) << PrettyMethod(method);
  return result;
}

extern "C" TwoWordReturn artInstrumentationMethodExitFromCode(Thread* self, ArtMethod** sp,
                                                              uint64_t gpr_result,
                                                              uint64_t fpr_result)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  // Instrumentation exit stub must not be entered with a pending exception.
  CHECK(!self->IsExceptionPending()) << "Enter instrumentation exit stub with pending exception "
                                     << self->GetException()->Dump();
  // Compute address of return PC and sanity check that it currently holds 0.
  size_t return_pc_offset = GetCalleeSaveReturnPcOffset(kRuntimeISA, Runtime::kRefsOnly);
  uintptr_t* return_pc = reinterpret_cast<uintptr_t*>(reinterpret_cast<uint8_t*>(sp) +
                                                      return_pc_offset);
  CHECK_EQ(*return_pc, 0U);

  // Pop the frame filling in the return pc. The low half of the return value is 0 when
  // deoptimization shouldn't be performed with the high-half having the return address. When
  // deoptimization should be performed the low half is zero and the high-half the address of the
  // deoptimization entry point.
  instrumentation::Instrumentation* instrumentation = Runtime::Current()->GetInstrumentation();
  TwoWordReturn return_or_deoptimize_pc = instrumentation->PopInstrumentationStackFrame(
      self, return_pc, gpr_result, fpr_result);
  return return_or_deoptimize_pc;
}

static void CantFindMethod(Runtime::ProfileBuffersMap& prof_counters,
                           art::ArtMethod* method,
                           uint32_t method_idx,
                           const DexFile* dex_file)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  LOG(INFO) << "Entries in counters: " << prof_counters.size();
  LOG(INFO) << "Method*: " << method << ", idx = " << method_idx
            << ", dex_file = " << dex_file << ", location = " << dex_file->GetLocation()
            << ", dex_file checksum = 0x" << std::hex << dex_file->GetHeader().checksum_;
  LOG(FATAL) << "Failed to locate counters for method: " << PrettyMethod(method, true);
}

static OneMethod* GetOneMethodFromArtMethod(art::ArtMethod* method, art::Thread* self)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  Runtime::ProfileBuffersMap& prof_counters = Runtime::Current()->GetProfileBuffers();
  // Handle Proxies properly.
  method = method->GetInterfaceMethodIfProxy(sizeof(void*));
  uint32_t method_idx = method->GetDexMethodIndex();
  const DexFile* dex_file = method->GetDexFile();
  MethodReference method_ref(dex_file, method_idx);
  MutexLock mu_prof(self, *Locks::profiler_lock_);
  auto it = prof_counters.find(method_ref);
  if (UNLIKELY(it == prof_counters.end())) {
    // Do error dump out of line.
    CantFindMethod(prof_counters, method, method_idx, dex_file);
    UNREACHABLE();
  }
  return it->second;
}

extern "C" uint64_t* art_quick_return_profiling_buffer(art::ArtMethod* method, art::Thread* self)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  OneMethod* info = GetOneMethodFromArtMethod(method, self);
  DCHECK(info != nullptr);
  return info->counts;
}

extern "C" void art_quick_profile_invoke(art::ArtMethod* method,
                                         art::Thread* self,
                                         uint32_t index,
                                         art::mirror::Object* object)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  OneMethod* info = GetOneMethodFromArtMethod(method, self);
  DCHECK(info != nullptr);

  // We now have the correct OneMethod.  Increment the correct counter.
  DCHECK_LT(index, info->num_method_invokes);
  OneCallSite* call_site = info->CallSiteAt(index);
  mirror::Class* cls = object->GetClass();

  for (size_t i = 0; i < OneCallSite::kNumInvokeTargets; ++i) {
    OneInvoke* invoke = &call_site->targets[i];
    mirror::Class* existing = invoke->klass.Read();
    if (existing == cls) {
      // Receiver type is already in the cache, increment the count.
      uint32_t new_count = invoke->count + 1;

      // Let us not overflow.
      if (new_count != 0) {
        invoke->count = new_count;
      }
      return;
    } else if (existing == nullptr) {
      // Cache entry is empty, try to put `cls` in it.
      GcRoot<mirror::Class> expected_root(nullptr);
      GcRoot<mirror::Class> desired_root(cls);
      if (!reinterpret_cast<Atomic<GcRoot<mirror::Class>>*>(&invoke->klass)->
              CompareExchangeStrongSequentiallyConsistent(expected_root, desired_root)) {
        // Some other thread put a class in the cache, continue iteration starting at this
        // entry in case the entry contains `cls`.
        --i;
      } else {
        // We successfully set `cls`, just return.
        invoke->count = 1;
        return;
      }
    }
  }
}

}  // namespace art
