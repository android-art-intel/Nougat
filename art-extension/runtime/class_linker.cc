/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include "class_linker.h"

#include <algorithm>
#include <deque>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <tuple>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

#include "art_field-inl.h"
#include "art_method-inl.h"
#include "base/arena_allocator.h"
#include "base/casts.h"
#include "base/logging.h"
#include "base/scoped_arena_containers.h"
#include "base/scoped_flock.h"
#include "base/stl_util.h"
#include "base/systrace.h"
#include "base/time_utils.h"
#include "base/unix_file/fd_file.h"
#include "base/value_object.h"
#include "class_linker-inl.h"
#include "class_table-inl.h"
#include "compiler_callbacks.h"
#include "debugger.h"
#include "dex_file-inl.h"
#include "entrypoints/entrypoint_utils.h"
#include "entrypoints/runtime_asm_entrypoints.h"
#include "experimental_flags.h"
#include "ext_profiling.h"
#include "gc_root-inl.h"
#include "gc/accounting/card_table-inl.h"
#include "gc/accounting/heap_bitmap-inl.h"
#include "gc/heap.h"
#include "gc/scoped_gc_critical_section.h"
#include "gc/space/image_space.h"
#include "handle_scope-inl.h"
#include "image-inl.h"
#include "intern_table.h"
#include "interpreter/interpreter.h"
#include "jit/jit.h"
#include "jit/jit_code_cache.h"
#include "jit/offline_profiling_info.h"
#include "leb128.h"
#include "linear_alloc.h"
#include "mirror/class.h"
#include "mirror/class-inl.h"
#include "mirror/class_loader.h"
#include "mirror/dex_cache-inl.h"
#include "mirror/field.h"
#include "mirror/iftable-inl.h"
#include "mirror/method.h"
#include "mirror/object-inl.h"
#include "mirror/object_array-inl.h"
#include "mirror/proxy.h"
#include "mirror/reference-inl.h"
#include "mirror/stack_trace_element.h"
#include "mirror/string-inl.h"
#include "native/dalvik_system_DexFile.h"
#include "oat.h"
#include "oat_file.h"
#include "oat_file-inl.h"
#include "oat_file_assistant.h"
#include "oat_file_manager.h"
#include "object_lock.h"
#include "os.h"
#include "runtime.h"
#include "ScopedLocalRef.h"
#include "scoped_thread_state_change.h"
#include "thread-inl.h"
#include "trace.h"
#include "utils.h"
#include "utils/dex_cache_arrays_layout-inl.h"
#include "verifier/method_verifier.h"
#include "well_known_classes.h"

namespace art {

static constexpr bool kSanityCheckObjects = kIsDebugBuild;
static constexpr bool kVerifyArtMethodDeclaringClasses = kIsDebugBuild;

static void ThrowNoClassDefFoundError(const char* fmt, ...)
    __attribute__((__format__(__printf__, 1, 2)))
    SHARED_REQUIRES(Locks::mutator_lock_);
static void ThrowNoClassDefFoundError(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  Thread* self = Thread::Current();
  self->ThrowNewExceptionV("Ljava/lang/NoClassDefFoundError;", fmt, args);
  va_end(args);
}

static bool HasInitWithString(Thread* self, ClassLinker* class_linker, const char* descriptor)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  ArtMethod* method = self->GetCurrentMethod(nullptr);
  StackHandleScope<1> hs(self);
  Handle<mirror::ClassLoader> class_loader(hs.NewHandle(method != nullptr ?
      method->GetDeclaringClass()->GetClassLoader() : nullptr));
  mirror::Class* exception_class = class_linker->FindClass(self, descriptor, class_loader);

  if (exception_class == nullptr) {
    // No exc class ~ no <init>-with-string.
    CHECK(self->IsExceptionPending());
    self->ClearException();
    return false;
  }

  ArtMethod* exception_init_method = exception_class->FindDeclaredDirectMethod(
      "<init>", "(Ljava/lang/String;)V", class_linker->GetImagePointerSize());
  return exception_init_method != nullptr;
}

// Helper for ThrowEarlierClassFailure. Throws the stored error.
static void HandleEarlierVerifyError(Thread* self, ClassLinker* class_linker, mirror::Class* c)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  mirror::Object* obj = c->GetVerifyError();
  DCHECK(obj != nullptr);
  self->AssertNoPendingException();
  if (obj->IsClass()) {
    // Previous error has been stored as class. Create a new exception of that type.

    // It's possible the exception doesn't have a <init>(String).
    std::string temp;
    const char* descriptor = obj->AsClass()->GetDescriptor(&temp);

    if (HasInitWithString(self, class_linker, descriptor)) {
      self->ThrowNewException(descriptor, PrettyDescriptor(c).c_str());
    } else {
      self->ThrowNewException(descriptor, nullptr);
    }
  } else {
    // Previous error has been stored as an instance. Just rethrow.
    mirror::Class* throwable_class =
        self->DecodeJObject(WellKnownClasses::java_lang_Throwable)->AsClass();
    mirror::Class* error_class = obj->GetClass();
    CHECK(throwable_class->IsAssignableFrom(error_class));
    self->SetException(obj->AsThrowable());
  }
  self->AssertPendingException();
}

void ClassLinker::ThrowEarlierClassFailure(mirror::Class* c, bool wrap_in_no_class_def) {
  // The class failed to initialize on a previous attempt, so we want to throw
  // a NoClassDefFoundError (v2 2.17.5).  The exception to this rule is if we
  // failed in verification, in which case v2 5.4.1 says we need to re-throw
  // the previous error.
  Runtime* const runtime = Runtime::Current();
  if (!runtime->IsAotCompiler()) {  // Give info if this occurs at runtime.
    std::string extra;
    if (c->GetVerifyError() != nullptr) {
      mirror::Object* verify_error = c->GetVerifyError();
      if (verify_error->IsClass()) {
        extra = PrettyDescriptor(verify_error->AsClass());
      } else {
        extra = verify_error->AsThrowable()->Dump();
      }
    }
    LOG(INFO) << "Rejecting re-init on previously-failed class " << PrettyClass(c) << ": " << extra;
  }

  CHECK(c->IsErroneous()) << PrettyClass(c) << " " << c->GetStatus();
  Thread* self = Thread::Current();
  if (runtime->IsAotCompiler()) {
    // At compile time, accurate errors and NCDFE are disabled to speed compilation.
    mirror::Throwable* pre_allocated = runtime->GetPreAllocatedNoClassDefFoundError();
    self->SetException(pre_allocated);
  } else {
    if (c->GetVerifyError() != nullptr) {
      // Rethrow stored error.
      HandleEarlierVerifyError(self, this, c);
    }
    if (c->GetVerifyError() == nullptr || wrap_in_no_class_def) {
      // If there isn't a recorded earlier error, or this is a repeat throw from initialization,
      // the top-level exception must be a NoClassDefFoundError. The potentially already pending
      // exception will be a cause.
      self->ThrowNewWrappedException("Ljava/lang/NoClassDefFoundError;",
                                     PrettyDescriptor(c).c_str());
    }
  }
}

static void VlogClassInitializationFailure(Handle<mirror::Class> klass)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  if (VLOG_IS_ON(class_linker)) {
    std::string temp;
    LOG(INFO) << "Failed to initialize class " << klass->GetDescriptor(&temp) << " from "
              << klass->GetLocation() << "\n" << Thread::Current()->GetException()->Dump();
  }
}

static void WrapExceptionInInitializer(Handle<mirror::Class> klass)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  Thread* self = Thread::Current();
  JNIEnv* env = self->GetJniEnv();

  ScopedLocalRef<jthrowable> cause(env, env->ExceptionOccurred());
  CHECK(cause.get() != nullptr);

  env->ExceptionClear();
  bool is_error = env->IsInstanceOf(cause.get(), WellKnownClasses::java_lang_Error);
  env->Throw(cause.get());

  // We only wrap non-Error exceptions; an Error can just be used as-is.
  if (!is_error) {
    self->ThrowNewWrappedException("Ljava/lang/ExceptionInInitializerError;", nullptr);
  }
  VlogClassInitializationFailure(klass);
}

// Gap between two fields in object layout.
struct FieldGap {
  uint32_t start_offset;  // The offset from the start of the object.
  uint32_t size;  // The gap size of 1, 2, or 4 bytes.
};
struct FieldGapsComparator {
  explicit FieldGapsComparator() {
  }
  bool operator() (const FieldGap& lhs, const FieldGap& rhs)
      NO_THREAD_SAFETY_ANALYSIS {
    // Sort by gap size, largest first. Secondary sort by starting offset.
    // Note that the priority queue returns the largest element, so operator()
    // should return true if lhs is less than rhs.
    return lhs.size < rhs.size || (lhs.size == rhs.size && lhs.start_offset > rhs.start_offset);
  }
};
typedef std::priority_queue<FieldGap, std::vector<FieldGap>, FieldGapsComparator> FieldGaps;

// Adds largest aligned gaps to queue of gaps.
static void AddFieldGap(uint32_t gap_start, uint32_t gap_end, FieldGaps* gaps) {
  DCHECK(gaps != nullptr);

  uint32_t current_offset = gap_start;
  while (current_offset != gap_end) {
    size_t remaining = gap_end - current_offset;
    if (remaining >= sizeof(uint32_t) && IsAligned<4>(current_offset)) {
      gaps->push(FieldGap {current_offset, sizeof(uint32_t)});
      current_offset += sizeof(uint32_t);
    } else if (remaining >= sizeof(uint16_t) && IsAligned<2>(current_offset)) {
      gaps->push(FieldGap {current_offset, sizeof(uint16_t)});
      current_offset += sizeof(uint16_t);
    } else {
      gaps->push(FieldGap {current_offset, sizeof(uint8_t)});
      current_offset += sizeof(uint8_t);
    }
    DCHECK_LE(current_offset, gap_end) << "Overran gap";
  }
}
// Shuffle fields forward, making use of gaps whenever possible.
template<int n>
static void ShuffleForward(size_t* current_field_idx,
                           MemberOffset* field_offset,
                           std::deque<ArtField*>* grouped_and_sorted_fields,
                           FieldGaps* gaps)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  DCHECK(current_field_idx != nullptr);
  DCHECK(grouped_and_sorted_fields != nullptr);
  DCHECK(gaps != nullptr);
  DCHECK(field_offset != nullptr);

  DCHECK(IsPowerOfTwo(n));
  while (!grouped_and_sorted_fields->empty()) {
    ArtField* field = grouped_and_sorted_fields->front();
    Primitive::Type type = field->GetTypeAsPrimitiveType();
    if (Primitive::ComponentSize(type) < n) {
      break;
    }
    if (!IsAligned<n>(field_offset->Uint32Value())) {
      MemberOffset old_offset = *field_offset;
      *field_offset = MemberOffset(RoundUp(field_offset->Uint32Value(), n));
      AddFieldGap(old_offset.Uint32Value(), field_offset->Uint32Value(), gaps);
    }
    CHECK(type != Primitive::kPrimNot) << PrettyField(field);  // should be primitive types
    grouped_and_sorted_fields->pop_front();
    if (!gaps->empty() && gaps->top().size >= n) {
      FieldGap gap = gaps->top();
      gaps->pop();
      DCHECK_ALIGNED(gap.start_offset, n);
      field->SetOffset(MemberOffset(gap.start_offset));
      if (gap.size > n) {
        AddFieldGap(gap.start_offset + n, gap.start_offset + gap.size, gaps);
      }
    } else {
      DCHECK_ALIGNED(field_offset->Uint32Value(), n);
      field->SetOffset(*field_offset);
      *field_offset = MemberOffset(field_offset->Uint32Value() + n);
    }
    ++(*current_field_idx);
  }
}

ClassLinker::ClassLinker(InternTable* intern_table)
    // dex_lock_ is recursive as it may be used in stack dumping.
    : dex_lock_("ClassLinker dex lock", kDefaultMutexLevel),
      cha_lock_("Class Hierarchy Analysis lock", kDefaultMutexLevel),
      dex_cache_boot_image_class_lookup_required_(false),
      failed_dex_cache_class_lookups_(0),
      class_roots_(nullptr),
      array_iftable_(nullptr),
      find_array_class_cache_next_victim_(0),
      init_done_(false),
      log_new_class_table_roots_(false),
      intern_table_(intern_table),
      quick_resolution_trampoline_(nullptr),
      quick_imt_conflict_trampoline_(nullptr),
      quick_generic_jni_trampoline_(nullptr),
      quick_to_interpreter_bridge_trampoline_(nullptr),
      image_pointer_size_(sizeof(void*)) {
  CHECK(intern_table_ != nullptr);
  static_assert(kFindArrayCacheSize == arraysize(find_array_class_cache_),
                "Array cache size wrong.");
  std::fill_n(find_array_class_cache_, kFindArrayCacheSize, GcRoot<mirror::Class>(nullptr));
}

void ClassLinker::CheckSystemClass(Thread* self, Handle<mirror::Class> c1, const char* descriptor) {
  mirror::Class* c2 = FindSystemClass(self, descriptor);
  if (c2 == nullptr) {
    LOG(FATAL) << "Could not find class " << descriptor;
    UNREACHABLE();
  }
  if (c1.Get() != c2) {
    std::ostringstream os1, os2;
    c1->DumpClass(os1, mirror::Class::kDumpClassFullDetail);
    c2->DumpClass(os2, mirror::Class::kDumpClassFullDetail);
    LOG(FATAL) << "InitWithoutImage: Class mismatch for " << descriptor
               << ". This is most likely the result of a broken build. Make sure that "
               << "libcore and art projects match.\n\n"
               << os1.str() << "\n\n" << os2.str();
    UNREACHABLE();
  }
}

void ClassLinker::DumpCHA() {
  ReaderMutexLock mu(Thread::Current(), cha_lock_);
  size_t total = 0;
  LOG(INFO) << "DumpCHA # of classes =  " << cha_string_table_.size();
  for (size_t z = 0; z < cha_string_table_.size(); z++) {
    LOG(INFO) << z << ": " << cha_string_table_[z] << " # of children = "
              << cha_child_classes_[z].size();
    total += cha_child_classes_[z].size();
    for (size_t y = 0; y < cha_child_classes_[z].size(); y++) {
      size_t child = cha_child_classes_[z][y];
      LOG(INFO) << z << ":" << y << " " << cha_string_table_[child];
    }
  }
  LOG(INFO) << "DumpCHA total # of children = " << total;
}

void ClassLinker::AddCHA(mirror::Class* parent, mirror::Class* child) {
  WriterMutexLock mu(Thread::Current(), cha_lock_);
  std::string temp;
  std::string parent_string(parent->GetDescriptor(&temp));
  std::string child_string(child->GetDescriptor(&temp));

  // Find the parent index.
  auto parent_iter = cha_string_map_.find(parent_string);
  size_t parent_index = 0;
  if (parent_iter == cha_string_map_.end()) {
    parent_index = cha_string_table_.size();
    cha_string_map_.insert({parent_string, parent_index});
    cha_string_table_.push_back(parent_string);
    cha_child_classes_.push_back(std::vector<size_t>(0));
  } else {
    parent_index = parent_iter->second;
  }

  // Find the child index.
  auto child_iter = cha_string_map_.find(child_string);
  size_t child_index = 0;
  if (child_iter == cha_string_map_.end()) {
    child_index = cha_string_table_.size();
    cha_string_map_.insert({child_string, child_index});
    cha_string_table_.push_back(child_string);
    cha_child_classes_.push_back(std::vector<size_t>(0));
  } else {
    child_index = child_iter->second;
  }

  // Add the child to the parent's child classes.
  cha_child_classes_[parent_index].push_back(child_index);
}

bool ClassLinker::SearchChildren(std::vector<size_t>& match_class_index,
                                 std::string parent,
                                 uint32_t& num_of_match,
                                 ArtMethod* resolved_method,
                                 Handle<mirror::ClassLoader> class_loader) {
  Thread* self = Thread::Current();
  const size_t pointer_size = GetImagePointerSize();
  std::vector<size_t> all_children;
  const char* resolved_method_name = resolved_method->GetName();
  const art::Signature resolved_method_signature = resolved_method->GetSignature();

  size_t parent_index = 0;
  if (GetCHAStringMapIndex(parent, parent_index)) {
    // We always push parent_index to match_class_index.
    match_class_index.push_back(parent_index);

    // We push parent_index to all_children.
    all_children.push_back(parent_index);
  } else {
    // We don't have this parent class.
    // We failed to analyze this parent class.
    return false;
  }

  while (!all_children.empty()) {
    parent_index = all_children.back();
    all_children.pop_back();

    size_t num_of_child = GetCHAChildClassesSize(parent_index);
    if (num_of_child == 0) {
      continue;
    } else if (num_of_child > kCHAMaxNumChildren) {
      // We failed to analyze, because of too many children.
      return false;
    }
    for (size_t i = 0; i < num_of_child; i++) {
      size_t child_index = GetCHAChildClassesChildIndex(parent_index, i);
      all_children.push_back(child_index);
      std::string child = GetCHAStringTableString(child_index);
      // Check all methods in this child to match resolved_method.
      mirror::Class* klass = LookupClass(self, child.c_str(),
          ComputeModifiedUtf8Hash(child.c_str()), class_loader.Get());
      if (klass == nullptr || !klass->IsResolved()) {
        // We cannot find the child class.
        return false;
      }
      for (auto& method : klass->GetVirtualMethods(pointer_size)) {
        ArtMethod* const np_method = method.GetInterfaceMethodIfProxy(pointer_size);
        if (!np_method->IsAbstract()
            && strcmp(resolved_method_name, np_method->GetName()) == 0
            && resolved_method_signature == np_method->GetSignature()) {
          // Find a match.
          num_of_match++;
          match_class_index.push_back(child_index);
          if (num_of_match > 1) {
            // For now, we just care about one target or more than one.
            return true;
          }
        }
      }
    }
  }
  return true;
}

bool ClassLinker::InitWithoutImage(std::vector<std::unique_ptr<const DexFile>> boot_class_path,
                                   std::string* error_msg) {
  VLOG(startup) << "ClassLinker::Init";

  Thread* const self = Thread::Current();
  Runtime* const runtime = Runtime::Current();
  gc::Heap* const heap = runtime->GetHeap();

  CHECK(!heap->HasBootImageSpace()) << "Runtime has image. We should use it.";
  CHECK(!init_done_);

  // Use the pointer size from the runtime since we are probably creating the image.
  image_pointer_size_ = InstructionSetPointerSize(runtime->GetInstructionSet());
  if (!ValidPointerSize(image_pointer_size_)) {
    *error_msg = StringPrintf("Invalid image pointer size: %zu", image_pointer_size_);
    return false;
  }

  // java_lang_Class comes first, it's needed for AllocClass
  // The GC can't handle an object with a null class since we can't get the size of this object.
  heap->IncrementDisableMovingGC(self);
  StackHandleScope<64> hs(self);  // 64 is picked arbitrarily.
  auto class_class_size = mirror::Class::ClassClassSize(image_pointer_size_);
  Handle<mirror::Class> java_lang_Class(hs.NewHandle(down_cast<mirror::Class*>(
      heap->AllocNonMovableObject<true>(self, nullptr, class_class_size, VoidFunctor()))));
  CHECK(java_lang_Class.Get() != nullptr);
  mirror::Class::SetClassClass(java_lang_Class.Get());
  java_lang_Class->SetClass(java_lang_Class.Get());
  if (kUseBakerOrBrooksReadBarrier) {
    java_lang_Class->AssertReadBarrierPointer();
  }
  java_lang_Class->SetClassSize(class_class_size);
  java_lang_Class->SetPrimitiveType(Primitive::kPrimNot);
  heap->DecrementDisableMovingGC(self);
  // AllocClass(mirror::Class*) can now be used

  // Class[] is used for reflection support.
  auto class_array_class_size = mirror::ObjectArray<mirror::Class>::ClassSize(image_pointer_size_);
  Handle<mirror::Class> class_array_class(hs.NewHandle(
      AllocClass(self, java_lang_Class.Get(), class_array_class_size)));
  class_array_class->SetComponentType(java_lang_Class.Get());

  // java_lang_Object comes next so that object_array_class can be created.
  Handle<mirror::Class> java_lang_Object(hs.NewHandle(
      AllocClass(self, java_lang_Class.Get(), mirror::Object::ClassSize(image_pointer_size_))));
  CHECK(java_lang_Object.Get() != nullptr);
  // backfill Object as the super class of Class.
  java_lang_Class->SetSuperClass(java_lang_Object.Get());
  mirror::Class::SetStatus(java_lang_Object, mirror::Class::kStatusLoaded, self);

  java_lang_Object->SetObjectSize(sizeof(mirror::Object));
  // Allocate in non-movable so that it's possible to check if a JNI weak global ref has been
  // cleared without triggering the read barrier and unintentionally mark the sentinel alive.
  runtime->SetSentinel(heap->AllocNonMovableObject<true>(self,
                                                         java_lang_Object.Get(),
                                                         java_lang_Object->GetObjectSize(),
                                                         VoidFunctor()));

  // Object[] next to hold class roots.
  Handle<mirror::Class> object_array_class(hs.NewHandle(
      AllocClass(self, java_lang_Class.Get(),
                 mirror::ObjectArray<mirror::Object>::ClassSize(image_pointer_size_))));
  object_array_class->SetComponentType(java_lang_Object.Get());

  // Setup the char (primitive) class to be used for char[].
  Handle<mirror::Class> char_class(hs.NewHandle(
      AllocClass(self, java_lang_Class.Get(),
                 mirror::Class::PrimitiveClassSize(image_pointer_size_))));
  // The primitive char class won't be initialized by
  // InitializePrimitiveClass until line 459, but strings (and
  // internal char arrays) will be allocated before that and the
  // component size, which is computed from the primitive type, needs
  // to be set here.
  char_class->SetPrimitiveType(Primitive::kPrimChar);

  // Setup the char[] class to be used for String.
  Handle<mirror::Class> char_array_class(hs.NewHandle(
      AllocClass(self, java_lang_Class.Get(), mirror::Array::ClassSize(image_pointer_size_))));
  char_array_class->SetComponentType(char_class.Get());
  mirror::CharArray::SetArrayClass(char_array_class.Get());

  // Setup String.
  Handle<mirror::Class> java_lang_String(hs.NewHandle(
      AllocClass(self, java_lang_Class.Get(), mirror::String::ClassSize(image_pointer_size_))));
  java_lang_String->SetStringClass();
  mirror::String::SetClass(java_lang_String.Get());
  mirror::Class::SetStatus(java_lang_String, mirror::Class::kStatusResolved, self);

  // Setup java.lang.ref.Reference.
  Handle<mirror::Class> java_lang_ref_Reference(hs.NewHandle(
      AllocClass(self, java_lang_Class.Get(), mirror::Reference::ClassSize(image_pointer_size_))));
  mirror::Reference::SetClass(java_lang_ref_Reference.Get());
  java_lang_ref_Reference->SetObjectSize(mirror::Reference::InstanceSize());
  mirror::Class::SetStatus(java_lang_ref_Reference, mirror::Class::kStatusResolved, self);

  // Create storage for root classes, save away our work so far (requires descriptors).
  class_roots_ = GcRoot<mirror::ObjectArray<mirror::Class>>(
      mirror::ObjectArray<mirror::Class>::Alloc(self, object_array_class.Get(),
                                                kClassRootsMax));
  CHECK(!class_roots_.IsNull());
  SetClassRoot(kJavaLangClass, java_lang_Class.Get());
  SetClassRoot(kJavaLangObject, java_lang_Object.Get());
  SetClassRoot(kClassArrayClass, class_array_class.Get());
  SetClassRoot(kObjectArrayClass, object_array_class.Get());
  SetClassRoot(kCharArrayClass, char_array_class.Get());
  SetClassRoot(kJavaLangString, java_lang_String.Get());
  SetClassRoot(kJavaLangRefReference, java_lang_ref_Reference.Get());

  // Setup the primitive type classes.
  SetClassRoot(kPrimitiveBoolean, CreatePrimitiveClass(self, Primitive::kPrimBoolean));
  SetClassRoot(kPrimitiveByte, CreatePrimitiveClass(self, Primitive::kPrimByte));
  SetClassRoot(kPrimitiveShort, CreatePrimitiveClass(self, Primitive::kPrimShort));
  SetClassRoot(kPrimitiveInt, CreatePrimitiveClass(self, Primitive::kPrimInt));
  SetClassRoot(kPrimitiveLong, CreatePrimitiveClass(self, Primitive::kPrimLong));
  SetClassRoot(kPrimitiveFloat, CreatePrimitiveClass(self, Primitive::kPrimFloat));
  SetClassRoot(kPrimitiveDouble, CreatePrimitiveClass(self, Primitive::kPrimDouble));
  SetClassRoot(kPrimitiveVoid, CreatePrimitiveClass(self, Primitive::kPrimVoid));

  // Create array interface entries to populate once we can load system classes.
  array_iftable_ = GcRoot<mirror::IfTable>(AllocIfTable(self, 2));

  // Create int array type for AllocDexCache (done in AppendToBootClassPath).
  Handle<mirror::Class> int_array_class(hs.NewHandle(
      AllocClass(self, java_lang_Class.Get(), mirror::Array::ClassSize(image_pointer_size_))));
  int_array_class->SetComponentType(GetClassRoot(kPrimitiveInt));
  mirror::IntArray::SetArrayClass(int_array_class.Get());
  SetClassRoot(kIntArrayClass, int_array_class.Get());

  // Create long array type for AllocDexCache (done in AppendToBootClassPath).
  Handle<mirror::Class> long_array_class(hs.NewHandle(
      AllocClass(self, java_lang_Class.Get(), mirror::Array::ClassSize(image_pointer_size_))));
  long_array_class->SetComponentType(GetClassRoot(kPrimitiveLong));
  mirror::LongArray::SetArrayClass(long_array_class.Get());
  SetClassRoot(kLongArrayClass, long_array_class.Get());

  // now that these are registered, we can use AllocClass() and AllocObjectArray

  // Set up DexCache. This cannot be done later since AppendToBootClassPath calls AllocDexCache.
  Handle<mirror::Class> java_lang_DexCache(hs.NewHandle(
      AllocClass(self, java_lang_Class.Get(), mirror::DexCache::ClassSize(image_pointer_size_))));
  SetClassRoot(kJavaLangDexCache, java_lang_DexCache.Get());
  java_lang_DexCache->SetDexCacheClass();
  java_lang_DexCache->SetObjectSize(mirror::DexCache::InstanceSize());
  mirror::Class::SetStatus(java_lang_DexCache, mirror::Class::kStatusResolved, self);

  // Set up array classes for string, field, method
  Handle<mirror::Class> object_array_string(hs.NewHandle(
      AllocClass(self, java_lang_Class.Get(),
                 mirror::ObjectArray<mirror::String>::ClassSize(image_pointer_size_))));
  object_array_string->SetComponentType(java_lang_String.Get());
  SetClassRoot(kJavaLangStringArrayClass, object_array_string.Get());

  LinearAlloc* linear_alloc = runtime->GetLinearAlloc();
  // Create runtime resolution and imt conflict methods.
  runtime->SetResolutionMethod(runtime->CreateResolutionMethod());
  runtime->SetImtConflictMethod(runtime->CreateImtConflictMethod(linear_alloc));
  runtime->SetImtUnimplementedMethod(runtime->CreateImtConflictMethod(linear_alloc));

  // Setup boot_class_path_ and register class_path now that we can use AllocObjectArray to create
  // DexCache instances. Needs to be after String, Field, Method arrays since AllocDexCache uses
  // these roots.
  if (boot_class_path.empty()) {
    *error_msg = "Boot classpath is empty.";
    return false;
  }
  for (auto& dex_file : boot_class_path) {
    if (dex_file.get() == nullptr) {
      *error_msg = "Null dex file.";
      return false;
    }
    AppendToBootClassPath(self, *dex_file);
    boot_dex_files_.push_back(std::move(dex_file));
  }

  // now we can use FindSystemClass

  // run char class through InitializePrimitiveClass to finish init
  InitializePrimitiveClass(char_class.Get(), Primitive::kPrimChar);
  SetClassRoot(kPrimitiveChar, char_class.Get());  // needs descriptor

  // Set up GenericJNI entrypoint. That is mainly a hack for common_compiler_test.h so that
  // we do not need friend classes or a publicly exposed setter.
  quick_generic_jni_trampoline_ = GetQuickGenericJniStub();
  if (!runtime->IsAotCompiler()) {
    // We need to set up the generic trampolines since we don't have an image.
    quick_resolution_trampoline_ = GetQuickResolutionStub();
    quick_imt_conflict_trampoline_ = GetQuickImtConflictStub();
    quick_to_interpreter_bridge_trampoline_ = GetQuickToInterpreterBridge();
  }

  // Object, String and DexCache need to be rerun through FindSystemClass to finish init
  mirror::Class::SetStatus(java_lang_Object, mirror::Class::kStatusNotReady, self);
  CheckSystemClass(self, java_lang_Object, "Ljava/lang/Object;");
  CHECK_EQ(java_lang_Object->GetObjectSize(), mirror::Object::InstanceSize());
  mirror::Class::SetStatus(java_lang_String, mirror::Class::kStatusNotReady, self);
  CheckSystemClass(self, java_lang_String, "Ljava/lang/String;");
  mirror::Class::SetStatus(java_lang_DexCache, mirror::Class::kStatusNotReady, self);
  CheckSystemClass(self, java_lang_DexCache, "Ljava/lang/DexCache;");
  CHECK_EQ(java_lang_DexCache->GetObjectSize(), mirror::DexCache::InstanceSize());

  // Setup the primitive array type classes - can't be done until Object has a vtable.
  SetClassRoot(kBooleanArrayClass, FindSystemClass(self, "[Z"));
  mirror::BooleanArray::SetArrayClass(GetClassRoot(kBooleanArrayClass));

  SetClassRoot(kByteArrayClass, FindSystemClass(self, "[B"));
  mirror::ByteArray::SetArrayClass(GetClassRoot(kByteArrayClass));

  CheckSystemClass(self, char_array_class, "[C");

  SetClassRoot(kShortArrayClass, FindSystemClass(self, "[S"));
  mirror::ShortArray::SetArrayClass(GetClassRoot(kShortArrayClass));

  CheckSystemClass(self, int_array_class, "[I");
  CheckSystemClass(self, long_array_class, "[J");

  SetClassRoot(kFloatArrayClass, FindSystemClass(self, "[F"));
  mirror::FloatArray::SetArrayClass(GetClassRoot(kFloatArrayClass));

  SetClassRoot(kDoubleArrayClass, FindSystemClass(self, "[D"));
  mirror::DoubleArray::SetArrayClass(GetClassRoot(kDoubleArrayClass));

  // Run Class through FindSystemClass. This initializes the dex_cache_ fields and register it
  // in class_table_.
  CheckSystemClass(self, java_lang_Class, "Ljava/lang/Class;");

  CheckSystemClass(self, class_array_class, "[Ljava/lang/Class;");
  CheckSystemClass(self, object_array_class, "[Ljava/lang/Object;");

  // Setup the single, global copy of "iftable".
  auto java_lang_Cloneable = hs.NewHandle(FindSystemClass(self, "Ljava/lang/Cloneable;"));
  CHECK(java_lang_Cloneable.Get() != nullptr);
  auto java_io_Serializable = hs.NewHandle(FindSystemClass(self, "Ljava/io/Serializable;"));
  CHECK(java_io_Serializable.Get() != nullptr);
  // We assume that Cloneable/Serializable don't have superinterfaces -- normally we'd have to
  // crawl up and explicitly list all of the supers as well.
  array_iftable_.Read()->SetInterface(0, java_lang_Cloneable.Get());
  array_iftable_.Read()->SetInterface(1, java_io_Serializable.Get());

  // Sanity check Class[] and Object[]'s interfaces. GetDirectInterface may cause thread
  // suspension.
  CHECK_EQ(java_lang_Cloneable.Get(),
           mirror::Class::GetDirectInterface(self, class_array_class, 0));
  CHECK_EQ(java_io_Serializable.Get(),
           mirror::Class::GetDirectInterface(self, class_array_class, 1));
  CHECK_EQ(java_lang_Cloneable.Get(),
           mirror::Class::GetDirectInterface(self, object_array_class, 0));
  CHECK_EQ(java_io_Serializable.Get(),
           mirror::Class::GetDirectInterface(self, object_array_class, 1));

  CHECK_EQ(object_array_string.Get(),
           FindSystemClass(self, GetClassRootDescriptor(kJavaLangStringArrayClass)));

  // End of special init trickery, all subsequent classes may be loaded via FindSystemClass.

  // Create java.lang.reflect.Proxy root.
  SetClassRoot(kJavaLangReflectProxy, FindSystemClass(self, "Ljava/lang/reflect/Proxy;"));

  // Create java.lang.reflect.Field.class root.
  auto* class_root = FindSystemClass(self, "Ljava/lang/reflect/Field;");
  CHECK(class_root != nullptr);
  SetClassRoot(kJavaLangReflectField, class_root);
  mirror::Field::SetClass(class_root);

  // Create java.lang.reflect.Field array root.
  class_root = FindSystemClass(self, "[Ljava/lang/reflect/Field;");
  CHECK(class_root != nullptr);
  SetClassRoot(kJavaLangReflectFieldArrayClass, class_root);
  mirror::Field::SetArrayClass(class_root);

  // Create java.lang.reflect.Constructor.class root and array root.
  class_root = FindSystemClass(self, "Ljava/lang/reflect/Constructor;");
  CHECK(class_root != nullptr);
  SetClassRoot(kJavaLangReflectConstructor, class_root);
  mirror::Constructor::SetClass(class_root);
  class_root = FindSystemClass(self, "[Ljava/lang/reflect/Constructor;");
  CHECK(class_root != nullptr);
  SetClassRoot(kJavaLangReflectConstructorArrayClass, class_root);
  mirror::Constructor::SetArrayClass(class_root);

  // Create java.lang.reflect.Method.class root and array root.
  class_root = FindSystemClass(self, "Ljava/lang/reflect/Method;");
  CHECK(class_root != nullptr);
  SetClassRoot(kJavaLangReflectMethod, class_root);
  mirror::Method::SetClass(class_root);
  class_root = FindSystemClass(self, "[Ljava/lang/reflect/Method;");
  CHECK(class_root != nullptr);
  SetClassRoot(kJavaLangReflectMethodArrayClass, class_root);
  mirror::Method::SetArrayClass(class_root);

  // java.lang.ref classes need to be specially flagged, but otherwise are normal classes
  // finish initializing Reference class
  mirror::Class::SetStatus(java_lang_ref_Reference, mirror::Class::kStatusNotReady, self);
  CheckSystemClass(self, java_lang_ref_Reference, "Ljava/lang/ref/Reference;");
  CHECK_EQ(java_lang_ref_Reference->GetObjectSize(), mirror::Reference::InstanceSize());
  CHECK_EQ(java_lang_ref_Reference->GetClassSize(),
           mirror::Reference::ClassSize(image_pointer_size_));
  class_root = FindSystemClass(self, "Ljava/lang/ref/FinalizerReference;");
  CHECK_EQ(class_root->GetClassFlags(), mirror::kClassFlagNormal);
  class_root->SetClassFlags(class_root->GetClassFlags() | mirror::kClassFlagFinalizerReference);
  class_root = FindSystemClass(self, "Ljava/lang/ref/PhantomReference;");
  CHECK_EQ(class_root->GetClassFlags(), mirror::kClassFlagNormal);
  class_root->SetClassFlags(class_root->GetClassFlags() | mirror::kClassFlagPhantomReference);
  class_root = FindSystemClass(self, "Ljava/lang/ref/SoftReference;");
  CHECK_EQ(class_root->GetClassFlags(), mirror::kClassFlagNormal);
  class_root->SetClassFlags(class_root->GetClassFlags() | mirror::kClassFlagSoftReference);
  class_root = FindSystemClass(self, "Ljava/lang/ref/WeakReference;");
  CHECK_EQ(class_root->GetClassFlags(), mirror::kClassFlagNormal);
  class_root->SetClassFlags(class_root->GetClassFlags() | mirror::kClassFlagWeakReference);

  // Setup the ClassLoader, verifying the object_size_.
  class_root = FindSystemClass(self, "Ljava/lang/ClassLoader;");
  class_root->SetClassLoaderClass();
  CHECK_EQ(class_root->GetObjectSize(), mirror::ClassLoader::InstanceSize());
  SetClassRoot(kJavaLangClassLoader, class_root);

  // Set up java.lang.Throwable, java.lang.ClassNotFoundException, and
  // java.lang.StackTraceElement as a convenience.
  SetClassRoot(kJavaLangThrowable, FindSystemClass(self, "Ljava/lang/Throwable;"));
  mirror::Throwable::SetClass(GetClassRoot(kJavaLangThrowable));
  SetClassRoot(kJavaLangClassNotFoundException,
               FindSystemClass(self, "Ljava/lang/ClassNotFoundException;"));
  SetClassRoot(kJavaLangStackTraceElement, FindSystemClass(self, "Ljava/lang/StackTraceElement;"));
  SetClassRoot(kJavaLangStackTraceElementArrayClass,
               FindSystemClass(self, "[Ljava/lang/StackTraceElement;"));
  mirror::StackTraceElement::SetClass(GetClassRoot(kJavaLangStackTraceElement));

  // Ensure void type is resolved in the core's dex cache so java.lang.Void is correctly
  // initialized.
  {
    const DexFile& dex_file = java_lang_Object->GetDexFile();
    const DexFile::TypeId* void_type_id = dex_file.FindTypeId("V");
    CHECK(void_type_id != nullptr);
    uint16_t void_type_idx = dex_file.GetIndexForTypeId(*void_type_id);
    // Now we resolve void type so the dex cache contains it. We use java.lang.Object class
    // as referrer so the used dex cache is core's one.
    mirror::Class* resolved_type = ResolveType(dex_file, void_type_idx, java_lang_Object.Get());
    CHECK_EQ(resolved_type, GetClassRoot(kPrimitiveVoid));
    self->AssertNoPendingException();
  }

  // Create conflict tables that depend on the class linker.
  runtime->FixupConflictTables();

  FinishInit(self);

  VLOG(startup) << "ClassLinker::InitFromCompiler exiting";

  return true;
}

void ClassLinker::FinishInit(Thread* self) {
  VLOG(startup) << "ClassLinker::FinishInit entering";

  // Let the heap know some key offsets into java.lang.ref instances
  // Note: we hard code the field indexes here rather than using FindInstanceField
  // as the types of the field can't be resolved prior to the runtime being
  // fully initialized
  mirror::Class* java_lang_ref_Reference = GetClassRoot(kJavaLangRefReference);
  mirror::Class* java_lang_ref_FinalizerReference =
      FindSystemClass(self, "Ljava/lang/ref/FinalizerReference;");

  ArtField* pendingNext = java_lang_ref_Reference->GetInstanceField(0);
  CHECK_STREQ(pendingNext->GetName(), "pendingNext");
  CHECK_STREQ(pendingNext->GetTypeDescriptor(), "Ljava/lang/ref/Reference;");

  ArtField* queue = java_lang_ref_Reference->GetInstanceField(1);
  CHECK_STREQ(queue->GetName(), "queue");
  CHECK_STREQ(queue->GetTypeDescriptor(), "Ljava/lang/ref/ReferenceQueue;");

  ArtField* queueNext = java_lang_ref_Reference->GetInstanceField(2);
  CHECK_STREQ(queueNext->GetName(), "queueNext");
  CHECK_STREQ(queueNext->GetTypeDescriptor(), "Ljava/lang/ref/Reference;");

  ArtField* referent = java_lang_ref_Reference->GetInstanceField(3);
  CHECK_STREQ(referent->GetName(), "referent");
  CHECK_STREQ(referent->GetTypeDescriptor(), "Ljava/lang/Object;");

  ArtField* zombie = java_lang_ref_FinalizerReference->GetInstanceField(2);
  CHECK_STREQ(zombie->GetName(), "zombie");
  CHECK_STREQ(zombie->GetTypeDescriptor(), "Ljava/lang/Object;");

  // ensure all class_roots_ are initialized
  for (size_t i = 0; i < kClassRootsMax; i++) {
    ClassRoot class_root = static_cast<ClassRoot>(i);
    mirror::Class* klass = GetClassRoot(class_root);
    CHECK(klass != nullptr);
    DCHECK(klass->IsArrayClass() || klass->IsPrimitive() || klass->GetDexCache() != nullptr);
    // note SetClassRoot does additional validation.
    // if possible add new checks there to catch errors early
  }

  CHECK(!array_iftable_.IsNull());

  // disable the slow paths in FindClass and CreatePrimitiveClass now
  // that Object, Class, and Object[] are setup
  init_done_ = true;

  VLOG(startup) << "ClassLinker::FinishInit exiting";
}

void ClassLinker::RunRootClinits() {
  Thread* self = Thread::Current();
  for (size_t i = 0; i < ClassLinker::kClassRootsMax; ++i) {
    mirror::Class* c = GetClassRoot(ClassRoot(i));
    if (!c->IsArrayClass() && !c->IsPrimitive()) {
      StackHandleScope<1> hs(self);
      Handle<mirror::Class> h_class(hs.NewHandle(GetClassRoot(ClassRoot(i))));
      EnsureInitialized(self, h_class, true, true);
      self->AssertNoPendingException();
    }
  }
}

static void SanityCheckArtMethod(ArtMethod* m,
                                 mirror::Class* expected_class,
                                 const std::vector<gc::space::ImageSpace*>& spaces)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  if (m->IsRuntimeMethod()) {
    mirror::Class* declaring_class = m->GetDeclaringClassUnchecked();
    CHECK(declaring_class == nullptr) << declaring_class << " " << PrettyMethod(m);
  } else if (m->IsCopied()) {
    CHECK(m->GetDeclaringClass() != nullptr) << PrettyMethod(m);
  } else if (expected_class != nullptr) {
    CHECK_EQ(m->GetDeclaringClassUnchecked(), expected_class) << PrettyMethod(m);
  }
  if (!spaces.empty()) {
    bool contains = false;
    for (gc::space::ImageSpace* space : spaces) {
      auto& header = space->GetImageHeader();
      size_t offset = reinterpret_cast<uint8_t*>(m) - space->Begin();

      const ImageSection& methods = header.GetMethodsSection();
      contains = contains || methods.Contains(offset);

      const ImageSection& runtime_methods = header.GetRuntimeMethodsSection();
      contains = contains || runtime_methods.Contains(offset);
    }
    CHECK(contains) << m << " not found";
  }
}

static void SanityCheckArtMethodPointerArray(mirror::PointerArray* arr,
                                             mirror::Class* expected_class,
                                             size_t pointer_size,
                                             const std::vector<gc::space::ImageSpace*>& spaces)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  CHECK(arr != nullptr);
  for (int32_t j = 0; j < arr->GetLength(); ++j) {
    auto* method = arr->GetElementPtrSize<ArtMethod*>(j, pointer_size);
    // expected_class == null means we are a dex cache.
    if (expected_class != nullptr) {
      CHECK(method != nullptr);
    }
    if (method != nullptr) {
      SanityCheckArtMethod(method, expected_class, spaces);
    }
  }
}

static void SanityCheckArtMethodPointerArray(ArtMethod** arr,
                                             size_t size,
                                             size_t pointer_size,
                                             const std::vector<gc::space::ImageSpace*>& spaces)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  CHECK_EQ(arr != nullptr, size != 0u);
  if (arr != nullptr) {
    bool contains = false;
    for (auto space : spaces) {
      auto offset = reinterpret_cast<uint8_t*>(arr) - space->Begin();
      if (space->GetImageHeader().GetImageSection(
          ImageHeader::kSectionDexCacheArrays).Contains(offset)) {
        contains = true;
        break;
      }
    }
    CHECK(contains);
  }
  for (size_t j = 0; j < size; ++j) {
    ArtMethod* method = mirror::DexCache::GetElementPtrSize(arr, j, pointer_size);
    // expected_class == null means we are a dex cache.
    if (method != nullptr) {
      SanityCheckArtMethod(method, nullptr, spaces);
    }
  }
}

static void SanityCheckObjectsCallback(mirror::Object* obj, void* arg ATTRIBUTE_UNUSED)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  DCHECK(obj != nullptr);
  CHECK(obj->GetClass() != nullptr) << "Null class in object " << obj;
  CHECK(obj->GetClass()->GetClass() != nullptr) << "Null class class " << obj;
  if (obj->IsClass()) {
    auto klass = obj->AsClass();
    for (ArtField& field : klass->GetIFields()) {
      CHECK_EQ(field.GetDeclaringClass(), klass);
    }
    for (ArtField& field : klass->GetSFields()) {
      CHECK_EQ(field.GetDeclaringClass(), klass);
    }
    auto* runtime = Runtime::Current();
    auto image_spaces = runtime->GetHeap()->GetBootImageSpaces();
    auto pointer_size = runtime->GetClassLinker()->GetImagePointerSize();
    for (auto& m : klass->GetMethods(pointer_size)) {
      SanityCheckArtMethod(&m, klass, image_spaces);
    }
    auto* vtable = klass->GetVTable();
    if (vtable != nullptr) {
      SanityCheckArtMethodPointerArray(vtable, nullptr, pointer_size, image_spaces);
    }
    if (klass->ShouldHaveEmbeddedImtAndVTable()) {
      for (size_t i = 0; i < mirror::Class::kImtSize; ++i) {
        SanityCheckArtMethod(
            klass->GetEmbeddedImTableEntry(i, pointer_size), nullptr, image_spaces);
      }
      for (int32_t i = 0; i < klass->GetEmbeddedVTableLength(); ++i) {
        SanityCheckArtMethod(klass->GetEmbeddedVTableEntry(i, pointer_size), nullptr, image_spaces);
      }
    }
    auto* iftable = klass->GetIfTable();
    if (iftable != nullptr) {
      for (int32_t i = 0; i < klass->GetIfTableCount(); ++i) {
        if (iftable->GetMethodArrayCount(i) > 0) {
          SanityCheckArtMethodPointerArray(
              iftable->GetMethodArray(i), nullptr, pointer_size, image_spaces);
        }
      }
    }
  }
}

// Set image methods' entry point to interpreter.
class SetInterpreterEntrypointArtMethodVisitor : public ArtMethodVisitor {
 public:
  explicit SetInterpreterEntrypointArtMethodVisitor(size_t image_pointer_size)
    : image_pointer_size_(image_pointer_size) {}

  void Visit(ArtMethod* method) OVERRIDE SHARED_REQUIRES(Locks::mutator_lock_) {
    if (kIsDebugBuild && !method->IsRuntimeMethod()) {
      CHECK(method->GetDeclaringClass() != nullptr);
    }
    if (!method->IsNative() && !method->IsRuntimeMethod() && !method->IsResolutionMethod()) {
      method->SetEntryPointFromQuickCompiledCodePtrSize(GetQuickToInterpreterBridge(),
                                                        image_pointer_size_);
    }
  }

 private:
  const size_t image_pointer_size_;

  DISALLOW_COPY_AND_ASSIGN(SetInterpreterEntrypointArtMethodVisitor);
};

struct TrampolineCheckData {
  const void* quick_resolution_trampoline;
  const void* quick_imt_conflict_trampoline;
  const void* quick_generic_jni_trampoline;
  const void* quick_to_interpreter_bridge_trampoline;
  size_t pointer_size;
  ArtMethod* m;
  bool error;
};

static void CheckTrampolines(mirror::Object* obj, void* arg) NO_THREAD_SAFETY_ANALYSIS {
  if (obj->IsClass()) {
    mirror::Class* klass = obj->AsClass();
    TrampolineCheckData* d = reinterpret_cast<TrampolineCheckData*>(arg);
    for (ArtMethod& m : klass->GetMethods(d->pointer_size)) {
      const void* entrypoint = m.GetEntryPointFromQuickCompiledCodePtrSize(d->pointer_size);
      if (entrypoint == d->quick_resolution_trampoline ||
          entrypoint == d->quick_imt_conflict_trampoline ||
          entrypoint == d->quick_generic_jni_trampoline ||
          entrypoint == d->quick_to_interpreter_bridge_trampoline) {
        d->m = &m;
        d->error = true;
        return;
      }
    }
  }
}

bool ClassLinker::InitFromBootImage(std::string* error_msg) {
  VLOG(startup) << __FUNCTION__ << " entering";
  CHECK(!init_done_);

  Runtime* const runtime = Runtime::Current();
  Thread* const self = Thread::Current();
  gc::Heap* const heap = runtime->GetHeap();
  std::vector<gc::space::ImageSpace*> spaces = heap->GetBootImageSpaces();
  CHECK(!spaces.empty());
  image_pointer_size_ = spaces[0]->GetImageHeader().GetPointerSize();
  if (!ValidPointerSize(image_pointer_size_)) {
    *error_msg = StringPrintf("Invalid image pointer size: %zu", image_pointer_size_);
    return false;
  }
  if (!runtime->IsAotCompiler()) {
    // Only the Aot compiler supports having an image with a different pointer size than the
    // runtime. This happens on the host for compiling 32 bit tests since we use a 64 bit libart
    // compiler. We may also use 32 bit dex2oat on a system with 64 bit apps.
    if (image_pointer_size_ != sizeof(void*)) {
      *error_msg = StringPrintf("Runtime must use current image pointer size: %zu vs %zu",
                                image_pointer_size_,
                                sizeof(void*));
      return false;
    }
  }
  dex_cache_boot_image_class_lookup_required_ = true;
  std::vector<const OatFile*> oat_files =
      runtime->GetOatFileManager().RegisterImageOatFiles(spaces);
  DCHECK(!oat_files.empty());
  const OatHeader& default_oat_header = oat_files[0]->GetOatHeader();
  CHECK_EQ(default_oat_header.GetImageFileLocationOatChecksum(), 0U);
  CHECK_EQ(default_oat_header.GetImageFileLocationOatDataBegin(), 0U);
  const char* image_file_location = oat_files[0]->GetOatHeader().
      GetStoreValueByKey(OatHeader::kImageLocationKey);
  CHECK(image_file_location == nullptr || *image_file_location == 0);
  quick_resolution_trampoline_ = default_oat_header.GetQuickResolutionTrampoline();
  quick_imt_conflict_trampoline_ = default_oat_header.GetQuickImtConflictTrampoline();
  quick_generic_jni_trampoline_ = default_oat_header.GetQuickGenericJniTrampoline();
  quick_to_interpreter_bridge_trampoline_ = default_oat_header.GetQuickToInterpreterBridge();
  if (kIsDebugBuild) {
    // Check that the other images use the same trampoline.
    for (size_t i = 1; i < oat_files.size(); ++i) {
      const OatHeader& ith_oat_header = oat_files[i]->GetOatHeader();
      const void* ith_quick_resolution_trampoline =
          ith_oat_header.GetQuickResolutionTrampoline();
      const void* ith_quick_imt_conflict_trampoline =
          ith_oat_header.GetQuickImtConflictTrampoline();
      const void* ith_quick_generic_jni_trampoline =
          ith_oat_header.GetQuickGenericJniTrampoline();
      const void* ith_quick_to_interpreter_bridge_trampoline =
          ith_oat_header.GetQuickToInterpreterBridge();
      if (ith_quick_resolution_trampoline != quick_resolution_trampoline_ ||
          ith_quick_imt_conflict_trampoline != quick_imt_conflict_trampoline_ ||
          ith_quick_generic_jni_trampoline != quick_generic_jni_trampoline_ ||
          ith_quick_to_interpreter_bridge_trampoline != quick_to_interpreter_bridge_trampoline_) {
        // Make sure that all methods in this image do not contain those trampolines as
        // entrypoints. Otherwise the class-linker won't be able to work with a single set.
        TrampolineCheckData data;
        data.error = false;
        data.pointer_size = GetImagePointerSize();
        data.quick_resolution_trampoline = ith_quick_resolution_trampoline;
        data.quick_imt_conflict_trampoline = ith_quick_imt_conflict_trampoline;
        data.quick_generic_jni_trampoline = ith_quick_generic_jni_trampoline;
        data.quick_to_interpreter_bridge_trampoline = ith_quick_to_interpreter_bridge_trampoline;
        ReaderMutexLock mu(self, *Locks::heap_bitmap_lock_);
        spaces[i]->GetLiveBitmap()->Walk(CheckTrampolines, &data);
        if (data.error) {
          ArtMethod* m = data.m;
          LOG(ERROR) << "Found a broken ArtMethod: " << PrettyMethod(m);
          *error_msg = "Found an ArtMethod with a bad entrypoint";
          return false;
        }
      }
    }
  }

  class_roots_ = GcRoot<mirror::ObjectArray<mirror::Class>>(
      down_cast<mirror::ObjectArray<mirror::Class>*>(
          spaces[0]->GetImageHeader().GetImageRoot(ImageHeader::kClassRoots)));
  mirror::Class::SetClassClass(class_roots_.Read()->Get(kJavaLangClass));

  // Special case of setting up the String class early so that we can test arbitrary objects
  // as being Strings or not
  mirror::String::SetClass(GetClassRoot(kJavaLangString));

  mirror::Class* java_lang_Object = GetClassRoot(kJavaLangObject);
  java_lang_Object->SetObjectSize(sizeof(mirror::Object));
  // Allocate in non-movable so that it's possible to check if a JNI weak global ref has been
  // cleared without triggering the read barrier and unintentionally mark the sentinel alive.
  runtime->SetSentinel(heap->AllocNonMovableObject<true>(
      self, java_lang_Object, java_lang_Object->GetObjectSize(), VoidFunctor()));

  // reinit array_iftable_ from any array class instance, they should be ==
  array_iftable_ = GcRoot<mirror::IfTable>(GetClassRoot(kObjectArrayClass)->GetIfTable());
  DCHECK_EQ(array_iftable_.Read(), GetClassRoot(kBooleanArrayClass)->GetIfTable());
  // String class root was set above
  mirror::Field::SetClass(GetClassRoot(kJavaLangReflectField));
  mirror::Field::SetArrayClass(GetClassRoot(kJavaLangReflectFieldArrayClass));
  mirror::Constructor::SetClass(GetClassRoot(kJavaLangReflectConstructor));
  mirror::Constructor::SetArrayClass(GetClassRoot(kJavaLangReflectConstructorArrayClass));
  mirror::Method::SetClass(GetClassRoot(kJavaLangReflectMethod));
  mirror::Method::SetArrayClass(GetClassRoot(kJavaLangReflectMethodArrayClass));
  mirror::Reference::SetClass(GetClassRoot(kJavaLangRefReference));
  mirror::BooleanArray::SetArrayClass(GetClassRoot(kBooleanArrayClass));
  mirror::ByteArray::SetArrayClass(GetClassRoot(kByteArrayClass));
  mirror::CharArray::SetArrayClass(GetClassRoot(kCharArrayClass));
  mirror::DoubleArray::SetArrayClass(GetClassRoot(kDoubleArrayClass));
  mirror::FloatArray::SetArrayClass(GetClassRoot(kFloatArrayClass));
  mirror::IntArray::SetArrayClass(GetClassRoot(kIntArrayClass));
  mirror::LongArray::SetArrayClass(GetClassRoot(kLongArrayClass));
  mirror::ShortArray::SetArrayClass(GetClassRoot(kShortArrayClass));
  mirror::Throwable::SetClass(GetClassRoot(kJavaLangThrowable));
  mirror::StackTraceElement::SetClass(GetClassRoot(kJavaLangStackTraceElement));

  for (gc::space::ImageSpace* image_space : spaces) {
    // Boot class loader, use a null handle.
    std::vector<std::unique_ptr<const DexFile>> dex_files;
    if (!AddImageSpace(image_space,
                       ScopedNullHandle<mirror::ClassLoader>(),
                       /*dex_elements*/nullptr,
                       /*dex_location*/nullptr,
                       /*out*/&dex_files,
                       error_msg)) {
      return false;
    }
    // Append opened dex files at the end.
    boot_dex_files_.insert(boot_dex_files_.end(),
                           std::make_move_iterator(dex_files.begin()),
                           std::make_move_iterator(dex_files.end()));
  }
  FinishInit(self);

  VLOG(startup) << __FUNCTION__ << " exiting";
  return true;
}

bool ClassLinker::IsBootClassLoader(ScopedObjectAccessAlreadyRunnable& soa,
                                    mirror::ClassLoader* class_loader) {
  return class_loader == nullptr ||
      class_loader->GetClass() ==
          soa.Decode<mirror::Class*>(WellKnownClasses::java_lang_BootClassLoader);
}

static mirror::String* GetDexPathListElementName(ScopedObjectAccessUnchecked& soa,
                                                 mirror::Object* element)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  ArtField* const dex_file_field =
      soa.DecodeField(WellKnownClasses::dalvik_system_DexPathList__Element_dexFile);
  ArtField* const dex_file_name_field =
      soa.DecodeField(WellKnownClasses::dalvik_system_DexFile_fileName);
  DCHECK(dex_file_field != nullptr);
  DCHECK(dex_file_name_field != nullptr);
  DCHECK(element != nullptr);
  CHECK_EQ(dex_file_field->GetDeclaringClass(), element->GetClass()) << PrettyTypeOf(element);
  mirror::Object* dex_file = dex_file_field->GetObject(element);
  if (dex_file == nullptr) {
    return nullptr;
  }
  mirror::Object* const name_object = dex_file_name_field->GetObject(dex_file);
  if (name_object != nullptr) {
    return name_object->AsString();
  }
  return nullptr;
}

static bool FlattenPathClassLoader(mirror::ClassLoader* class_loader,
                                   std::list<mirror::String*>* out_dex_file_names,
                                   std::string* error_msg)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  DCHECK(out_dex_file_names != nullptr);
  DCHECK(error_msg != nullptr);
  ScopedObjectAccessUnchecked soa(Thread::Current());
  ArtField* const dex_path_list_field =
      soa.DecodeField(WellKnownClasses::dalvik_system_PathClassLoader_pathList);
  ArtField* const dex_elements_field =
      soa.DecodeField(WellKnownClasses::dalvik_system_DexPathList_dexElements);
  CHECK(dex_path_list_field != nullptr);
  CHECK(dex_elements_field != nullptr);
  while (!ClassLinker::IsBootClassLoader(soa, class_loader)) {
    if (class_loader->GetClass() !=
        soa.Decode<mirror::Class*>(WellKnownClasses::dalvik_system_PathClassLoader)) {
      *error_msg = StringPrintf("Unknown class loader type %s", PrettyTypeOf(class_loader).c_str());
      // Unsupported class loader.
      return false;
    }
    mirror::Object* dex_path_list = dex_path_list_field->GetObject(class_loader);
    if (dex_path_list != nullptr) {
      // DexPathList has an array dexElements of Elements[] which each contain a dex file.
      mirror::Object* dex_elements_obj = dex_elements_field->GetObject(dex_path_list);
      // Loop through each dalvik.system.DexPathList$Element's dalvik.system.DexFile and look
      // at the mCookie which is a DexFile vector.
      if (dex_elements_obj != nullptr) {
        mirror::ObjectArray<mirror::Object>* dex_elements =
            dex_elements_obj->AsObjectArray<mirror::Object>();
        // Reverse order since we insert the parent at the front.
        for (int32_t i = dex_elements->GetLength() - 1; i >= 0; --i) {
          mirror::Object* const element = dex_elements->GetWithoutChecks(i);
          if (element == nullptr) {
            *error_msg = StringPrintf("Null dex element at index %d", i);
            return false;
          }
          mirror::String* const name = GetDexPathListElementName(soa, element);
          if (name == nullptr) {
            *error_msg = StringPrintf("Null name for dex element at index %d", i);
            return false;
          }
          out_dex_file_names->push_front(name);
        }
      }
    }
    class_loader = class_loader->GetParent();
  }
  return true;
}

class FixupArtMethodArrayVisitor : public ArtMethodVisitor {
 public:
  explicit FixupArtMethodArrayVisitor(const ImageHeader& header) : header_(header) {}

  virtual void Visit(ArtMethod* method) SHARED_REQUIRES(Locks::mutator_lock_) {
    GcRoot<mirror::Class>* resolved_types = method->GetDexCacheResolvedTypes(sizeof(void*));
    const bool is_copied = method->IsCopied();
    if (resolved_types != nullptr) {
      bool in_image_space = false;
      if (kIsDebugBuild || is_copied) {
        in_image_space = header_.GetImageSection(ImageHeader::kSectionDexCacheArrays).Contains(
            reinterpret_cast<const uint8_t*>(resolved_types) - header_.GetImageBegin());
      }
      // Must be in image space for non-miranda method.
      DCHECK(is_copied || in_image_space)
          << resolved_types << " is not in image starting at "
          << reinterpret_cast<void*>(header_.GetImageBegin());
      if (!is_copied || in_image_space) {
        // Go through the array so that we don't need to do a slow map lookup.
        method->SetDexCacheResolvedTypes(*reinterpret_cast<GcRoot<mirror::Class>**>(resolved_types),
                                         sizeof(void*));
      }
    }
    ArtMethod** resolved_methods = method->GetDexCacheResolvedMethods(sizeof(void*));
    if (resolved_methods != nullptr) {
      bool in_image_space = false;
      if (kIsDebugBuild || is_copied) {
        in_image_space = header_.GetImageSection(ImageHeader::kSectionDexCacheArrays).Contains(
              reinterpret_cast<const uint8_t*>(resolved_methods) - header_.GetImageBegin());
      }
      // Must be in image space for non-miranda method.
      DCHECK(is_copied || in_image_space)
          << resolved_methods << " is not in image starting at "
          << reinterpret_cast<void*>(header_.GetImageBegin());
      if (!is_copied || in_image_space) {
        // Go through the array so that we don't need to do a slow map lookup.
        method->SetDexCacheResolvedMethods(*reinterpret_cast<ArtMethod***>(resolved_methods),
                                           sizeof(void*));
      }
    }
  }

 private:
  const ImageHeader& header_;
};

class VerifyClassInTableArtMethodVisitor : public ArtMethodVisitor {
 public:
  explicit VerifyClassInTableArtMethodVisitor(ClassTable* table) : table_(table) {}

  virtual void Visit(ArtMethod* method)
      SHARED_REQUIRES(Locks::mutator_lock_, Locks::classlinker_classes_lock_) {
    mirror::Class* klass = method->GetDeclaringClass();
    if (klass != nullptr && !Runtime::Current()->GetHeap()->ObjectIsInBootImageSpace(klass)) {
      CHECK_EQ(table_->LookupByDescriptor(klass), klass) << PrettyClass(klass);
    }
  }

 private:
  ClassTable* const table_;
};

class VerifyDeclaringClassVisitor : public ArtMethodVisitor {
 public:
  VerifyDeclaringClassVisitor() SHARED_REQUIRES(Locks::mutator_lock_, Locks::heap_bitmap_lock_)
      : live_bitmap_(Runtime::Current()->GetHeap()->GetLiveBitmap()) {}

  virtual void Visit(ArtMethod* method)
      SHARED_REQUIRES(Locks::mutator_lock_, Locks::heap_bitmap_lock_) {
    mirror::Class* klass = method->GetDeclaringClassUnchecked();
    if (klass != nullptr) {
      CHECK(live_bitmap_->Test(klass)) << "Image method has unmarked declaring class";
    }
  }

 private:
  gc::accounting::HeapBitmap* const live_bitmap_;
};

bool ClassLinker::UpdateAppImageClassLoadersAndDexCaches(
    gc::space::ImageSpace* space,
    Handle<mirror::ClassLoader> class_loader,
    Handle<mirror::ObjectArray<mirror::DexCache>> dex_caches,
    ClassTable::ClassSet* new_class_set,
    bool* out_forward_dex_cache_array,
    std::string* out_error_msg) {
  DCHECK(out_forward_dex_cache_array != nullptr);
  DCHECK(out_error_msg != nullptr);
  Thread* const self = Thread::Current();
  gc::Heap* const heap = Runtime::Current()->GetHeap();
  const ImageHeader& header = space->GetImageHeader();
  {
    // Add image classes into the class table for the class loader, and fixup the dex caches and
    // class loader fields.
    WriterMutexLock mu(self, *Locks::classlinker_classes_lock_);
    ClassTable* table = InsertClassTableForClassLoader(class_loader.Get());
    // Dex cache array fixup is all or nothing, we must reject app images that have mixed since we
    // rely on clobering the dex cache arrays in the image to forward to bss.
    size_t num_dex_caches_with_bss_arrays = 0;
    const size_t num_dex_caches = dex_caches->GetLength();
    for (size_t i = 0; i < num_dex_caches; i++) {
      mirror::DexCache* const dex_cache = dex_caches->Get(i);
      const DexFile* const dex_file = dex_cache->GetDexFile();
      const OatFile::OatDexFile* oat_dex_file = dex_file->GetOatDexFile();
      if (oat_dex_file != nullptr && oat_dex_file->GetDexCacheArrays() != nullptr) {
        ++num_dex_caches_with_bss_arrays;
      }
    }
    *out_forward_dex_cache_array = num_dex_caches_with_bss_arrays != 0;
    if (*out_forward_dex_cache_array) {
      if (num_dex_caches_with_bss_arrays != num_dex_caches) {
        // Reject application image since we cannot forward only some of the dex cache arrays.
        // TODO: We could get around this by having a dedicated forwarding slot. It should be an
        // uncommon case.
        *out_error_msg = StringPrintf("Dex caches in bss does not match total: %zu vs %zu",
                                      num_dex_caches_with_bss_arrays,
                                      num_dex_caches);
        return false;
      }
    }
    // Only add the classes to the class loader after the points where we can return false.
    for (size_t i = 0; i < num_dex_caches; i++) {
      mirror::DexCache* const dex_cache = dex_caches->Get(i);
      const DexFile* const dex_file = dex_cache->GetDexFile();
      const OatFile::OatDexFile* oat_dex_file = dex_file->GetOatDexFile();
      if (oat_dex_file != nullptr && oat_dex_file->GetDexCacheArrays() != nullptr) {
      // If the oat file expects the dex cache arrays to be in the BSS, then allocate there and
        // copy over the arrays.
        DCHECK(dex_file != nullptr);
        const size_t num_strings = dex_file->NumStringIds();
        const size_t num_types = dex_file->NumTypeIds();
        const size_t num_methods = dex_file->NumMethodIds();
        const size_t num_fields = dex_file->NumFieldIds();
        CHECK_EQ(num_strings, dex_cache->NumStrings());
        CHECK_EQ(num_types, dex_cache->NumResolvedTypes());
        CHECK_EQ(num_methods, dex_cache->NumResolvedMethods());
        CHECK_EQ(num_fields, dex_cache->NumResolvedFields());
        DexCacheArraysLayout layout(image_pointer_size_, dex_file);
        uint8_t* const raw_arrays = oat_dex_file->GetDexCacheArrays();
        // The space is not yet visible to the GC, we can avoid the read barriers and use
        // std::copy_n.
        if (num_strings != 0u) {
          GcRoot<mirror::String>* const image_resolved_strings = dex_cache->GetStrings();
          GcRoot<mirror::String>* const strings =
              reinterpret_cast<GcRoot<mirror::String>*>(raw_arrays + layout.StringsOffset());
          for (size_t j = 0; kIsDebugBuild && j < num_strings; ++j) {
            DCHECK(strings[j].IsNull());
          }
          std::copy_n(image_resolved_strings, num_strings, strings);
          dex_cache->SetStrings(strings);
        }
        if (num_types != 0u) {
          GcRoot<mirror::Class>* const image_resolved_types = dex_cache->GetResolvedTypes();
          GcRoot<mirror::Class>* const types =
              reinterpret_cast<GcRoot<mirror::Class>*>(raw_arrays + layout.TypesOffset());
          for (size_t j = 0; kIsDebugBuild && j < num_types; ++j) {
            DCHECK(types[j].IsNull());
          }
          std::copy_n(image_resolved_types, num_types, types);
          // Store a pointer to the new location for fast ArtMethod patching without requiring map.
          // This leaves random garbage at the start of the dex cache array, but nobody should ever
          // read from it again.
          *reinterpret_cast<GcRoot<mirror::Class>**>(image_resolved_types) = types;
          dex_cache->SetResolvedTypes(types);
        }
        if (num_methods != 0u) {
          ArtMethod** const methods = reinterpret_cast<ArtMethod**>(
              raw_arrays + layout.MethodsOffset());
          ArtMethod** const image_resolved_methods = dex_cache->GetResolvedMethods();
          for (size_t j = 0; kIsDebugBuild && j < num_methods; ++j) {
            DCHECK(methods[j] == nullptr);
          }
          std::copy_n(image_resolved_methods, num_methods, methods);
          // Store a pointer to the new location for fast ArtMethod patching without requiring map.
          *reinterpret_cast<ArtMethod***>(image_resolved_methods) = methods;
          dex_cache->SetResolvedMethods(methods);
        }
        if (num_fields != 0u) {
          ArtField** const fields =
              reinterpret_cast<ArtField**>(raw_arrays + layout.FieldsOffset());
          for (size_t j = 0; kIsDebugBuild && j < num_fields; ++j) {
            DCHECK(fields[j] == nullptr);
          }
          std::copy_n(dex_cache->GetResolvedFields(), num_fields, fields);
          dex_cache->SetResolvedFields(fields);
        }
      }
      {
        WriterMutexLock mu2(self, dex_lock_);
        // Make sure to do this after we update the arrays since we store the resolved types array
        // in DexCacheData in RegisterDexFileLocked. We need the array pointer to be the one in the
        // BSS.
        mirror::DexCache* existing_dex_cache = FindDexCacheLocked(self,
                                                                  *dex_file,
                                                                  /*allow_failure*/true);
        CHECK(existing_dex_cache == nullptr);
        StackHandleScope<1> hs3(self);
        RegisterDexFileLocked(*dex_file, hs3.NewHandle(dex_cache));
      }
      GcRoot<mirror::Class>* const types = dex_cache->GetResolvedTypes();
      const size_t num_types = dex_cache->NumResolvedTypes();
      if (new_class_set == nullptr) {
        for (int32_t j = 0; j < static_cast<int32_t>(num_types); j++) {
          // The image space is not yet added to the heap, avoid read barriers.
          mirror::Class* klass = types[j].Read();
          // There may also be boot image classes,
          if (space->HasAddress(klass)) {
            DCHECK_NE(klass->GetStatus(), mirror::Class::kStatusError);
            // Update the class loader from the one in the image class loader to the one that loaded
            // the app image.
            klass->SetClassLoader(class_loader.Get());
            // The resolved type could be from another dex cache, go through the dex cache just in
            // case. May be null for array classes.
            if (klass->GetDexCacheStrings() != nullptr) {
              DCHECK(!klass->IsArrayClass());
              klass->SetDexCacheStrings(klass->GetDexCache()->GetStrings());
            }
            // If there are multiple dex caches, there may be the same class multiple times
            // in different dex caches. Check for this since inserting will add duplicates
            // otherwise.
            if (num_dex_caches > 1) {
              mirror::Class* existing = table->LookupByDescriptor(klass);
              if (existing != nullptr) {
                DCHECK_EQ(existing, klass) << PrettyClass(klass);
              } else {
                table->Insert(klass);
              }
            } else {
              table->Insert(klass);
            }
            // Double checked VLOG to avoid overhead.
            if (VLOG_IS_ON(image)) {
              VLOG(image) << PrettyClass(klass) << " " << klass->GetStatus();
              if (!klass->IsArrayClass()) {
                VLOG(image) << "From " << klass->GetDexCache()->GetDexFile()->GetBaseLocation();
              }
              VLOG(image) << "Direct methods";
              for (ArtMethod& m : klass->GetDirectMethods(sizeof(void*))) {
                VLOG(image) << PrettyMethod(&m);
              }
              VLOG(image) << "Virtual methods";
              for (ArtMethod& m : klass->GetVirtualMethods(sizeof(void*))) {
                VLOG(image) << PrettyMethod(&m);
              }
            }
          } else {
            DCHECK(klass == nullptr || heap->ObjectIsInBootImageSpace(klass))
                << klass << " " << PrettyClass(klass);
          }
        }
      }
      if (kIsDebugBuild) {
        for (int32_t j = 0; j < static_cast<int32_t>(num_types); j++) {
          // The image space is not yet added to the heap, avoid read barriers.
          mirror::Class* klass = types[j].Read();
          if (space->HasAddress(klass)) {
            DCHECK_NE(klass->GetStatus(), mirror::Class::kStatusError);
            if (kIsDebugBuild) {
              if (new_class_set != nullptr) {
                auto it = new_class_set->Find(GcRoot<mirror::Class>(klass));
                DCHECK(it != new_class_set->end());
                DCHECK_EQ(it->Read(), klass);
                mirror::Class* super_class = klass->GetSuperClass();
                if (super_class != nullptr && !heap->ObjectIsInBootImageSpace(super_class)) {
                  auto it2 = new_class_set->Find(GcRoot<mirror::Class>(super_class));
                  DCHECK(it2 != new_class_set->end());
                  DCHECK_EQ(it2->Read(), super_class);
                }
              } else {
                DCHECK_EQ(table->LookupByDescriptor(klass), klass);
                mirror::Class* super_class = klass->GetSuperClass();
                if (super_class != nullptr && !heap->ObjectIsInBootImageSpace(super_class)) {
                  CHECK_EQ(table->LookupByDescriptor(super_class), super_class);
                }
              }
            }
            if (kIsDebugBuild) {
              for (ArtMethod& m : klass->GetDirectMethods(sizeof(void*))) {
                const void* code = m.GetEntryPointFromQuickCompiledCode();
                const void* oat_code = m.IsInvokable() ? GetQuickOatCodeFor(&m) : code;
                if (!IsQuickResolutionStub(code) &&
                    !IsQuickGenericJniStub(code) &&
                    !IsQuickToInterpreterBridge(code) &&
                    !m.IsNative()) {
                  DCHECK_EQ(code, oat_code) << PrettyMethod(&m);
                }
              }
              for (ArtMethod& m : klass->GetVirtualMethods(sizeof(void*))) {
                const void* code = m.GetEntryPointFromQuickCompiledCode();
                const void* oat_code = m.IsInvokable() ? GetQuickOatCodeFor(&m) : code;
                if (!IsQuickResolutionStub(code) &&
                    !IsQuickGenericJniStub(code) &&
                    !IsQuickToInterpreterBridge(code) &&
                    !m.IsNative()) {
                  DCHECK_EQ(code, oat_code) << PrettyMethod(&m);
                }
              }
            }
          }
        }
      }
    }
  }
  if (*out_forward_dex_cache_array) {
    ScopedTrace timing("Fixup ArtMethod dex cache arrays");
    FixupArtMethodArrayVisitor visitor(header);
    header.VisitPackedArtMethods(&visitor, space->Begin(), sizeof(void*));
    Runtime::Current()->GetHeap()->WriteBarrierEveryFieldOf(class_loader.Get());
  }
  if (kVerifyArtMethodDeclaringClasses) {
    ScopedTrace timing("Verify declaring classes");
    ReaderMutexLock rmu(self, *Locks::heap_bitmap_lock_);
    VerifyDeclaringClassVisitor visitor;
    header.VisitPackedArtMethods(&visitor, space->Begin(), sizeof(void*));
  }
  return true;
}

// Update the class loader and resolved string dex cache array of classes. Should only be used on
// classes in the image space.
class UpdateClassLoaderAndResolvedStringsVisitor {
 public:
  UpdateClassLoaderAndResolvedStringsVisitor(gc::space::ImageSpace* space,
                                             mirror::ClassLoader* class_loader,
                                             bool forward_strings)
      : space_(space),
        class_loader_(class_loader),
        forward_strings_(forward_strings) {}

  bool operator()(mirror::Class* klass) const SHARED_REQUIRES(Locks::mutator_lock_) {
    if (forward_strings_) {
      GcRoot<mirror::String>* strings = klass->GetDexCacheStrings();
      if (strings != nullptr) {
        DCHECK(
            space_->GetImageHeader().GetImageSection(ImageHeader::kSectionDexCacheArrays).Contains(
                reinterpret_cast<uint8_t*>(strings) - space_->Begin()))
            << "String dex cache array for " << PrettyClass(klass) << " is not in app image";
        // Dex caches have already been updated, so take the strings pointer from there.
        GcRoot<mirror::String>* new_strings = klass->GetDexCache()->GetStrings();
        DCHECK_NE(strings, new_strings);
        klass->SetDexCacheStrings(new_strings);
      }
    }
    // Finally, update class loader.
    klass->SetClassLoader(class_loader_);
    return true;
  }

  gc::space::ImageSpace* const space_;
  mirror::ClassLoader* const class_loader_;
  const bool forward_strings_;
};

static std::unique_ptr<const DexFile> OpenOatDexFile(const OatFile* oat_file,
                                                     const char* location,
                                                     std::string* error_msg)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  DCHECK(error_msg != nullptr);
  std::unique_ptr<const DexFile> dex_file;
  const OatFile::OatDexFile* oat_dex_file = oat_file->GetOatDexFile(location, nullptr);
  if (oat_dex_file == nullptr) {
    *error_msg = StringPrintf("Failed finding oat dex file for %s %s",
                              oat_file->GetLocation().c_str(),
                              location);
    return std::unique_ptr<const DexFile>();
  }
  std::string inner_error_msg;
  dex_file = oat_dex_file->OpenDexFile(&inner_error_msg);
  if (dex_file == nullptr) {
    *error_msg = StringPrintf("Failed to open dex file %s from within oat file %s error '%s'",
                              location,
                              oat_file->GetLocation().c_str(),
                              inner_error_msg.c_str());
    return std::unique_ptr<const DexFile>();
  }

  if (dex_file->GetLocationChecksum() != oat_dex_file->GetDexFileLocationChecksum()) {
    *error_msg = StringPrintf("Checksums do not match for %s: %x vs %x",
                              location,
                              dex_file->GetLocationChecksum(),
                              oat_dex_file->GetDexFileLocationChecksum());
    return std::unique_ptr<const DexFile>();
  }
  return dex_file;
}

bool ClassLinker::OpenImageDexFiles(gc::space::ImageSpace* space,
                                    std::vector<std::unique_ptr<const DexFile>>* out_dex_files,
                                    std::string* error_msg) {
  ScopedAssertNoThreadSuspension nts(Thread::Current(), __FUNCTION__);
  const ImageHeader& header = space->GetImageHeader();
  mirror::Object* dex_caches_object = header.GetImageRoot(ImageHeader::kDexCaches);
  DCHECK(dex_caches_object != nullptr);
  mirror::ObjectArray<mirror::DexCache>* dex_caches =
      dex_caches_object->AsObjectArray<mirror::DexCache>();
  const OatFile* oat_file = space->GetOatFile();
  for (int32_t i = 0; i < dex_caches->GetLength(); i++) {
    mirror::DexCache* dex_cache = dex_caches->Get(i);
    std::string dex_file_location(dex_cache->GetLocation()->ToModifiedUtf8());
    std::unique_ptr<const DexFile> dex_file = OpenOatDexFile(oat_file,
                                                             dex_file_location.c_str(),
                                                             error_msg);
    if (dex_file == nullptr) {
      return false;
    }
    dex_cache->SetDexFile(dex_file.get());
    out_dex_files->push_back(std::move(dex_file));
  }
  return true;
}

class CHAClassVisitor : public ClassVisitor {
 public:
  explicit CHAClassVisitor(ClassLinker* linker) : class_linker(linker) {}

  bool operator()(mirror::Class* klass) OVERRIDE SHARED_REQUIRES(Locks::mutator_lock_) {
    mirror::Class* parent = klass->GetSuperClass();
    if (parent != nullptr) {
      class_linker->AddCHA(parent, klass);
    }
    return true;
  }

 private:
  ClassLinker* class_linker;
};

bool ClassLinker::AddImageSpace(
    gc::space::ImageSpace* space,
    Handle<mirror::ClassLoader> class_loader,
    jobjectArray dex_elements,
    const char* dex_location,
    std::vector<std::unique_ptr<const DexFile>>* out_dex_files,
    std::string* error_msg) {
  DCHECK(out_dex_files != nullptr);
  DCHECK(error_msg != nullptr);
  const uint64_t start_time = NanoTime();
  const bool app_image = class_loader.Get() != nullptr;
  const ImageHeader& header = space->GetImageHeader();
  mirror::Object* dex_caches_object = header.GetImageRoot(ImageHeader::kDexCaches);
  DCHECK(dex_caches_object != nullptr);
  Runtime* const runtime = Runtime::Current();
  gc::Heap* const heap = runtime->GetHeap();
  Thread* const self = Thread::Current();
  StackHandleScope<2> hs(self);
  Handle<mirror::ObjectArray<mirror::DexCache>> dex_caches(
      hs.NewHandle(dex_caches_object->AsObjectArray<mirror::DexCache>()));
  Handle<mirror::ObjectArray<mirror::Class>> class_roots(hs.NewHandle(
      header.GetImageRoot(ImageHeader::kClassRoots)->AsObjectArray<mirror::Class>()));
  const OatFile* oat_file = space->GetOatFile();
  std::unordered_set<mirror::ClassLoader*> image_class_loaders;
  // Check that the image is what we are expecting.
  if (image_pointer_size_ != space->GetImageHeader().GetPointerSize()) {
    *error_msg = StringPrintf("Application image pointer size does not match runtime: %zu vs %zu",
                              static_cast<size_t>(space->GetImageHeader().GetPointerSize()),
                              image_pointer_size_);
    return false;
  }
  DCHECK(class_roots.Get() != nullptr);
  if (class_roots->GetLength() != static_cast<int32_t>(kClassRootsMax)) {
    *error_msg = StringPrintf("Expected %d class roots but got %d",
                              class_roots->GetLength(),
                              static_cast<int32_t>(kClassRootsMax));
    return false;
  }
  // Check against existing class roots to make sure they match the ones in the boot image.
  for (size_t i = 0; i < kClassRootsMax; i++) {
    if (class_roots->Get(i) != GetClassRoot(static_cast<ClassRoot>(i))) {
      *error_msg = "App image class roots must have pointer equality with runtime ones.";
      return false;
    }
  }
  if (oat_file->GetOatHeader().GetDexFileCount() !=
      static_cast<uint32_t>(dex_caches->GetLength())) {
    *error_msg = "Dex cache count and dex file count mismatch while trying to initialize from "
                 "image";
    return false;
  }

  std::unique_ptr<ExactProfileFile> exact_profiling(
      ExactProfiler::GetProfileFile(*oat_file, true, true));
  Runtime::DexProfilersMap& dex_profile_map = Runtime::Current()->GetDexProfilers();

  StackHandleScope<1> hs2(self);
  MutableHandle<mirror::DexCache> h_dex_cache(hs2.NewHandle<mirror::DexCache>(nullptr));
  for (int32_t i = 0; i < dex_caches->GetLength(); i++) {
    h_dex_cache.Assign(dex_caches->Get(i));
    std::string dex_file_location(h_dex_cache->GetLocation()->ToModifiedUtf8());
    // TODO: Only store qualified paths.
    // If non qualified, qualify it.
    if (dex_file_location.find('/') == std::string::npos) {
      std::string dex_location_path = dex_location;
      const size_t pos = dex_location_path.find_last_of('/');
      CHECK_NE(pos, std::string::npos);
      dex_location_path = dex_location_path.substr(0, pos + 1);  // Keep trailing '/'
      dex_file_location = dex_location_path + dex_file_location;
    }
    std::unique_ptr<const DexFile> dex_file = OpenOatDexFile(oat_file,
                                                             dex_file_location.c_str(),
                                                             error_msg);
    if (dex_file == nullptr) {
      return false;
    }

    if (exact_profiling.get() != nullptr && exact_profiling->generating_profile) {
      // Allocate the counters for the dex file.
      MutexLock mu_prof(self, *Locks::profiler_lock_);
      ExactProfiler::AllocateProfileCounters(exact_profiling.get(), i, dex_file_location,
                                             dex_file.get(),
                                             h_dex_cache->GetResolvedMethods(),
                                             h_dex_cache->NumResolvedMethods());
      // Remember this mapping for later.
      dex_profile_map.Put(dex_file.get(), Runtime::ProfilerIndexPair(exact_profiling.get(), i));
    }

    if (app_image) {
      // The current dex file field is bogus, overwrite it so that we can get the dex file in the
      // loop below.
      h_dex_cache->SetDexFile(dex_file.get());
      // Check that each class loader resolved the same way.
      // TODO: Store image class loaders as image roots.
      GcRoot<mirror::Class>* const types = h_dex_cache->GetResolvedTypes();
      for (int32_t j = 0, num_types = h_dex_cache->NumResolvedTypes(); j < num_types; j++) {
        mirror::Class* klass = types[j].Read();
        if (klass != nullptr) {
          DCHECK_NE(klass->GetStatus(), mirror::Class::kStatusError);
          mirror::ClassLoader* image_class_loader = klass->GetClassLoader();
          image_class_loaders.insert(image_class_loader);
        }
      }
    } else {
      if (kSanityCheckObjects) {
        SanityCheckArtMethodPointerArray(h_dex_cache->GetResolvedMethods(),
                                         h_dex_cache->NumResolvedMethods(),
                                         image_pointer_size_,
                                         heap->GetBootImageSpaces());
      }
      // Register dex files, keep track of existing ones that are conflicts.
      AppendToBootClassPath(*dex_file.get(), h_dex_cache);
    }
    out_dex_files->push_back(std::move(dex_file));
  }

  if (exact_profiling.get() != nullptr) {
    // Remember the mapping for later use.
    Runtime::Current()->GetProfilers().Put(oat_file, exact_profiling.release());
  }

  if (app_image) {
    ScopedObjectAccessUnchecked soa(Thread::Current());
    // Check that the class loader resolves the same way as the ones in the image.
    // Image class loader [A][B][C][image dex files]
    // Class loader = [???][dex_elements][image dex files]
    // Need to ensure that [???][dex_elements] == [A][B][C].
    // For each class loader, PathClassLoader, the laoder checks the parent first. Also the logic
    // for PathClassLoader does this by looping through the array of dex files. To ensure they
    // resolve the same way, simply flatten the hierarchy in the way the resolution order would be,
    // and check that the dex file names are the same.
    for (mirror::ClassLoader* image_class_loader : image_class_loaders) {
      if (IsBootClassLoader(soa, image_class_loader)) {
        // The dex cache can reference types from the boot class loader.
        continue;
      }
      std::list<mirror::String*> image_dex_file_names;
      std::string temp_error_msg;
      if (!FlattenPathClassLoader(image_class_loader, &image_dex_file_names, &temp_error_msg)) {
        *error_msg = StringPrintf("Failed to flatten image class loader hierarchy '%s'",
                                  temp_error_msg.c_str());
        return false;
      }
      std::list<mirror::String*> loader_dex_file_names;
      if (!FlattenPathClassLoader(class_loader.Get(), &loader_dex_file_names, &temp_error_msg)) {
        *error_msg = StringPrintf("Failed to flatten class loader hierarchy '%s'",
                                  temp_error_msg.c_str());
        return false;
      }
      // Add the temporary dex path list elements at the end.
      auto* elements = soa.Decode<mirror::ObjectArray<mirror::Object>*>(dex_elements);
      for (size_t i = 0, num_elems = elements->GetLength(); i < num_elems; ++i) {
        mirror::Object* element = elements->GetWithoutChecks(i);
        if (element != nullptr) {
          // If we are somewhere in the middle of the array, there may be nulls at the end.
          loader_dex_file_names.push_back(GetDexPathListElementName(soa, element));
        }
      }
      // Ignore the number of image dex files since we are adding those to the class loader anyways.
      CHECK_GE(static_cast<size_t>(image_dex_file_names.size()),
               static_cast<size_t>(dex_caches->GetLength()));
      size_t image_count = image_dex_file_names.size() - dex_caches->GetLength();
      // Check that the dex file names match.
      bool equal = image_count == loader_dex_file_names.size();
      if (equal) {
        auto it1 = image_dex_file_names.begin();
        auto it2 = loader_dex_file_names.begin();
        for (size_t i = 0; equal && i < image_count; ++i, ++it1, ++it2) {
          equal = equal && (*it1)->Equals(*it2);
        }
      }
      if (!equal) {
        VLOG(image) << "Image dex files " << image_dex_file_names.size();
        for (mirror::String* name : image_dex_file_names) {
          VLOG(image) << name->ToModifiedUtf8();
        }
        VLOG(image) << "Loader dex files " << loader_dex_file_names.size();
        for (mirror::String* name : loader_dex_file_names) {
          VLOG(image) << name->ToModifiedUtf8();
        }
        *error_msg = "Rejecting application image due to class loader mismatch";
        // Ignore class loader mismatch for now since these would just use possibly incorrect
        // oat code anyways. The structural class check should be done in the parent.
      }
    }
  }

  if (kSanityCheckObjects) {
    for (int32_t i = 0; i < dex_caches->GetLength(); i++) {
      auto* dex_cache = dex_caches->Get(i);
      for (size_t j = 0; j < dex_cache->NumResolvedFields(); ++j) {
        auto* field = dex_cache->GetResolvedField(j, image_pointer_size_);
        if (field != nullptr) {
          CHECK(field->GetDeclaringClass()->GetClass() != nullptr);
        }
      }
    }
    if (!app_image) {
      heap->VisitObjects(SanityCheckObjectsCallback, nullptr);
    }
  }

  // Set entry point to interpreter if in InterpretOnly mode.
  if (!runtime->IsAotCompiler() && runtime->GetInstrumentation()->InterpretOnly()) {
    SetInterpreterEntrypointArtMethodVisitor visitor(image_pointer_size_);
    header.VisitPackedArtMethods(&visitor, space->Begin(), image_pointer_size_);
  }

  ClassTable* class_table = nullptr;
  {
    WriterMutexLock mu(self, *Locks::classlinker_classes_lock_);
    class_table = InsertClassTableForClassLoader(class_loader.Get());
  }
  // If we have a class table section, read it and use it for verification in
  // UpdateAppImageClassLoadersAndDexCaches.
  ClassTable::ClassSet temp_set;
  const ImageSection& class_table_section = header.GetImageSection(ImageHeader::kSectionClassTable);
  const bool added_class_table = class_table_section.Size() > 0u;
  if (added_class_table) {
    const uint64_t start_time2 = NanoTime();
    size_t read_count = 0;
    temp_set = ClassTable::ClassSet(space->Begin() + class_table_section.Offset(),
                                    /*make copy*/false,
                                    &read_count);
    if (!app_image) {
      dex_cache_boot_image_class_lookup_required_ = false;
    }
    VLOG(image) << "Adding class table classes took " << PrettyDuration(NanoTime() - start_time2);
  }
  if (app_image) {
    bool forward_dex_cache_arrays = false;
    if (!UpdateAppImageClassLoadersAndDexCaches(space,
                                                class_loader,
                                                dex_caches,
                                                added_class_table ? &temp_set : nullptr,
                                                /*out*/&forward_dex_cache_arrays,
                                                /*out*/error_msg)) {
      return false;
    }
    // Update class loader and resolved strings. If added_class_table is false, the resolved
    // strings were forwarded UpdateAppImageClassLoadersAndDexCaches.
    UpdateClassLoaderAndResolvedStringsVisitor visitor(space,
                                                       class_loader.Get(),
                                                       forward_dex_cache_arrays);
    if (added_class_table) {
      for (GcRoot<mirror::Class>& root : temp_set) {
        visitor(root.Read());
      }
    }
    // forward_dex_cache_arrays is true iff we copied all of the dex cache arrays into the .bss.
    // In this case, madvise away the dex cache arrays section of the image to reduce RAM usage and
    // mark as PROT_NONE to catch any invalid accesses.
    if (forward_dex_cache_arrays) {
      const ImageSection& dex_cache_section = header.GetImageSection(
          ImageHeader::kSectionDexCacheArrays);
      uint8_t* section_begin = AlignUp(space->Begin() + dex_cache_section.Offset(), kPageSize);
      uint8_t* section_end = AlignDown(space->Begin() + dex_cache_section.End(), kPageSize);
      if (section_begin < section_end) {
        madvise(section_begin, section_end - section_begin, MADV_DONTNEED);
        mprotect(section_begin, section_end - section_begin, PROT_NONE);
        VLOG(image) << "Released and protected dex cache array image section from "
                    << reinterpret_cast<const void*>(section_begin) << "-"
                    << reinterpret_cast<const void*>(section_end);
      }
    }
  }
  if (added_class_table) {
    WriterMutexLock mu(self, *Locks::classlinker_classes_lock_);
    class_table->AddClassSet(std::move(temp_set));
  }
  if (kIsDebugBuild && app_image) {
    // This verification needs to happen after the classes have been added to the class loader.
    // Since it ensures classes are in the class table.
    VerifyClassInTableArtMethodVisitor visitor2(class_table);
    header.VisitPackedArtMethods(&visitor2, space->Begin(), sizeof(void*));
  }

  VLOG(class_linker) << "Adding image space took " << PrettyDuration(NanoTime() - start_time);
  return true;
}

bool ClassLinker::ClassInClassTable(mirror::Class* klass) {
  ClassTable* const class_table = ClassTableForClassLoader(klass->GetClassLoader());
  return class_table != nullptr && class_table->Contains(klass);
}

void ClassLinker::VisitClassRoots(RootVisitor* visitor, VisitRootFlags flags) {
  // Acquire tracing_enabled before locking class linker lock to prevent lock order violation. Since
  // enabling tracing requires the mutator lock, there are no race conditions here.
  const bool tracing_enabled = Trace::IsTracingEnabled();
  Thread* const self = Thread::Current();
  WriterMutexLock mu(self, *Locks::classlinker_classes_lock_);
  BufferedRootVisitor<kDefaultBufferedRootCount> buffered_visitor(
      visitor, RootInfo(kRootStickyClass));
  if ((flags & kVisitRootFlagAllRoots) != 0) {
    // Argument for how root visiting deals with ArtField and ArtMethod roots.
    // There is 3 GC cases to handle:
    // Non moving concurrent:
    // This case is easy to handle since the reference members of ArtMethod and ArtFields are held
    // live by the class and class roots.
    //
    // Moving non-concurrent:
    // This case needs to call visit VisitNativeRoots in case the classes or dex cache arrays move.
    // To prevent missing roots, this case needs to ensure that there is no
    // suspend points between the point which we allocate ArtMethod arrays and place them in a
    // class which is in the class table.
    //
    // Moving concurrent:
    // Need to make sure to not copy ArtMethods without doing read barriers since the roots are
    // marked concurrently and we don't hold the classlinker_classes_lock_ when we do the copy.
    boot_class_table_.VisitRoots(buffered_visitor);

    // If tracing is enabled, then mark all the class loaders to prevent unloading.
    if (tracing_enabled) {
      for (const ClassLoaderData& data : class_loaders_) {
        GcRoot<mirror::Object> root(GcRoot<mirror::Object>(self->DecodeJObject(data.weak_root)));
        root.VisitRoot(visitor, RootInfo(kRootVMInternal));
      }
    }
  } else if ((flags & kVisitRootFlagNewRoots) != 0) {
    for (auto& root : new_class_roots_) {
      mirror::Class* old_ref = root.Read<kWithoutReadBarrier>();
      root.VisitRoot(visitor, RootInfo(kRootStickyClass));
      mirror::Class* new_ref = root.Read<kWithoutReadBarrier>();
      // Concurrent moving GC marked new roots through the to-space invariant.
      CHECK_EQ(new_ref, old_ref);
    }
  }
  buffered_visitor.Flush();  // Flush before clearing new_class_roots_.
  if ((flags & kVisitRootFlagClearRootLog) != 0) {
    new_class_roots_.clear();
  }
  if ((flags & kVisitRootFlagStartLoggingNewRoots) != 0) {
    log_new_class_table_roots_ = true;
  } else if ((flags & kVisitRootFlagStopLoggingNewRoots) != 0) {
    log_new_class_table_roots_ = false;
  }
  // We deliberately ignore the class roots in the image since we
  // handle image roots by using the MS/CMS rescanning of dirty cards.
}

// Keep in sync with InitCallback. Anything we visit, we need to
// reinit references to when reinitializing a ClassLinker from a
// mapped image.
void ClassLinker::VisitRoots(RootVisitor* visitor, VisitRootFlags flags) {
  class_roots_.VisitRootIfNonNull(visitor, RootInfo(kRootVMInternal));
  VisitClassRoots(visitor, flags);
  array_iftable_.VisitRootIfNonNull(visitor, RootInfo(kRootVMInternal));
  // Instead of visiting the find_array_class_cache_ drop it so that it doesn't prevent class
  // unloading if we are marking roots.
  DropFindArrayClassCache();
}

class VisitClassLoaderClassesVisitor : public ClassLoaderVisitor {
 public:
  explicit VisitClassLoaderClassesVisitor(ClassVisitor* visitor)
      : visitor_(visitor),
        done_(false) {}

  void Visit(mirror::ClassLoader* class_loader)
      SHARED_REQUIRES(Locks::classlinker_classes_lock_, Locks::mutator_lock_) OVERRIDE {
    ClassTable* const class_table = class_loader->GetClassTable();
    if (!done_ && class_table != nullptr && !class_table->Visit(*visitor_)) {
      // If the visitor ClassTable returns false it means that we don't need to continue.
      done_ = true;
    }
  }

 private:
  ClassVisitor* const visitor_;
  // If done is true then we don't need to do any more visiting.
  bool done_;
};

void ClassLinker::VisitClassesInternal(ClassVisitor* visitor) {
  if (boot_class_table_.Visit(*visitor)) {
    VisitClassLoaderClassesVisitor loader_visitor(visitor);
    VisitClassLoaders(&loader_visitor);
  }
}

void ClassLinker::VisitClasses(ClassVisitor* visitor) {
  if (dex_cache_boot_image_class_lookup_required_) {
    AddBootImageClassesToClassTable();
  }
  Thread* const self = Thread::Current();
  ReaderMutexLock mu(self, *Locks::classlinker_classes_lock_);
  // Not safe to have thread suspension when we are holding a lock.
  if (self != nullptr) {
    ScopedAssertNoThreadSuspension nts(self, __FUNCTION__);
    VisitClassesInternal(visitor);
  } else {
    VisitClassesInternal(visitor);
  }
}

class GetClassesInToVector : public ClassVisitor {
 public:
  bool operator()(mirror::Class* klass) OVERRIDE {
    classes_.push_back(klass);
    return true;
  }
  std::vector<mirror::Class*> classes_;
};

class GetClassInToObjectArray : public ClassVisitor {
 public:
  explicit GetClassInToObjectArray(mirror::ObjectArray<mirror::Class>* arr)
      : arr_(arr), index_(0) {}

  bool operator()(mirror::Class* klass) OVERRIDE SHARED_REQUIRES(Locks::mutator_lock_) {
    ++index_;
    if (index_ <= arr_->GetLength()) {
      arr_->Set(index_ - 1, klass);
      return true;
    }
    return false;
  }

  bool Succeeded() const SHARED_REQUIRES(Locks::mutator_lock_) {
    return index_ <= arr_->GetLength();
  }

 private:
  mirror::ObjectArray<mirror::Class>* const arr_;
  int32_t index_;
};

void ClassLinker::VisitClassesWithoutClassesLock(ClassVisitor* visitor) {
  // TODO: it may be possible to avoid secondary storage if we iterate over dex caches. The problem
  // is avoiding duplicates.
  Thread* const self = Thread::Current();
  if (!kMovingClasses) {
    GetClassesInToVector accumulator;
    VisitClasses(&accumulator);
    for (mirror::Class* klass : accumulator.classes_) {
      if (!visitor->operator()(klass)) {
        return;
      }
    }
  } else {
    StackHandleScope<1> hs(self);
    auto classes = hs.NewHandle<mirror::ObjectArray<mirror::Class>>(nullptr);
    // We size the array assuming classes won't be added to the class table during the visit.
    // If this assumption fails we iterate again.
    while (true) {
      size_t class_table_size;
      {
        ReaderMutexLock mu(self, *Locks::classlinker_classes_lock_);
        // Add 100 in case new classes get loaded when we are filling in the object array.
        class_table_size = NumZygoteClasses() + NumNonZygoteClasses() + 100;
      }
      mirror::Class* class_type = mirror::Class::GetJavaLangClass();
      mirror::Class* array_of_class = FindArrayClass(self, &class_type);
      classes.Assign(
          mirror::ObjectArray<mirror::Class>::Alloc(self, array_of_class, class_table_size));
      CHECK(classes.Get() != nullptr);  // OOME.
      GetClassInToObjectArray accumulator(classes.Get());
      VisitClasses(&accumulator);
      if (accumulator.Succeeded()) {
        break;
      }
    }
    for (int32_t i = 0; i < classes->GetLength(); ++i) {
      // If the class table shrank during creation of the clases array we expect null elements. If
      // the class table grew then the loop repeats. If classes are created after the loop has
      // finished then we don't visit.
      mirror::Class* klass = classes->Get(i);
      if (klass != nullptr && !visitor->operator()(klass)) {
        return;
      }
    }
  }
}

ClassLinker::~ClassLinker() {
  mirror::Class::ResetClass();
  mirror::Constructor::ResetClass();
  mirror::Field::ResetClass();
  mirror::Method::ResetClass();
  mirror::Reference::ResetClass();
  mirror::StackTraceElement::ResetClass();
  mirror::String::ResetClass();
  mirror::Throwable::ResetClass();
  mirror::BooleanArray::ResetArrayClass();
  mirror::ByteArray::ResetArrayClass();
  mirror::CharArray::ResetArrayClass();
  mirror::Constructor::ResetArrayClass();
  mirror::DoubleArray::ResetArrayClass();
  mirror::Field::ResetArrayClass();
  mirror::FloatArray::ResetArrayClass();
  mirror::Method::ResetArrayClass();
  mirror::IntArray::ResetArrayClass();
  mirror::LongArray::ResetArrayClass();
  mirror::ShortArray::ResetArrayClass();
  Thread* const self = Thread::Current();
  for (const ClassLoaderData& data : class_loaders_) {
    DeleteClassLoader(self, data);
  }
  class_loaders_.clear();
}

void ClassLinker::DeleteClassLoader(Thread* self, const ClassLoaderData& data) {
  Runtime* const runtime = Runtime::Current();
  JavaVMExt* const vm = runtime->GetJavaVM();
  vm->DeleteWeakGlobalRef(self, data.weak_root);
  // Notify the JIT that we need to remove the methods and/or profiling info.
  if (runtime->GetJit() != nullptr) {
    jit::JitCodeCache* code_cache = runtime->GetJit()->GetCodeCache();
    if (code_cache != nullptr) {
      code_cache->RemoveMethodsIn(self, *data.allocator);
    }
  }
  delete data.allocator;
  delete data.class_table;
}

mirror::PointerArray* ClassLinker::AllocPointerArray(Thread* self, size_t length) {
  return down_cast<mirror::PointerArray*>(image_pointer_size_ == 8u ?
      static_cast<mirror::Array*>(mirror::LongArray::Alloc(self, length)) :
      static_cast<mirror::Array*>(mirror::IntArray::Alloc(self, length)));
}

mirror::DexCache* ClassLinker::AllocDexCache(Thread* self,
                                             const DexFile& dex_file,
                                             LinearAlloc* linear_alloc) {
  StackHandleScope<6> hs(self);
  auto dex_cache(hs.NewHandle(down_cast<mirror::DexCache*>(
      GetClassRoot(kJavaLangDexCache)->AllocObject(self))));
  if (dex_cache.Get() == nullptr) {
    self->AssertPendingOOMException();
    return nullptr;
  }
  auto location(hs.NewHandle(intern_table_->InternStrong(dex_file.GetLocation().c_str())));
  if (location.Get() == nullptr) {
    self->AssertPendingOOMException();
    return nullptr;
  }
  DexCacheArraysLayout layout(image_pointer_size_, &dex_file);
  uint8_t* raw_arrays = nullptr;
  if (dex_file.GetOatDexFile() != nullptr &&
      dex_file.GetOatDexFile()->GetDexCacheArrays() != nullptr) {
    raw_arrays = dex_file.GetOatDexFile()->GetDexCacheArrays();
  } else if (dex_file.NumStringIds() != 0u || dex_file.NumTypeIds() != 0u ||
      dex_file.NumMethodIds() != 0u || dex_file.NumFieldIds() != 0u) {
    // NOTE: We "leak" the raw_arrays because we never destroy the dex cache.
    DCHECK(image_pointer_size_ == 4u || image_pointer_size_ == 8u);
    // Zero-initialized.
    raw_arrays = reinterpret_cast<uint8_t*>(linear_alloc->Alloc(self, layout.Size()));
  }
  GcRoot<mirror::String>* strings = (dex_file.NumStringIds() == 0u) ? nullptr :
      reinterpret_cast<GcRoot<mirror::String>*>(raw_arrays + layout.StringsOffset());
  GcRoot<mirror::Class>* types = (dex_file.NumTypeIds() == 0u) ? nullptr :
      reinterpret_cast<GcRoot<mirror::Class>*>(raw_arrays + layout.TypesOffset());
  ArtMethod** methods = (dex_file.NumMethodIds() == 0u) ? nullptr :
      reinterpret_cast<ArtMethod**>(raw_arrays + layout.MethodsOffset());
  ArtField** fields = (dex_file.NumFieldIds() == 0u) ? nullptr :
      reinterpret_cast<ArtField**>(raw_arrays + layout.FieldsOffset());
  if (kIsDebugBuild) {
    // Sanity check to make sure all the dex cache arrays are empty. b/28992179
    for (size_t i = 0; i < dex_file.NumStringIds(); ++i) {
      CHECK(strings[i].Read<kWithoutReadBarrier>() == nullptr);
    }
    for (size_t i = 0; i < dex_file.NumTypeIds(); ++i) {
      CHECK(types[i].Read<kWithoutReadBarrier>() == nullptr);
    }
    for (size_t i = 0; i < dex_file.NumMethodIds(); ++i) {
      CHECK(mirror::DexCache::GetElementPtrSize(methods, i, image_pointer_size_) == nullptr);
    }
    for (size_t i = 0; i < dex_file.NumFieldIds(); ++i) {
      CHECK(mirror::DexCache::GetElementPtrSize(fields, i, image_pointer_size_) == nullptr);
    }
  }
  dex_cache->Init(&dex_file,
                  location.Get(),
                  strings,
                  dex_file.NumStringIds(),
                  types,
                  dex_file.NumTypeIds(),
                  methods,
                  dex_file.NumMethodIds(),
                  fields,
                  dex_file.NumFieldIds(),
                  image_pointer_size_);
  return dex_cache.Get();
}

mirror::Class* ClassLinker::AllocClass(Thread* self, mirror::Class* java_lang_Class,
                                       uint32_t class_size) {
  DCHECK_GE(class_size, sizeof(mirror::Class));
  gc::Heap* heap = Runtime::Current()->GetHeap();
  mirror::Class::InitializeClassVisitor visitor(class_size);
  mirror::Object* k = kMovingClasses ?
      heap->AllocObject<true>(self, java_lang_Class, class_size, visitor) :
      heap->AllocNonMovableObject<true>(self, java_lang_Class, class_size, visitor);
  if (UNLIKELY(k == nullptr)) {
    self->AssertPendingOOMException();
    return nullptr;
  }
  return k->AsClass();
}

mirror::Class* ClassLinker::AllocClass(Thread* self, uint32_t class_size) {
  return AllocClass(self, GetClassRoot(kJavaLangClass), class_size);
}

mirror::ObjectArray<mirror::StackTraceElement>* ClassLinker::AllocStackTraceElementArray(
    Thread* self,
    size_t length) {
  return mirror::ObjectArray<mirror::StackTraceElement>::Alloc(
      self, GetClassRoot(kJavaLangStackTraceElementArrayClass), length);
}

mirror::Class* ClassLinker::EnsureResolved(Thread* self,
                                           const char* descriptor,
                                           mirror::Class* klass) {
  DCHECK(klass != nullptr);

  // For temporary classes we must wait for them to be retired.
  if (init_done_ && klass->IsTemp()) {
    CHECK(!klass->IsResolved());
    if (klass->IsErroneous()) {
      ThrowEarlierClassFailure(klass);
      return nullptr;
    }
    StackHandleScope<1> hs(self);
    Handle<mirror::Class> h_class(hs.NewHandle(klass));
    ObjectLock<mirror::Class> lock(self, h_class);
    // Loop and wait for the resolving thread to retire this class.
    while (!h_class->IsRetired() && !h_class->IsErroneous()) {
      lock.WaitIgnoringInterrupts();
    }
    if (h_class->IsErroneous()) {
      ThrowEarlierClassFailure(h_class.Get());
      return nullptr;
    }
    CHECK(h_class->IsRetired());
    // Get the updated class from class table.
    klass = LookupClass(self, descriptor, ComputeModifiedUtf8Hash(descriptor),
                        h_class.Get()->GetClassLoader());
  }

  // Wait for the class if it has not already been linked.
  if (!klass->IsResolved() && !klass->IsErroneous()) {
    StackHandleScope<1> hs(self);
    HandleWrapper<mirror::Class> h_class(hs.NewHandleWrapper(&klass));
    ObjectLock<mirror::Class> lock(self, h_class);
    // Check for circular dependencies between classes.
    if (!h_class->IsResolved() && h_class->GetClinitThreadId() == self->GetTid()) {
      ThrowClassCircularityError(h_class.Get());
      mirror::Class::SetStatus(h_class, mirror::Class::kStatusError, self);
      return nullptr;
    }
    // Wait for the pending initialization to complete.
    while (!h_class->IsResolved() && !h_class->IsErroneous()) {
      lock.WaitIgnoringInterrupts();
    }
  }

  if (klass->IsErroneous()) {
    ThrowEarlierClassFailure(klass);
    return nullptr;
  }
  // Return the loaded class.  No exceptions should be pending.
  CHECK(klass->IsResolved()) << PrettyClass(klass);
  self->AssertNoPendingException();
  return klass;
}

typedef std::pair<const DexFile*, const DexFile::ClassDef*> ClassPathEntry;

// Search a collection of DexFiles for a descriptor
ClassPathEntry FindInClassPath(const char* descriptor,
                               size_t hash, const std::vector<const DexFile*>& class_path) {
  for (const DexFile* dex_file : class_path) {
    const DexFile::ClassDef* dex_class_def = dex_file->FindClassDef(descriptor, hash);
    if (dex_class_def != nullptr) {
      return ClassPathEntry(dex_file, dex_class_def);
    }
  }
  return ClassPathEntry(nullptr, nullptr);
}

bool ClassLinker::FindClassInPathClassLoader(ScopedObjectAccessAlreadyRunnable& soa,
                                             Thread* self,
                                             const char* descriptor,
                                             size_t hash,
                                             Handle<mirror::ClassLoader> class_loader,
                                             mirror::Class** result) {
  // Termination case: boot class-loader.
  if (IsBootClassLoader(soa, class_loader.Get())) {
    // The boot class loader, search the boot class path.
    ClassPathEntry pair = FindInClassPath(descriptor, hash, boot_class_path_);
    if (pair.second != nullptr) {
      mirror::Class* klass = LookupClass(self, descriptor, hash, nullptr);
      if (klass != nullptr) {
        *result = EnsureResolved(self, descriptor, klass);
      } else {
        *result = DefineClass(self,
                              descriptor,
                              hash,
                              ScopedNullHandle<mirror::ClassLoader>(),
                              *pair.first,
                              *pair.second);
      }
      if (*result == nullptr) {
        CHECK(self->IsExceptionPending()) << descriptor;
        self->ClearException();
      }
    } else {
      *result = nullptr;
    }
    return true;
  }

  // Unsupported class-loader?
  if (class_loader->GetClass() !=
      soa.Decode<mirror::Class*>(WellKnownClasses::dalvik_system_PathClassLoader)) {
    *result = nullptr;
    return false;
  }

  // Handles as RegisterDexFile may allocate dex caches (and cause thread suspension).
  StackHandleScope<4> hs(self);
  Handle<mirror::ClassLoader> h_parent(hs.NewHandle(class_loader->GetParent()));
  bool recursive_result = FindClassInPathClassLoader(soa, self, descriptor, hash, h_parent, result);

  if (!recursive_result) {
    // Something wrong up the chain.
    return false;
  }

  if (*result != nullptr) {
    // Found the class up the chain.
    return true;
  }

  // Handle this step.
  // Handle as if this is the child PathClassLoader.
  // The class loader is a PathClassLoader which inherits from BaseDexClassLoader.
  // We need to get the DexPathList and loop through it.
  ArtField* const cookie_field = soa.DecodeField(WellKnownClasses::dalvik_system_DexFile_cookie);
  ArtField* const dex_file_field =
      soa.DecodeField(WellKnownClasses::dalvik_system_DexPathList__Element_dexFile);
  mirror::Object* dex_path_list =
      soa.DecodeField(WellKnownClasses::dalvik_system_PathClassLoader_pathList)->
      GetObject(class_loader.Get());
  if (dex_path_list != nullptr && dex_file_field != nullptr && cookie_field != nullptr) {
    // DexPathList has an array dexElements of Elements[] which each contain a dex file.
    mirror::Object* dex_elements_obj =
        soa.DecodeField(WellKnownClasses::dalvik_system_DexPathList_dexElements)->
        GetObject(dex_path_list);
    // Loop through each dalvik.system.DexPathList$Element's dalvik.system.DexFile and look
    // at the mCookie which is a DexFile vector.
    if (dex_elements_obj != nullptr) {
      Handle<mirror::ObjectArray<mirror::Object>> dex_elements =
          hs.NewHandle(dex_elements_obj->AsObjectArray<mirror::Object>());
      for (int32_t i = 0; i < dex_elements->GetLength(); ++i) {
        mirror::Object* element = dex_elements->GetWithoutChecks(i);
        if (element == nullptr) {
          // Should never happen, fall back to java code to throw a NPE.
          break;
        }
        mirror::Object* dex_file = dex_file_field->GetObject(element);
        if (dex_file != nullptr) {
          mirror::LongArray* long_array = cookie_field->GetObject(dex_file)->AsLongArray();
          if (long_array == nullptr) {
            // This should never happen so log a warning.
            LOG(WARNING) << "Null DexFile::mCookie for " << descriptor;
            break;
          }
          int32_t long_array_size = long_array->GetLength();
          // First element is the oat file.
          for (int32_t j = kDexFileIndexStart; j < long_array_size; ++j) {
            const DexFile* cp_dex_file = reinterpret_cast<const DexFile*>(static_cast<uintptr_t>(
                long_array->GetWithoutChecks(j)));
            const DexFile::ClassDef* dex_class_def = cp_dex_file->FindClassDef(descriptor, hash);
            if (dex_class_def != nullptr) {
              mirror::Class* klass = DefineClass(self,
                                                 descriptor,
                                                 hash,
                                                 class_loader,
                                                 *cp_dex_file,
                                                 *dex_class_def);
              if (klass == nullptr) {
                CHECK(self->IsExceptionPending()) << descriptor;
                self->ClearException();
                // TODO: Is it really right to break here, and not check the other dex files?
                return true;
              }
              *result = klass;
              return true;
            }
          }
        }
      }
    }
    self->AssertNoPendingException();
  }

  // Result is still null from the parent call, no need to set it again...
  return true;
}

mirror::Class* ClassLinker::FindClass(Thread* self,
                                      const char* descriptor,
                                      Handle<mirror::ClassLoader> class_loader) {
  DCHECK_NE(*descriptor, '\0') << "descriptor is empty string";
  DCHECK(self != nullptr);
  self->AssertNoPendingException();
  if (descriptor[1] == '\0') {
    // only the descriptors of primitive types should be 1 character long, also avoid class lookup
    // for primitive classes that aren't backed by dex files.
    return FindPrimitiveClass(descriptor[0]);
  }
  const size_t hash = ComputeModifiedUtf8Hash(descriptor);
  // Find the class in the loaded classes table.
  mirror::Class* klass = LookupClass(self, descriptor, hash, class_loader.Get());
  if (klass != nullptr) {
    return EnsureResolved(self, descriptor, klass);
  }
  // Class is not yet loaded.
  if (descriptor[0] == '[') {
    return CreateArrayClass(self, descriptor, hash, class_loader);
  } else if (class_loader.Get() == nullptr) {
    // The boot class loader, search the boot class path.
    ClassPathEntry pair = FindInClassPath(descriptor, hash, boot_class_path_);
    if (pair.second != nullptr) {
      return DefineClass(self,
                         descriptor,
                         hash,
                         ScopedNullHandle<mirror::ClassLoader>(),
                         *pair.first,
                         *pair.second);
    } else {
      // The boot class loader is searched ahead of the application class loader, failures are
      // expected and will be wrapped in a ClassNotFoundException. Use the pre-allocated error to
      // trigger the chaining with a proper stack trace.
      mirror::Throwable* pre_allocated = Runtime::Current()->GetPreAllocatedNoClassDefFoundError();
      self->SetException(pre_allocated);
      return nullptr;
    }
  } else {
    ScopedObjectAccessUnchecked soa(self);
    mirror::Class* cp_klass;
    if (FindClassInPathClassLoader(soa, self, descriptor, hash, class_loader, &cp_klass)) {
      // The chain was understood. So the value in cp_klass is either the class we were looking
      // for, or not found.
      if (cp_klass != nullptr) {
        return cp_klass;
      }
      // TODO: We handle the boot classpath loader in FindClassInPathClassLoader. Try to unify this
      //       and the branch above. TODO: throw the right exception here.

      // We'll let the Java-side rediscover all this and throw the exception with the right stack
      // trace.
    }

    if (Runtime::Current()->IsAotCompiler()) {
      // Oops, compile-time, can't run actual class-loader code.
      mirror::Throwable* pre_allocated = Runtime::Current()->GetPreAllocatedNoClassDefFoundError();
      self->SetException(pre_allocated);
      return nullptr;
    }

    ScopedLocalRef<jobject> class_loader_object(soa.Env(),
                                                soa.AddLocalReference<jobject>(class_loader.Get()));
    std::string class_name_string(DescriptorToDot(descriptor));
    ScopedLocalRef<jobject> result(soa.Env(), nullptr);
    {
      ScopedThreadStateChange tsc(self, kNative);
      ScopedLocalRef<jobject> class_name_object(soa.Env(),
                                                soa.Env()->NewStringUTF(class_name_string.c_str()));
      if (class_name_object.get() == nullptr) {
        DCHECK(self->IsExceptionPending());  // OOME.
        return nullptr;
      }
      CHECK(class_loader_object.get() != nullptr);
      result.reset(soa.Env()->CallObjectMethod(class_loader_object.get(),
                                               WellKnownClasses::java_lang_ClassLoader_loadClass,
                                               class_name_object.get()));
    }
    if (self->IsExceptionPending()) {
      // If the ClassLoader threw, pass that exception up.
      return nullptr;
    } else if (result.get() == nullptr) {
      // broken loader - throw NPE to be compatible with Dalvik
      ThrowNullPointerException(StringPrintf("ClassLoader.loadClass returned null for %s",
                                             class_name_string.c_str()).c_str());
      return nullptr;
    } else {
      // success, return mirror::Class*
      return soa.Decode<mirror::Class*>(result.get());
    }
  }
  UNREACHABLE();
}

mirror::Class* ClassLinker::DefineClass(Thread* self,
                                        const char* descriptor,
                                        size_t hash,
                                        Handle<mirror::ClassLoader> class_loader,
                                        const DexFile& dex_file,
                                        const DexFile::ClassDef& dex_class_def) {
  StackHandleScope<3> hs(self);
  auto klass = hs.NewHandle<mirror::Class>(nullptr);

  // Load the class from the dex file.
  if (UNLIKELY(!init_done_)) {
    // finish up init of hand crafted class_roots_
    if (strcmp(descriptor, "Ljava/lang/Object;") == 0) {
      klass.Assign(GetClassRoot(kJavaLangObject));
    } else if (strcmp(descriptor, "Ljava/lang/Class;") == 0) {
      klass.Assign(GetClassRoot(kJavaLangClass));
    } else if (strcmp(descriptor, "Ljava/lang/String;") == 0) {
      klass.Assign(GetClassRoot(kJavaLangString));
    } else if (strcmp(descriptor, "Ljava/lang/ref/Reference;") == 0) {
      klass.Assign(GetClassRoot(kJavaLangRefReference));
    } else if (strcmp(descriptor, "Ljava/lang/DexCache;") == 0) {
      klass.Assign(GetClassRoot(kJavaLangDexCache));
    }
  }

  if (klass.Get() == nullptr) {
    // Allocate a class with the status of not ready.
    // Interface object should get the right size here. Regular class will
    // figure out the right size later and be replaced with one of the right
    // size when the class becomes resolved.
    klass.Assign(AllocClass(self, SizeOfClassWithoutEmbeddedTables(dex_file, dex_class_def)));
  }
  if (UNLIKELY(klass.Get() == nullptr)) {
    self->AssertPendingOOMException();
    return nullptr;
  }
  mirror::DexCache* dex_cache = RegisterDexFile(dex_file, class_loader.Get());
  if (dex_cache == nullptr) {
    self->AssertPendingOOMException();
    return nullptr;
  }
  klass->SetDexCache(dex_cache);
  SetupClass(dex_file, dex_class_def, klass, class_loader.Get());

  // Mark the string class by setting its access flag.
  if (UNLIKELY(!init_done_)) {
    if (strcmp(descriptor, "Ljava/lang/String;") == 0) {
      klass->SetStringClass();
    }
  }

  ObjectLock<mirror::Class> lock(self, klass);
  klass->SetClinitThreadId(self->GetTid());

  // Add the newly loaded class to the loaded classes table.
  mirror::Class* existing = InsertClass(descriptor, klass.Get(), hash);
  if (existing != nullptr) {
    // We failed to insert because we raced with another thread. Calling EnsureResolved may cause
    // this thread to block.
    return EnsureResolved(self, descriptor, existing);
  }

  // Load the fields and other things after we are inserted in the table. This is so that we don't
  // end up allocating unfree-able linear alloc resources and then lose the race condition. The
  // other reason is that the field roots are only visited from the class table. So we need to be
  // inserted before we allocate / fill in these fields.
  LoadClass(self, dex_file, dex_class_def, klass);
  if (self->IsExceptionPending()) {
    VLOG(class_linker) << self->GetException()->Dump();
    // An exception occured during load, set status to erroneous while holding klass' lock in case
    // notification is necessary.
    if (!klass->IsErroneous()) {
      mirror::Class::SetStatus(klass, mirror::Class::kStatusError, self);
    }
    return nullptr;
  }

  // Finish loading (if necessary) by finding parents
  CHECK(!klass->IsLoaded());
  if (!LoadSuperAndInterfaces(klass, dex_file)) {
    // Loading failed.
    if (!klass->IsErroneous()) {
      mirror::Class::SetStatus(klass, mirror::Class::kStatusError, self);
    }
    return nullptr;
  }
  CHECK(klass->IsLoaded());
  // Link the class (if necessary)
  CHECK(!klass->IsResolved());
  // TODO: Use fast jobjects?
  auto interfaces = hs.NewHandle<mirror::ObjectArray<mirror::Class>>(nullptr);

  MutableHandle<mirror::Class> h_new_class = hs.NewHandle<mirror::Class>(nullptr);
  if (!LinkClass(self, descriptor, klass, interfaces, &h_new_class)) {
    // Linking failed.
    if (!klass->IsErroneous()) {
      mirror::Class::SetStatus(klass, mirror::Class::kStatusError, self);
    }
    return nullptr;
  }
  self->AssertNoPendingException();
  CHECK(h_new_class.Get() != nullptr) << descriptor;
  CHECK(h_new_class->IsResolved()) << descriptor;

  // Instrumentation may have updated entrypoints for all methods of all
  // classes. However it could not update methods of this class while we
  // were loading it. Now the class is resolved, we can update entrypoints
  // as required by instrumentation.
  if (Runtime::Current()->GetInstrumentation()->AreExitStubsInstalled()) {
    // We must be in the kRunnable state to prevent instrumentation from
    // suspending all threads to update entrypoints while we are doing it
    // for this class.
    DCHECK_EQ(self->GetState(), kRunnable);
    Runtime::Current()->GetInstrumentation()->InstallStubsForClass(h_new_class.Get());
  }

  /*
   * We send CLASS_PREPARE events to the debugger from here.  The
   * definition of "preparation" is creating the static fields for a
   * class and initializing them to the standard default values, but not
   * executing any code (that comes later, during "initialization").
   *
   * We did the static preparation in LinkClass.
   *
   * The class has been prepared and resolved but possibly not yet verified
   * at this point.
   */
  Dbg::PostClassPrepare(h_new_class.Get());

  // Notify native debugger of the new class and its layout.
  jit::Jit::NewTypeLoadedIfUsingJit(h_new_class.Get());

  return h_new_class.Get();
}

uint32_t ClassLinker::SizeOfClassWithoutEmbeddedTables(const DexFile& dex_file,
                                                       const DexFile::ClassDef& dex_class_def) {
  const uint8_t* class_data = dex_file.GetClassData(dex_class_def);
  size_t num_ref = 0;
  size_t num_8 = 0;
  size_t num_16 = 0;
  size_t num_32 = 0;
  size_t num_64 = 0;
  if (class_data != nullptr) {
    // We allow duplicate definitions of the same field in a class_data_item
    // but ignore the repeated indexes here, b/21868015.
    uint32_t last_field_idx = DexFile::kDexNoIndex;
    for (ClassDataItemIterator it(dex_file, class_data); it.HasNextStaticField(); it.Next()) {
      uint32_t field_idx = it.GetMemberIndex();
      // Ordering enforced by DexFileVerifier.
      DCHECK(last_field_idx == DexFile::kDexNoIndex || last_field_idx <= field_idx);
      if (UNLIKELY(field_idx == last_field_idx)) {
        continue;
      }
      last_field_idx = field_idx;
      const DexFile::FieldId& field_id = dex_file.GetFieldId(field_idx);
      const char* descriptor = dex_file.GetFieldTypeDescriptor(field_id);
      char c = descriptor[0];
      switch (c) {
        case 'L':
        case '[':
          num_ref++;
          break;
        case 'J':
        case 'D':
          num_64++;
          break;
        case 'I':
        case 'F':
          num_32++;
          break;
        case 'S':
        case 'C':
          num_16++;
          break;
        case 'B':
        case 'Z':
          num_8++;
          break;
        default:
          LOG(FATAL) << "Unknown descriptor: " << c;
          UNREACHABLE();
      }
    }
  }
  return mirror::Class::ComputeClassSize(false,
                                         0,
                                         num_8,
                                         num_16,
                                         num_32,
                                         num_64,
                                         num_ref,
                                         image_pointer_size_);
}

OatFile::OatClass ClassLinker::FindOatClass(const DexFile& dex_file,
                                            uint16_t class_def_idx,
                                            bool* found) {
  DCHECK_NE(class_def_idx, DexFile::kDexNoIndex16);
  const OatFile::OatDexFile* oat_dex_file = dex_file.GetOatDexFile();
  if (oat_dex_file == nullptr) {
    *found = false;
    return OatFile::OatClass::Invalid();
  }
  *found = true;
  return oat_dex_file->GetOatClass(class_def_idx);
}

static uint32_t GetOatMethodIndexFromMethodIndex(const DexFile& dex_file,
                                                 uint16_t class_def_idx,
                                                 uint32_t method_idx) {
  const DexFile::ClassDef& class_def = dex_file.GetClassDef(class_def_idx);
  const uint8_t* class_data = dex_file.GetClassData(class_def);
  CHECK(class_data != nullptr);
  ClassDataItemIterator it(dex_file, class_data);
  // Skip fields
  while (it.HasNextStaticField()) {
    it.Next();
  }
  while (it.HasNextInstanceField()) {
    it.Next();
  }
  // Process methods
  size_t class_def_method_index = 0;
  while (it.HasNextDirectMethod()) {
    if (it.GetMemberIndex() == method_idx) {
      return class_def_method_index;
    }
    class_def_method_index++;
    it.Next();
  }
  while (it.HasNextVirtualMethod()) {
    if (it.GetMemberIndex() == method_idx) {
      return class_def_method_index;
    }
    class_def_method_index++;
    it.Next();
  }
  DCHECK(!it.HasNext());
  LOG(FATAL) << "Failed to find method index " << method_idx << " in " << dex_file.GetLocation();
  UNREACHABLE();
}

const OatFile::OatMethod ClassLinker::FindOatMethodFor(ArtMethod* method, bool* found) {
  // Although we overwrite the trampoline of non-static methods, we may get here via the resolution
  // method for direct methods (or virtual methods made direct).
  mirror::Class* declaring_class = method->GetDeclaringClass();
  size_t oat_method_index;
  if (method->IsStatic() || method->IsDirect()) {
    // Simple case where the oat method index was stashed at load time.
    oat_method_index = method->GetMethodIndex();
  } else {
    // We're invoking a virtual method directly (thanks to sharpening), compute the oat_method_index
    // by search for its position in the declared virtual methods.
    oat_method_index = declaring_class->NumDirectMethods();
    bool found_virtual = false;
    for (ArtMethod& art_method : declaring_class->GetVirtualMethods(image_pointer_size_)) {
      // Check method index instead of identity in case of duplicate method definitions.
      if (method->GetDexMethodIndex() == art_method.GetDexMethodIndex()) {
        found_virtual = true;
        break;
      }
      oat_method_index++;
    }
    CHECK(found_virtual) << "Didn't find oat method index for virtual method: "
                         << PrettyMethod(method);
  }
  DCHECK_EQ(oat_method_index,
            GetOatMethodIndexFromMethodIndex(*declaring_class->GetDexCache()->GetDexFile(),
                                             method->GetDeclaringClass()->GetDexClassDefIndex(),
                                             method->GetDexMethodIndex()));
  OatFile::OatClass oat_class = FindOatClass(*declaring_class->GetDexCache()->GetDexFile(),
                                             declaring_class->GetDexClassDefIndex(),
                                             found);
  if (!(*found)) {
    return OatFile::OatMethod::Invalid();
  }
  return oat_class.GetOatMethod(oat_method_index);
}

// Special case to get oat code without overwriting a trampoline.
const void* ClassLinker::GetQuickOatCodeFor(ArtMethod* method) {
  CHECK(method->IsInvokable()) << PrettyMethod(method);
  if (method->IsProxyMethod()) {
    return GetQuickProxyInvokeHandler();
  }
  bool found;
  OatFile::OatMethod oat_method = FindOatMethodFor(method, &found);
  if (found) {
    auto* code = oat_method.GetQuickCode();
    if (code != nullptr) {
      return code;
    }
  }
  if (method->IsNative()) {
    // No code and native? Use generic trampoline.
    return GetQuickGenericJniStub();
  }
  return GetQuickToInterpreterBridge();
}

const void* ClassLinker::GetOatMethodQuickCodeFor(ArtMethod* method) {
  if (method->IsNative() || !method->IsInvokable() || method->IsProxyMethod()) {
    return nullptr;
  }
  bool found;
  OatFile::OatMethod oat_method = FindOatMethodFor(method, &found);
  if (found) {
    return oat_method.GetQuickCode();
  }
  return nullptr;
}

bool ClassLinker::ShouldUseInterpreterEntrypoint(ArtMethod* method, const void* quick_code) {
  if (UNLIKELY(method->IsNative() || method->IsProxyMethod())) {
    return false;
  }

  if (quick_code == nullptr) {
    return true;
  }

  Runtime* runtime = Runtime::Current();
  instrumentation::Instrumentation* instr = runtime->GetInstrumentation();
  if (instr->InterpretOnly()) {
    return true;
  }

  if (runtime->GetClassLinker()->IsQuickToInterpreterBridge(quick_code)) {
    // Doing this check avoids doing compiled/interpreter transitions.
    return true;
  }

  if (Dbg::IsForcedInterpreterNeededForCalling(Thread::Current(), method)) {
    // Force the use of interpreter when it is required by the debugger.
    return true;
  }

  if (runtime->IsNativeDebuggable()) {
    DCHECK(runtime->UseJitCompilation() && runtime->GetJit()->JitAtFirstUse());
    // If we are doing native debugging, ignore application's AOT code,
    // since we want to JIT it with extra stackmaps for native debugging.
    // On the other hand, keep all AOT code from the boot image, since the
    // blocking JIT would results in non-negligible performance impact.
    return !runtime->GetHeap()->IsInBootImageOatFile(quick_code);
  }

  if (Dbg::IsDebuggerActive()) {
    // Boot image classes may be AOT-compiled as non-debuggable.
    // This is not suitable for the Java debugger, so ignore the AOT code.
    return runtime->GetHeap()->IsInBootImageOatFile(quick_code);
  }

  return false;
}

void ClassLinker::FixupStaticTrampolines(mirror::Class* klass) {
  DCHECK(klass->IsInitialized()) << PrettyDescriptor(klass);
  if (klass->NumDirectMethods() == 0) {
    return;  // No direct methods => no static methods.
  }
  Runtime* runtime = Runtime::Current();
  if (!runtime->IsStarted()) {
    if (runtime->IsAotCompiler() || runtime->GetHeap()->HasBootImageSpace()) {
      return;  // OAT file unavailable.
    }
  }

  const DexFile& dex_file = klass->GetDexFile();
  const DexFile::ClassDef* dex_class_def = klass->GetClassDef();
  CHECK(dex_class_def != nullptr);
  const uint8_t* class_data = dex_file.GetClassData(*dex_class_def);
  // There should always be class data if there were direct methods.
  CHECK(class_data != nullptr) << PrettyDescriptor(klass);
  ClassDataItemIterator it(dex_file, class_data);
  // Skip fields
  while (it.HasNextStaticField()) {
    it.Next();
  }
  while (it.HasNextInstanceField()) {
    it.Next();
  }
  bool has_oat_class;
  OatFile::OatClass oat_class = FindOatClass(dex_file,
                                             klass->GetDexClassDefIndex(),
                                             &has_oat_class);
  // Link the code of methods skipped by LinkCode.
  for (size_t method_index = 0; it.HasNextDirectMethod(); ++method_index, it.Next()) {
    ArtMethod* method = klass->GetDirectMethod(method_index, image_pointer_size_);
    if (!method->IsStatic()) {
      // Only update static methods.
      continue;
    }
    const void* quick_code = nullptr;
    if (has_oat_class) {
      OatFile::OatMethod oat_method = oat_class.GetOatMethod(method_index);
      quick_code = oat_method.GetQuickCode();
    }
    // Check whether the method is native, in which case it's generic JNI.
    if (quick_code == nullptr && method->IsNative()) {
      quick_code = GetQuickGenericJniStub();
    } else if (ShouldUseInterpreterEntrypoint(method, quick_code)) {
      // Use interpreter entry point.
      quick_code = GetQuickToInterpreterBridge();
    }
    runtime->GetInstrumentation()->UpdateMethodsCode(method, quick_code);
  }
  // Ignore virtual methods on the iterator.
}

void ClassLinker::EnsureThrowsInvocationError(ArtMethod* method) {
  DCHECK(method != nullptr);
  DCHECK(!method->IsInvokable());
  method->SetEntryPointFromQuickCompiledCodePtrSize(quick_to_interpreter_bridge_trampoline_,
                                                    image_pointer_size_);
}

void ClassLinker::LinkCode(ArtMethod* method, const OatFile::OatClass* oat_class,
                           uint32_t class_def_method_index) {
  Runtime* const runtime = Runtime::Current();
  if (runtime->IsAotCompiler()) {
    // The following code only applies to a non-compiler runtime.
    return;
  }
  // Method shouldn't have already been linked.
  DCHECK(method->GetEntryPointFromQuickCompiledCode() == nullptr);
  if (oat_class != nullptr) {
    // Every kind of method should at least get an invoke stub from the oat_method.
    // non-abstract methods also get their code pointers.
    const OatFile::OatMethod oat_method = oat_class->GetOatMethod(class_def_method_index);
    oat_method.LinkMethod(method);
  }

  // Install entry point from interpreter.
  const void* quick_code = method->GetEntryPointFromQuickCompiledCode();
  bool enter_interpreter = ShouldUseInterpreterEntrypoint(method, quick_code);

  if (!method->IsInvokable()) {
    EnsureThrowsInvocationError(method);
    return;
  }

  if (method->IsStatic() && !method->IsConstructor()) {
    // For static methods excluding the class initializer, install the trampoline.
    // It will be replaced by the proper entry point by ClassLinker::FixupStaticTrampolines
    // after initializing class (see ClassLinker::InitializeClass method).
    method->SetEntryPointFromQuickCompiledCode(GetQuickResolutionStub());
  } else if (quick_code == nullptr && method->IsNative()) {
    method->SetEntryPointFromQuickCompiledCode(GetQuickGenericJniStub());
  } else if (enter_interpreter) {
    // Set entry point from compiled code if there's no code or in interpreter only mode.
    method->SetEntryPointFromQuickCompiledCode(GetQuickToInterpreterBridge());
  }

  if (method->IsNative()) {
    // Unregistering restores the dlsym lookup stub.
    method->UnregisterNative();

    if (enter_interpreter || quick_code == nullptr) {
      // We have a native method here without code. Then it should have either the generic JNI
      // trampoline as entrypoint (non-static), or the resolution trampoline (static).
      // TODO: this doesn't handle all the cases where trampolines may be installed.
      const void* entry_point = method->GetEntryPointFromQuickCompiledCode();
      DCHECK(IsQuickGenericJniStub(entry_point) || IsQuickResolutionStub(entry_point));
    }
  }
}

void ClassLinker::SetupClass(const DexFile& dex_file,
                             const DexFile::ClassDef& dex_class_def,
                             Handle<mirror::Class> klass,
                             mirror::ClassLoader* class_loader) {
  CHECK(klass.Get() != nullptr);
  CHECK(klass->GetDexCache() != nullptr);
  CHECK_EQ(mirror::Class::kStatusNotReady, klass->GetStatus());
  const char* descriptor = dex_file.GetClassDescriptor(dex_class_def);
  CHECK(descriptor != nullptr);

  klass->SetClass(GetClassRoot(kJavaLangClass));
  uint32_t access_flags = dex_class_def.GetJavaAccessFlags();
  CHECK_EQ(access_flags & ~kAccJavaFlagsMask, 0U);
  klass->SetAccessFlags(access_flags);
  klass->SetClassLoader(class_loader);
  DCHECK_EQ(klass->GetPrimitiveType(), Primitive::kPrimNot);
  mirror::Class::SetStatus(klass, mirror::Class::kStatusIdx, nullptr);

  klass->SetDexClassDefIndex(dex_file.GetIndexForClassDef(dex_class_def));
  klass->SetDexTypeIndex(dex_class_def.class_idx_);
  CHECK(klass->GetDexCacheStrings() != nullptr);
}

void ClassLinker::LoadClass(Thread* self,
                            const DexFile& dex_file,
                            const DexFile::ClassDef& dex_class_def,
                            Handle<mirror::Class> klass) {
  const uint8_t* class_data = dex_file.GetClassData(dex_class_def);
  if (class_data == nullptr) {
    return;  // no fields or methods - for example a marker interface
  }
  bool has_oat_class = false;
  if (Runtime::Current()->IsStarted() && !Runtime::Current()->IsAotCompiler()) {
    OatFile::OatClass oat_class = FindOatClass(dex_file, klass->GetDexClassDefIndex(),
                                               &has_oat_class);
    if (has_oat_class) {
      LoadClassMembers(self, dex_file, class_data, klass, &oat_class);
    }
  }
  if (!has_oat_class) {
    LoadClassMembers(self, dex_file, class_data, klass, nullptr);
  }
}

LengthPrefixedArray<ArtField>* ClassLinker::AllocArtFieldArray(Thread* self,
                                                               LinearAlloc* allocator,
                                                               size_t length) {
  if (length == 0) {
    return nullptr;
  }
  // If the ArtField alignment changes, review all uses of LengthPrefixedArray<ArtField>.
  static_assert(alignof(ArtField) == 4, "ArtField alignment is expected to be 4.");
  size_t storage_size = LengthPrefixedArray<ArtField>::ComputeSize(length);
  void* array_storage = allocator->Alloc(self, storage_size);
  auto* ret = new(array_storage) LengthPrefixedArray<ArtField>(length);
  CHECK(ret != nullptr);
  std::uninitialized_fill_n(&ret->At(0), length, ArtField());
  return ret;
}

LengthPrefixedArray<ArtMethod>* ClassLinker::AllocArtMethodArray(Thread* self,
                                                                 LinearAlloc* allocator,
                                                                 size_t length) {
  if (length == 0) {
    return nullptr;
  }
  const size_t method_alignment = ArtMethod::Alignment(image_pointer_size_);
  const size_t method_size = ArtMethod::Size(image_pointer_size_);
  const size_t storage_size =
      LengthPrefixedArray<ArtMethod>::ComputeSize(length, method_size, method_alignment);
  void* array_storage = allocator->Alloc(self, storage_size);
  auto* ret = new (array_storage) LengthPrefixedArray<ArtMethod>(length);
  CHECK(ret != nullptr);
  for (size_t i = 0; i < length; ++i) {
    new(reinterpret_cast<void*>(&ret->At(i, method_size, method_alignment))) ArtMethod;
  }
  return ret;
}

LinearAlloc* ClassLinker::GetAllocatorForClassLoader(mirror::ClassLoader* class_loader) {
  if (class_loader == nullptr) {
    return Runtime::Current()->GetLinearAlloc();
  }
  LinearAlloc* allocator = class_loader->GetAllocator();
  DCHECK(allocator != nullptr);
  return allocator;
}

LinearAlloc* ClassLinker::GetOrCreateAllocatorForClassLoader(mirror::ClassLoader* class_loader) {
  if (class_loader == nullptr) {
    return Runtime::Current()->GetLinearAlloc();
  }
  WriterMutexLock mu(Thread::Current(), *Locks::classlinker_classes_lock_);
  LinearAlloc* allocator = class_loader->GetAllocator();
  if (allocator == nullptr) {
    RegisterClassLoader(class_loader);
    allocator = class_loader->GetAllocator();
    CHECK(allocator != nullptr);
  }
  return allocator;
}

void ClassLinker::LoadClassMembers(Thread* self,
                                   const DexFile& dex_file,
                                   const uint8_t* class_data,
                                   Handle<mirror::Class> klass,
                                   const OatFile::OatClass* oat_class) {
  {
    // Note: We cannot have thread suspension until the field and method arrays are setup or else
    // Class::VisitFieldRoots may miss some fields or methods.
    ScopedAssertNoThreadSuspension nts(self, __FUNCTION__);
    // Does this dex_file have exact profiling?
    ExactProfileFile* exact_profile = nullptr;
    Runtime::DexProfilersMap& dex_profiles = Runtime::Current()->GetDexProfilers();
    auto exact_it = dex_profiles.find(&dex_file);
    if (exact_it != dex_profiles.end()) {
      exact_profile = exact_it->second.first;
    }

    // Load static fields.
    // We allow duplicate definitions of the same field in a class_data_item
    // but ignore the repeated indexes here, b/21868015.
    LinearAlloc* const allocator = GetAllocatorForClassLoader(klass->GetClassLoader());
    ClassDataItemIterator it(dex_file, class_data);
    LengthPrefixedArray<ArtField>* sfields = AllocArtFieldArray(self,
                                                                allocator,
                                                                it.NumStaticFields());
    size_t num_sfields = 0;
    uint32_t last_field_idx = 0u;
    for (; it.HasNextStaticField(); it.Next()) {
      uint32_t field_idx = it.GetMemberIndex();
      DCHECK_GE(field_idx, last_field_idx);  // Ordering enforced by DexFileVerifier.
      if (num_sfields == 0 || LIKELY(field_idx > last_field_idx)) {
        DCHECK_LT(num_sfields, it.NumStaticFields());
        LoadField(it, klass, &sfields->At(num_sfields));
        ++num_sfields;
        last_field_idx = field_idx;
      }
    }
    // Load instance fields.
    LengthPrefixedArray<ArtField>* ifields = AllocArtFieldArray(self,
                                                                allocator,
                                                                it.NumInstanceFields());
    size_t num_ifields = 0u;
    last_field_idx = 0u;
    for (; it.HasNextInstanceField(); it.Next()) {
      uint32_t field_idx = it.GetMemberIndex();
      DCHECK_GE(field_idx, last_field_idx);  // Ordering enforced by DexFileVerifier.
      if (num_ifields == 0 || LIKELY(field_idx > last_field_idx)) {
        DCHECK_LT(num_ifields, it.NumInstanceFields());
        LoadField(it, klass, &ifields->At(num_ifields));
        ++num_ifields;
        last_field_idx = field_idx;
      }
    }
    if (UNLIKELY(num_sfields != it.NumStaticFields()) ||
        UNLIKELY(num_ifields != it.NumInstanceFields())) {
      LOG(WARNING) << "Duplicate fields in class " << PrettyDescriptor(klass.Get())
          << " (unique static fields: " << num_sfields << "/" << it.NumStaticFields()
          << ", unique instance fields: " << num_ifields << "/" << it.NumInstanceFields() << ")";
      // NOTE: Not shrinking the over-allocated sfields/ifields, just setting size.
      if (sfields != nullptr) {
        sfields->SetSize(num_sfields);
      }
      if (ifields != nullptr) {
        ifields->SetSize(num_ifields);
      }
    }
    // Set the field arrays.
    klass->SetSFieldsPtr(sfields);
    DCHECK_EQ(klass->NumStaticFields(), num_sfields);
    klass->SetIFieldsPtr(ifields);
    DCHECK_EQ(klass->NumInstanceFields(), num_ifields);
    // Load methods.
    klass->SetMethodsPtr(
        AllocArtMethodArray(self, allocator, it.NumDirectMethods() + it.NumVirtualMethods()),
        it.NumDirectMethods(),
        it.NumVirtualMethods());
    size_t class_def_method_index = 0;
    uint32_t last_dex_method_index = DexFile::kDexNoIndex;
    size_t last_class_def_method_index = 0;
    // TODO These should really use the iterators.
    for (size_t i = 0; it.HasNextDirectMethod(); i++, it.Next()) {
      ArtMethod* method = klass->GetDirectMethodUnchecked(i, image_pointer_size_);
      LoadMethod(self, dex_file, it, klass, method);
      LinkCode(method, oat_class, class_def_method_index);
      uint32_t it_method_index = it.GetMemberIndex();
      if (last_dex_method_index == it_method_index) {
        // duplicate case
        method->SetMethodIndex(last_class_def_method_index);
      } else {
        method->SetMethodIndex(class_def_method_index);
        last_dex_method_index = it_method_index;
        last_class_def_method_index = class_def_method_index;
      }
      if (exact_profile != nullptr && exact_profile->generating_profile) {
        MutexLock mu_prof(Thread::Current(), *Locks::profiler_lock_);
        ExactProfiler::AllocateProfileCounters(method, exact_profile, &dex_file,
                                               exact_it->second.second, it.GetMemberIndex());
      }
      class_def_method_index++;
    }
    for (size_t i = 0; it.HasNextVirtualMethod(); i++, it.Next()) {
      ArtMethod* method = klass->GetVirtualMethodUnchecked(i, image_pointer_size_);
      LoadMethod(self, dex_file, it, klass, method);
      DCHECK_EQ(class_def_method_index, it.NumDirectMethods() + i);
      LinkCode(method, oat_class, class_def_method_index);
      if (exact_profile != nullptr && exact_profile->generating_profile) {
        MutexLock mu_prof(Thread::Current(), *Locks::profiler_lock_);
        ExactProfiler::AllocateProfileCounters(method, exact_profile, &dex_file,
                                               exact_it->second.second, it.GetMemberIndex());
      }
      class_def_method_index++;
    }
    DCHECK(!it.HasNext());
  }
  // Ensure that the card is marked so that remembered sets pick up native roots.
  Runtime::Current()->GetHeap()->WriteBarrierEveryFieldOf(klass.Get());
  self->AllowThreadSuspension();
}

void ClassLinker::LoadField(const ClassDataItemIterator& it,
                            Handle<mirror::Class> klass,
                            ArtField* dst) {
  const uint32_t field_idx = it.GetMemberIndex();
  dst->SetDexFieldIndex(field_idx);
  dst->SetDeclaringClass(klass.Get());
  dst->SetAccessFlags(it.GetFieldAccessFlags());
}

void ClassLinker::LoadMethod(Thread* self,
                             const DexFile& dex_file,
                             const ClassDataItemIterator& it,
                             Handle<mirror::Class> klass,
                             ArtMethod* dst) {
  uint32_t dex_method_idx = it.GetMemberIndex();
  const DexFile::MethodId& method_id = dex_file.GetMethodId(dex_method_idx);
  const char* method_name = dex_file.StringDataByIdx(method_id.name_idx_);

  ScopedAssertNoThreadSuspension ants(self, "LoadMethod");
  dst->SetDexMethodIndex(dex_method_idx);
  dst->SetDeclaringClass(klass.Get());
  dst->SetCodeItemOffset(it.GetMethodCodeItemOffset());

  dst->SetDexCacheResolvedMethods(klass->GetDexCache()->GetResolvedMethods(), image_pointer_size_);
  dst->SetDexCacheResolvedTypes(klass->GetDexCache()->GetResolvedTypes(), image_pointer_size_);

  uint32_t access_flags = it.GetMethodAccessFlags();

  if (UNLIKELY(strcmp("finalize", method_name) == 0)) {
    // Set finalizable flag on declaring class.
    if (strcmp("V", dex_file.GetShorty(method_id.proto_idx_)) == 0) {
      // Void return type.
      if (klass->GetClassLoader() != nullptr) {  // All non-boot finalizer methods are flagged.
        klass->SetFinalizable();
      } else {
        std::string temp;
        const char* klass_descriptor = klass->GetDescriptor(&temp);
        // The Enum class declares a "final" finalize() method to prevent subclasses from
        // introducing a finalizer. We don't want to set the finalizable flag for Enum or its
        // subclasses, so we exclude it here.
        // We also want to avoid setting the flag on Object, where we know that finalize() is
        // empty.
        if (strcmp(klass_descriptor, "Ljava/lang/Object;") != 0 &&
            strcmp(klass_descriptor, "Ljava/lang/Enum;") != 0) {
          klass->SetFinalizable();
        }
      }
    }
  } else if (method_name[0] == '<') {
    // Fix broken access flags for initializers. Bug 11157540.
    bool is_init = (strcmp("<init>", method_name) == 0);
    bool is_clinit = !is_init && (strcmp("<clinit>", method_name) == 0);
    if (UNLIKELY(!is_init && !is_clinit)) {
      LOG(WARNING) << "Unexpected '<' at start of method name " << method_name;
    } else {
      if (UNLIKELY((access_flags & kAccConstructor) == 0)) {
        LOG(WARNING) << method_name << " didn't have expected constructor access flag in class "
            << PrettyDescriptor(klass.Get()) << " in dex file " << dex_file.GetLocation();
        access_flags |= kAccConstructor;
      }
    }
  }
  dst->SetAccessFlags(access_flags);
}

void ClassLinker::AppendToBootClassPath(Thread* self, const DexFile& dex_file) {
  StackHandleScope<1> hs(self);
  Handle<mirror::DexCache> dex_cache(hs.NewHandle(AllocDexCache(
      self,
      dex_file,
      Runtime::Current()->GetLinearAlloc())));
  CHECK(dex_cache.Get() != nullptr) << "Failed to allocate dex cache for "
                                    << dex_file.GetLocation();
  AppendToBootClassPath(dex_file, dex_cache);
}

void ClassLinker::AppendToBootClassPath(const DexFile& dex_file,
                                        Handle<mirror::DexCache> dex_cache) {
  CHECK(dex_cache.Get() != nullptr) << dex_file.GetLocation();
  boot_class_path_.push_back(&dex_file);
  RegisterDexFile(dex_file, dex_cache);
}

void ClassLinker::RegisterDexFileLocked(const DexFile& dex_file,
                                        Handle<mirror::DexCache> dex_cache) {
  Thread* const self = Thread::Current();
  dex_lock_.AssertExclusiveHeld(self);
  CHECK(dex_cache.Get() != nullptr) << dex_file.GetLocation();
  // For app images, the dex cache location may be a suffix of the dex file location since the
  // dex file location is an absolute path.
  const std::string dex_cache_location = dex_cache->GetLocation()->ToModifiedUtf8();
  const size_t dex_cache_length = dex_cache_location.length();
  CHECK_GT(dex_cache_length, 0u) << dex_file.GetLocation();
  std::string dex_file_location = dex_file.GetLocation();
  CHECK_GE(dex_file_location.length(), dex_cache_length)
      << dex_cache_location << " " << dex_file.GetLocation();
  // Take suffix.
  const std::string dex_file_suffix = dex_file_location.substr(
      dex_file_location.length() - dex_cache_length,
      dex_cache_length);
  // Example dex_cache location is SettingsProvider.apk and
  // dex file location is /system/priv-app/SettingsProvider/SettingsProvider.apk
  CHECK_EQ(dex_cache_location, dex_file_suffix);
  // Clean up pass to remove null dex caches.
  // Null dex caches can occur due to class unloading and we are lazily removing null entries.
  JavaVMExt* const vm = self->GetJniEnv()->vm;
  for (auto it = dex_caches_.begin(); it != dex_caches_.end(); ) {
    DexCacheData data = *it;
    if (self->IsJWeakCleared(data.weak_root)) {
      vm->DeleteWeakGlobalRef(self, data.weak_root);
      it = dex_caches_.erase(it);
    } else {
      ++it;
    }
  }
  jweak dex_cache_jweak = vm->AddWeakGlobalRef(self, dex_cache.Get());
  dex_cache->SetDexFile(&dex_file);
  DexCacheData data;
  data.weak_root = dex_cache_jweak;
  data.dex_file = dex_cache->GetDexFile();
  data.resolved_types = dex_cache->GetResolvedTypes();
  dex_caches_.push_back(data);
}

mirror::DexCache* ClassLinker::RegisterDexFile(const DexFile& dex_file,
                                               mirror::ClassLoader* class_loader) {
  Thread* self = Thread::Current();
  {
    ReaderMutexLock mu(self, dex_lock_);
    mirror::DexCache* dex_cache = FindDexCacheLocked(self, dex_file, true);
    if (dex_cache != nullptr) {
      return dex_cache;
    }
  }
  LinearAlloc* const linear_alloc = GetOrCreateAllocatorForClassLoader(class_loader);
  DCHECK(linear_alloc != nullptr);
  ClassTable* table;
  {
    WriterMutexLock mu(self, *Locks::classlinker_classes_lock_);
    table = InsertClassTableForClassLoader(class_loader);
  }
  // Don't alloc while holding the lock, since allocation may need to
  // suspend all threads and another thread may need the dex_lock_ to
  // get to a suspend point.
  StackHandleScope<1> hs(self);
  Handle<mirror::DexCache> h_dex_cache(hs.NewHandle(AllocDexCache(self, dex_file, linear_alloc)));
  {
    WriterMutexLock mu(self, dex_lock_);
    mirror::DexCache* dex_cache = FindDexCacheLocked(self, dex_file, true);
    if (dex_cache != nullptr) {
      return dex_cache;
    }
    if (h_dex_cache.Get() == nullptr) {
      self->AssertPendingOOMException();
      return nullptr;
    }
    RegisterDexFileLocked(dex_file, h_dex_cache);
  }
  table->InsertStrongRoot(h_dex_cache.Get());
  return h_dex_cache.Get();
}

void ClassLinker::RegisterDexFile(const DexFile& dex_file,
                                  Handle<mirror::DexCache> dex_cache) {
  WriterMutexLock mu(Thread::Current(), dex_lock_);
  RegisterDexFileLocked(dex_file, dex_cache);
}

mirror::DexCache* ClassLinker::FindDexCache(Thread* self,
                                            const DexFile& dex_file,
                                            bool allow_failure) {
  ReaderMutexLock mu(self, dex_lock_);
  return FindDexCacheLocked(self, dex_file, allow_failure);
}

mirror::DexCache* ClassLinker::FindDexCacheLocked(Thread* self,
                                                  const DexFile& dex_file,
                                                  bool allow_failure) {
  // Search assuming unique-ness of dex file.
  for (const DexCacheData& data : dex_caches_) {
    // Avoid decoding (and read barriers) other unrelated dex caches.
    if (data.dex_file == &dex_file) {
      mirror::DexCache* dex_cache =
          down_cast<mirror::DexCache*>(self->DecodeJObject(data.weak_root));
      if (dex_cache != nullptr) {
        return dex_cache;
      } else {
        break;
      }
    }
  }
  if (allow_failure) {
    return nullptr;
  }
  std::string location(dex_file.GetLocation());
  // Failure, dump diagnostic and abort.
  for (const DexCacheData& data : dex_caches_) {
    mirror::DexCache* dex_cache = down_cast<mirror::DexCache*>(self->DecodeJObject(data.weak_root));
    if (dex_cache != nullptr) {
      LOG(ERROR) << "Registered dex file " << dex_cache->GetDexFile()->GetLocation();
    }
  }
  LOG(FATAL) << "Failed to find DexCache for DexFile " << location;
  UNREACHABLE();
}

void ClassLinker::FixupDexCaches(ArtMethod* resolution_method) {
  Thread* const self = Thread::Current();
  ReaderMutexLock mu(self, dex_lock_);
  for (const DexCacheData& data : dex_caches_) {
    if (!self->IsJWeakCleared(data.weak_root)) {
      mirror::DexCache* dex_cache = down_cast<mirror::DexCache*>(
          self->DecodeJObject(data.weak_root));
      if (dex_cache != nullptr) {
        dex_cache->Fixup(resolution_method, image_pointer_size_);
      }
    }
  }
}

mirror::Class* ClassLinker::CreatePrimitiveClass(Thread* self, Primitive::Type type) {
  mirror::Class* klass = AllocClass(self, mirror::Class::PrimitiveClassSize(image_pointer_size_));
  if (UNLIKELY(klass == nullptr)) {
    self->AssertPendingOOMException();
    return nullptr;
  }
  return InitializePrimitiveClass(klass, type);
}

mirror::Class* ClassLinker::InitializePrimitiveClass(mirror::Class* primitive_class,
                                                     Primitive::Type type) {
  CHECK(primitive_class != nullptr);
  // Must hold lock on object when initializing.
  Thread* self = Thread::Current();
  StackHandleScope<1> hs(self);
  Handle<mirror::Class> h_class(hs.NewHandle(primitive_class));
  ObjectLock<mirror::Class> lock(self, h_class);
  h_class->SetAccessFlags(kAccPublic | kAccFinal | kAccAbstract);
  h_class->SetPrimitiveType(type);
  mirror::Class::SetStatus(h_class, mirror::Class::kStatusInitialized, self);
  const char* descriptor = Primitive::Descriptor(type);
  mirror::Class* existing = InsertClass(descriptor, h_class.Get(),
                                        ComputeModifiedUtf8Hash(descriptor));
  CHECK(existing == nullptr) << "InitPrimitiveClass(" << type << ") failed";
  return h_class.Get();
}

// Create an array class (i.e. the class object for the array, not the
// array itself).  "descriptor" looks like "[C" or "[[[[B" or
// "[Ljava/lang/String;".
//
// If "descriptor" refers to an array of primitives, look up the
// primitive type's internally-generated class object.
//
// "class_loader" is the class loader of the class that's referring to
// us.  It's used to ensure that we're looking for the element type in
// the right context.  It does NOT become the class loader for the
// array class; that always comes from the base element class.
//
// Returns null with an exception raised on failure.
mirror::Class* ClassLinker::CreateArrayClass(Thread* self, const char* descriptor, size_t hash,
                                             Handle<mirror::ClassLoader> class_loader) {
  // Identify the underlying component type
  CHECK_EQ('[', descriptor[0]);
  StackHandleScope<2> hs(self);
  MutableHandle<mirror::Class> component_type(hs.NewHandle(FindClass(self, descriptor + 1,
                                                                     class_loader)));
  if (component_type.Get() == nullptr) {
    DCHECK(self->IsExceptionPending());
    // We need to accept erroneous classes as component types.
    const size_t component_hash = ComputeModifiedUtf8Hash(descriptor + 1);
    component_type.Assign(LookupClass(self, descriptor + 1, component_hash, class_loader.Get()));
    if (component_type.Get() == nullptr) {
      DCHECK(self->IsExceptionPending());
      return nullptr;
    } else {
      self->ClearException();
    }
  }
  if (UNLIKELY(component_type->IsPrimitiveVoid())) {
    ThrowNoClassDefFoundError("Attempt to create array of void primitive type");
    return nullptr;
  }
  // See if the component type is already loaded.  Array classes are
  // always associated with the class loader of their underlying
  // element type -- an array of Strings goes with the loader for
  // java/lang/String -- so we need to look for it there.  (The
  // caller should have checked for the existence of the class
  // before calling here, but they did so with *their* class loader,
  // not the component type's loader.)
  //
  // If we find it, the caller adds "loader" to the class' initiating
  // loader list, which should prevent us from going through this again.
  //
  // This call is unnecessary if "loader" and "component_type->GetClassLoader()"
  // are the same, because our caller (FindClass) just did the
  // lookup.  (Even if we get this wrong we still have correct behavior,
  // because we effectively do this lookup again when we add the new
  // class to the hash table --- necessary because of possible races with
  // other threads.)
  if (class_loader.Get() != component_type->GetClassLoader()) {
    mirror::Class* new_class = LookupClass(self, descriptor, hash, component_type->GetClassLoader());
    if (new_class != nullptr) {
      return new_class;
    }
  }

  // Fill out the fields in the Class.
  //
  // It is possible to execute some methods against arrays, because
  // all arrays are subclasses of java_lang_Object_, so we need to set
  // up a vtable.  We can just point at the one in java_lang_Object_.
  //
  // Array classes are simple enough that we don't need to do a full
  // link step.
  auto new_class = hs.NewHandle<mirror::Class>(nullptr);
  if (UNLIKELY(!init_done_)) {
    // Classes that were hand created, ie not by FindSystemClass
    if (strcmp(descriptor, "[Ljava/lang/Class;") == 0) {
      new_class.Assign(GetClassRoot(kClassArrayClass));
    } else if (strcmp(descriptor, "[Ljava/lang/Object;") == 0) {
      new_class.Assign(GetClassRoot(kObjectArrayClass));
    } else if (strcmp(descriptor, GetClassRootDescriptor(kJavaLangStringArrayClass)) == 0) {
      new_class.Assign(GetClassRoot(kJavaLangStringArrayClass));
    } else if (strcmp(descriptor, "[C") == 0) {
      new_class.Assign(GetClassRoot(kCharArrayClass));
    } else if (strcmp(descriptor, "[I") == 0) {
      new_class.Assign(GetClassRoot(kIntArrayClass));
    } else if (strcmp(descriptor, "[J") == 0) {
      new_class.Assign(GetClassRoot(kLongArrayClass));
    }
  }
  if (new_class.Get() == nullptr) {
    new_class.Assign(AllocClass(self, mirror::Array::ClassSize(image_pointer_size_)));
    if (new_class.Get() == nullptr) {
      self->AssertPendingOOMException();
      return nullptr;
    }
    new_class->SetComponentType(component_type.Get());
  }
  ObjectLock<mirror::Class> lock(self, new_class);  // Must hold lock on object when initializing.
  DCHECK(new_class->GetComponentType() != nullptr);
  mirror::Class* java_lang_Object = GetClassRoot(kJavaLangObject);
  new_class->SetSuperClass(java_lang_Object);
  if (Runtime::Current()->UseCHA()) {
    mirror::Class* parent = java_lang_Object;
    mirror::Class* child = new_class.Get();
    AddCHA(parent, child);
  }
  new_class->SetVTable(java_lang_Object->GetVTable());
  new_class->SetPrimitiveType(Primitive::kPrimNot);
  new_class->SetClassLoader(component_type->GetClassLoader());
  if (component_type->IsPrimitive()) {
    new_class->SetClassFlags(mirror::kClassFlagNoReferenceFields);
  } else {
    new_class->SetClassFlags(mirror::kClassFlagObjectArray);
  }
  mirror::Class::SetStatus(new_class, mirror::Class::kStatusLoaded, self);
  {
    ArtMethod* imt[mirror::Class::kImtSize];
    std::fill_n(imt, arraysize(imt), Runtime::Current()->GetImtUnimplementedMethod());
    new_class->PopulateEmbeddedImtAndVTable(imt, image_pointer_size_);
  }
  mirror::Class::SetStatus(new_class, mirror::Class::kStatusInitialized, self);
  // don't need to set new_class->SetObjectSize(..)
  // because Object::SizeOf delegates to Array::SizeOf


  // All arrays have java/lang/Cloneable and java/io/Serializable as
  // interfaces.  We need to set that up here, so that stuff like
  // "instanceof" works right.
  //
  // Note: The GC could run during the call to FindSystemClass,
  // so we need to make sure the class object is GC-valid while we're in
  // there.  Do this by clearing the interface list so the GC will just
  // think that the entries are null.


  // Use the single, global copies of "interfaces" and "iftable"
  // (remember not to free them for arrays).
  {
    mirror::IfTable* array_iftable = array_iftable_.Read();
    CHECK(array_iftable != nullptr);
    new_class->SetIfTable(array_iftable);
  }

  // Inherit access flags from the component type.
  int access_flags = new_class->GetComponentType()->GetAccessFlags();
  // Lose any implementation detail flags; in particular, arrays aren't finalizable.
  access_flags &= kAccJavaFlagsMask;
  // Arrays can't be used as a superclass or interface, so we want to add "abstract final"
  // and remove "interface".
  access_flags |= kAccAbstract | kAccFinal;
  access_flags &= ~kAccInterface;

  new_class->SetAccessFlags(access_flags);

  mirror::Class* existing = InsertClass(descriptor, new_class.Get(), hash);
  if (existing == nullptr) {
    jit::Jit::NewTypeLoadedIfUsingJit(new_class.Get());
    return new_class.Get();
  }
  // Another thread must have loaded the class after we
  // started but before we finished.  Abandon what we've
  // done.
  //
  // (Yes, this happens.)

  return existing;
}

mirror::Class* ClassLinker::FindPrimitiveClass(char type) {
  switch (type) {
    case 'B':
      return GetClassRoot(kPrimitiveByte);
    case 'C':
      return GetClassRoot(kPrimitiveChar);
    case 'D':
      return GetClassRoot(kPrimitiveDouble);
    case 'F':
      return GetClassRoot(kPrimitiveFloat);
    case 'I':
      return GetClassRoot(kPrimitiveInt);
    case 'J':
      return GetClassRoot(kPrimitiveLong);
    case 'S':
      return GetClassRoot(kPrimitiveShort);
    case 'Z':
      return GetClassRoot(kPrimitiveBoolean);
    case 'V':
      return GetClassRoot(kPrimitiveVoid);
    default:
      break;
  }
  std::string printable_type(PrintableChar(type));
  ThrowNoClassDefFoundError("Not a primitive type: %s", printable_type.c_str());
  return nullptr;
}

mirror::Class* ClassLinker::InsertClass(const char* descriptor, mirror::Class* klass, size_t hash) {
  if (VLOG_IS_ON(class_linker)) {
    mirror::DexCache* dex_cache = klass->GetDexCache();
    std::string source;
    if (dex_cache != nullptr) {
      source += " from ";
      source += dex_cache->GetLocation()->ToModifiedUtf8();
    }
    LOG(INFO) << "Loaded class " << descriptor << source;
  }
  WriterMutexLock mu(Thread::Current(), *Locks::classlinker_classes_lock_);
  mirror::ClassLoader* const class_loader = klass->GetClassLoader();
  ClassTable* const class_table = InsertClassTableForClassLoader(class_loader);
  mirror::Class* existing = class_table->Lookup(descriptor, hash);
  if (existing != nullptr) {
    return existing;
  }
  if (kIsDebugBuild &&
      !klass->IsTemp() &&
      class_loader == nullptr &&
      dex_cache_boot_image_class_lookup_required_) {
    // Check a class loaded with the system class loader matches one in the image if the class
    // is in the image.
    existing = LookupClassFromBootImage(descriptor);
    if (existing != nullptr) {
      CHECK_EQ(klass, existing);
    }
  }
  VerifyObject(klass);
  class_table->InsertWithHash(klass, hash);
  if (class_loader != nullptr) {
    // This is necessary because we need to have the card dirtied for remembered sets.
    Runtime::Current()->GetHeap()->WriteBarrierEveryFieldOf(class_loader);
  }
  if (log_new_class_table_roots_) {
    new_class_roots_.push_back(GcRoot<mirror::Class>(klass));
  }
  return nullptr;
}

// TODO This should really be in mirror::Class.
void ClassLinker::UpdateClassMethods(mirror::Class* klass,
                                     LengthPrefixedArray<ArtMethod>* new_methods) {
  klass->SetMethodsPtrUnchecked(new_methods,
                                klass->NumDirectMethods(),
                                klass->NumDeclaredVirtualMethods());
  // Need to mark the card so that the remembered sets and mod union tables get updated.
  Runtime::Current()->GetHeap()->WriteBarrierEveryFieldOf(klass);
}

bool ClassLinker::RemoveClass(const char* descriptor, mirror::ClassLoader* class_loader) {
  WriterMutexLock mu(Thread::Current(), *Locks::classlinker_classes_lock_);
  ClassTable* const class_table = ClassTableForClassLoader(class_loader);
  return class_table != nullptr && class_table->Remove(descriptor);
}

mirror::Class* ClassLinker::LookupClass(Thread* self,
                                        const char* descriptor,
                                        size_t hash,
                                        mirror::ClassLoader* class_loader) {
  {
    ReaderMutexLock mu(self, *Locks::classlinker_classes_lock_);
    ClassTable* const class_table = ClassTableForClassLoader(class_loader);
    if (class_table != nullptr) {
      mirror::Class* result = class_table->Lookup(descriptor, hash);
      if (result != nullptr) {
        return result;
      }
    }
  }
  if (class_loader != nullptr || !dex_cache_boot_image_class_lookup_required_) {
    return nullptr;
  }
  // Lookup failed but need to search dex_caches_.
  mirror::Class* result = LookupClassFromBootImage(descriptor);
  if (result != nullptr) {
    result = InsertClass(descriptor, result, hash);
  } else {
    // Searching the image dex files/caches failed, we don't want to get into this situation
    // often as map searches are faster, so after kMaxFailedDexCacheLookups move all image
    // classes into the class table.
    constexpr uint32_t kMaxFailedDexCacheLookups = 1000;
    if (++failed_dex_cache_class_lookups_ > kMaxFailedDexCacheLookups) {
      AddBootImageClassesToClassTable();
    }
  }
  return result;
}

static std::vector<mirror::ObjectArray<mirror::DexCache>*> GetImageDexCaches(
    std::vector<gc::space::ImageSpace*> image_spaces) SHARED_REQUIRES(Locks::mutator_lock_) {
  CHECK(!image_spaces.empty());
  std::vector<mirror::ObjectArray<mirror::DexCache>*> dex_caches_vector;
  for (gc::space::ImageSpace* image_space : image_spaces) {
    mirror::Object* root = image_space->GetImageHeader().GetImageRoot(ImageHeader::kDexCaches);
    DCHECK(root != nullptr);
    dex_caches_vector.push_back(root->AsObjectArray<mirror::DexCache>());
  }
  return dex_caches_vector;
}

void ClassLinker::AddBootImageClassesToClassTable() {
  if (dex_cache_boot_image_class_lookup_required_) {
    AddImageClassesToClassTable(Runtime::Current()->GetHeap()->GetBootImageSpaces(),
                                /*class_loader*/nullptr);
    dex_cache_boot_image_class_lookup_required_ = false;
  }
}

void ClassLinker::AddImageClassesToClassTable(std::vector<gc::space::ImageSpace*> image_spaces,
                                              mirror::ClassLoader* class_loader) {
  Thread* self = Thread::Current();
  WriterMutexLock mu(self, *Locks::classlinker_classes_lock_);
  ScopedAssertNoThreadSuspension ants(self, "Moving image classes to class table");

  ClassTable* const class_table = InsertClassTableForClassLoader(class_loader);

  std::string temp;
  std::vector<mirror::ObjectArray<mirror::DexCache>*> dex_caches_vector =
      GetImageDexCaches(image_spaces);
  for (mirror::ObjectArray<mirror::DexCache>* dex_caches : dex_caches_vector) {
    for (int32_t i = 0; i < dex_caches->GetLength(); i++) {
      mirror::DexCache* dex_cache = dex_caches->Get(i);
      GcRoot<mirror::Class>* types = dex_cache->GetResolvedTypes();
      for (int32_t j = 0, num_types = dex_cache->NumResolvedTypes(); j < num_types; j++) {
        mirror::Class* klass = types[j].Read();
        if (klass != nullptr) {
          DCHECK_EQ(klass->GetClassLoader(), class_loader);
          const char* descriptor = klass->GetDescriptor(&temp);
          size_t hash = ComputeModifiedUtf8Hash(descriptor);
          mirror::Class* existing = class_table->Lookup(descriptor, hash);
          if (existing != nullptr) {
            CHECK_EQ(existing, klass) << PrettyClassAndClassLoader(existing) << " != "
                << PrettyClassAndClassLoader(klass);
          } else {
            class_table->Insert(klass);
            if (log_new_class_table_roots_) {
              new_class_roots_.push_back(GcRoot<mirror::Class>(klass));
            }
          }
        }
      }
    }
  }
}

class MoveClassTableToPreZygoteVisitor : public ClassLoaderVisitor {
 public:
  explicit MoveClassTableToPreZygoteVisitor() {}

  void Visit(mirror::ClassLoader* class_loader)
      REQUIRES(Locks::classlinker_classes_lock_)
      SHARED_REQUIRES(Locks::mutator_lock_) OVERRIDE {
    ClassTable* const class_table = class_loader->GetClassTable();
    if (class_table != nullptr) {
      class_table->FreezeSnapshot();
    }
  }
};

void ClassLinker::MoveClassTableToPreZygote() {
  WriterMutexLock mu(Thread::Current(), *Locks::classlinker_classes_lock_);
  boot_class_table_.FreezeSnapshot();
  MoveClassTableToPreZygoteVisitor visitor;
  VisitClassLoaders(&visitor);
}

mirror::Class* ClassLinker::LookupClassFromBootImage(const char* descriptor) {
  ScopedAssertNoThreadSuspension ants(Thread::Current(), "Image class lookup");
  std::vector<mirror::ObjectArray<mirror::DexCache>*> dex_caches_vector =
      GetImageDexCaches(Runtime::Current()->GetHeap()->GetBootImageSpaces());
  for (mirror::ObjectArray<mirror::DexCache>* dex_caches : dex_caches_vector) {
    for (int32_t i = 0; i < dex_caches->GetLength(); ++i) {
      mirror::DexCache* dex_cache = dex_caches->Get(i);
      const DexFile* dex_file = dex_cache->GetDexFile();
      // Try binary searching the type index by descriptor.
      const DexFile::TypeId* type_id = dex_file->FindTypeId(descriptor);
      if (type_id != nullptr) {
        uint16_t type_idx = dex_file->GetIndexForTypeId(*type_id);
        mirror::Class* klass = dex_cache->GetResolvedType(type_idx);
        if (klass != nullptr) {
          return klass;
        }
      }
    }
  }
  return nullptr;
}

// Look up classes by hash and descriptor and put all matching ones in the result array.
class LookupClassesVisitor : public ClassLoaderVisitor {
 public:
  LookupClassesVisitor(const char* descriptor, size_t hash, std::vector<mirror::Class*>* result)
     : descriptor_(descriptor),
       hash_(hash),
       result_(result) {}

  void Visit(mirror::ClassLoader* class_loader)
      SHARED_REQUIRES(Locks::classlinker_classes_lock_, Locks::mutator_lock_) OVERRIDE {
    ClassTable* const class_table = class_loader->GetClassTable();
    mirror::Class* klass = class_table->Lookup(descriptor_, hash_);
    if (klass != nullptr) {
      result_->push_back(klass);
    }
  }

 private:
  const char* const descriptor_;
  const size_t hash_;
  std::vector<mirror::Class*>* const result_;
};

void ClassLinker::LookupClasses(const char* descriptor, std::vector<mirror::Class*>& result) {
  result.clear();
  if (dex_cache_boot_image_class_lookup_required_) {
    AddBootImageClassesToClassTable();
  }
  Thread* const self = Thread::Current();
  ReaderMutexLock mu(self, *Locks::classlinker_classes_lock_);
  const size_t hash = ComputeModifiedUtf8Hash(descriptor);
  mirror::Class* klass = boot_class_table_.Lookup(descriptor, hash);
  if (klass != nullptr) {
    result.push_back(klass);
  }
  LookupClassesVisitor visitor(descriptor, hash, &result);
  VisitClassLoaders(&visitor);
}

bool ClassLinker::AttemptSupertypeVerification(Thread* self,
                                               Handle<mirror::Class> klass,
                                               Handle<mirror::Class> supertype) {
  DCHECK(self != nullptr);
  DCHECK(klass.Get() != nullptr);
  DCHECK(supertype.Get() != nullptr);

  if (!supertype->IsVerified() && !supertype->IsErroneous()) {
    VerifyClass(self, supertype);
  }
  if (supertype->IsCompileTimeVerified()) {
    // Either we are verified or we soft failed and need to retry at runtime.
    return true;
  }
  // If we got this far then we have a hard failure.
  std::string error_msg =
      StringPrintf("Rejecting class %s that attempts to sub-type erroneous class %s",
                   PrettyDescriptor(klass.Get()).c_str(),
                   PrettyDescriptor(supertype.Get()).c_str());
  LOG(WARNING) << error_msg  << " in " << klass->GetDexCache()->GetLocation()->ToModifiedUtf8();
  StackHandleScope<1> hs(self);
  Handle<mirror::Throwable> cause(hs.NewHandle(self->GetException()));
  if (cause.Get() != nullptr) {
    // Set during VerifyClass call (if at all).
    self->ClearException();
  }
  // Change into a verify error.
  ThrowVerifyError(klass.Get(), "%s", error_msg.c_str());
  if (cause.Get() != nullptr) {
    self->GetException()->SetCause(cause.Get());
  }
  ClassReference ref(klass->GetDexCache()->GetDexFile(), klass->GetDexClassDefIndex());
  if (Runtime::Current()->IsAotCompiler()) {
    Runtime::Current()->GetCompilerCallbacks()->ClassRejected(ref);
  }
  // Need to grab the lock to change status.
  ObjectLock<mirror::Class> super_lock(self, klass);
  mirror::Class::SetStatus(klass, mirror::Class::kStatusError, self);
  return false;
}

void ClassLinker::VerifyClass(Thread* self, Handle<mirror::Class> klass, LogSeverity log_level) {
  {
    // TODO: assert that the monitor on the Class is held
    ObjectLock<mirror::Class> lock(self, klass);

    // Is somebody verifying this now?
    mirror::Class::Status old_status = klass->GetStatus();
    while (old_status == mirror::Class::kStatusVerifying ||
        old_status == mirror::Class::kStatusVerifyingAtRuntime) {
      lock.WaitIgnoringInterrupts();
      CHECK(klass->IsErroneous() || (klass->GetStatus() > old_status))
          << "Class '" << PrettyClass(klass.Get()) << "' performed an illegal verification state "
          << "transition from " << old_status << " to " << klass->GetStatus();
      old_status = klass->GetStatus();
    }

    // The class might already be erroneous, for example at compile time if we attempted to verify
    // this class as a parent to another.
    if (klass->IsErroneous()) {
      ThrowEarlierClassFailure(klass.Get());
      return;
    }

    // Don't attempt to re-verify if already sufficiently verified.
    if (klass->IsVerified()) {
      EnsureSkipAccessChecksMethods(klass);
      return;
    }
    if (klass->IsCompileTimeVerified() && Runtime::Current()->IsAotCompiler()) {
      return;
    }

    if (klass->GetStatus() == mirror::Class::kStatusResolved) {
      mirror::Class::SetStatus(klass, mirror::Class::kStatusVerifying, self);
    } else {
      CHECK_EQ(klass->GetStatus(), mirror::Class::kStatusRetryVerificationAtRuntime)
            << PrettyClass(klass.Get());
      CHECK(!Runtime::Current()->IsAotCompiler());
      mirror::Class::SetStatus(klass, mirror::Class::kStatusVerifyingAtRuntime, self);
    }

    // Skip verification if disabled.
    if (!Runtime::Current()->IsVerificationEnabled()) {
      mirror::Class::SetStatus(klass, mirror::Class::kStatusVerified, self);
      EnsureSkipAccessChecksMethods(klass);
      return;
    }
  }

  // Verify super class.
  StackHandleScope<2> hs(self);
  MutableHandle<mirror::Class> supertype(hs.NewHandle(klass->GetSuperClass()));
  // If we have a superclass and we get a hard verification failure we can return immediately.
  if (supertype.Get() != nullptr && !AttemptSupertypeVerification(self, klass, supertype)) {
    CHECK(self->IsExceptionPending()) << "Verification error should be pending.";
    return;
  }

  // Verify all default super-interfaces.
  //
  // (1) Don't bother if the superclass has already had a soft verification failure.
  //
  // (2) Interfaces shouldn't bother to do this recursive verification because they cannot cause
  //     recursive initialization by themselves. This is because when an interface is initialized
  //     directly it must not initialize its superinterfaces. We are allowed to verify regardless
  //     but choose not to for an optimization. If the interfaces is being verified due to a class
  //     initialization (which would need all the default interfaces to be verified) the class code
  //     will trigger the recursive verification anyway.
  if ((supertype.Get() == nullptr || supertype->IsVerified())  // See (1)
      && !klass->IsInterface()) {                              // See (2)
    int32_t iftable_count = klass->GetIfTableCount();
    MutableHandle<mirror::Class> iface(hs.NewHandle<mirror::Class>(nullptr));
    // Loop through all interfaces this class has defined. It doesn't matter the order.
    for (int32_t i = 0; i < iftable_count; i++) {
      iface.Assign(klass->GetIfTable()->GetInterface(i));
      DCHECK(iface.Get() != nullptr);
      // We only care if we have default interfaces and can skip if we are already verified...
      if (LIKELY(!iface->HasDefaultMethods() || iface->IsVerified())) {
        continue;
      } else if (UNLIKELY(!AttemptSupertypeVerification(self, klass, iface))) {
        // We had a hard failure while verifying this interface. Just return immediately.
        CHECK(self->IsExceptionPending()) << "Verification error should be pending.";
        return;
      } else if (UNLIKELY(!iface->IsVerified())) {
        // We softly failed to verify the iface. Stop checking and clean up.
        // Put the iface into the supertype handle so we know what caused us to fail.
        supertype.Assign(iface.Get());
        break;
      }
    }
  }

  // At this point if verification failed, then supertype is the "first" supertype that failed
  // verification (without a specific order). If verification succeeded, then supertype is either
  // null or the original superclass of klass and is verified.
  DCHECK(supertype.Get() == nullptr ||
         supertype.Get() == klass->GetSuperClass() ||
         !supertype->IsVerified());

  // Try to use verification information from the oat file, otherwise do runtime verification.
  const DexFile& dex_file = *klass->GetDexCache()->GetDexFile();
  mirror::Class::Status oat_file_class_status(mirror::Class::kStatusNotReady);
  bool preverified = VerifyClassUsingOatFile(dex_file, klass.Get(), oat_file_class_status);
  // If the oat file says the class had an error, re-run the verifier. That way we will get a
  // precise error message. To ensure a rerun, test:
  //     oat_file_class_status == mirror::Class::kStatusError => !preverified
  DCHECK(!(oat_file_class_status == mirror::Class::kStatusError) || !preverified);

  verifier::MethodVerifier::FailureKind verifier_failure = verifier::MethodVerifier::kNoFailure;
  std::string error_msg;
  if (!preverified) {
    Runtime* runtime = Runtime::Current();
    verifier_failure = verifier::MethodVerifier::VerifyClass(self,
                                                             klass.Get(),
                                                             runtime->GetCompilerCallbacks(),
                                                             runtime->IsAotCompiler(),
                                                             log_level,
                                                             &error_msg);
  }

  // Verification is done, grab the lock again.
  ObjectLock<mirror::Class> lock(self, klass);

  if (preverified || verifier_failure != verifier::MethodVerifier::kHardFailure) {
    if (!preverified && verifier_failure != verifier::MethodVerifier::kNoFailure) {
      VLOG(class_linker) << "Soft verification failure in class " << PrettyDescriptor(klass.Get())
          << " in " << klass->GetDexCache()->GetLocation()->ToModifiedUtf8()
          << " because: " << error_msg;
    }
    self->AssertNoPendingException();
    // Make sure all classes referenced by catch blocks are resolved.
    ResolveClassExceptionHandlerTypes(klass);
    if (verifier_failure == verifier::MethodVerifier::kNoFailure) {
      // Even though there were no verifier failures we need to respect whether the super-class and
      // super-default-interfaces were verified or requiring runtime reverification.
      if (supertype.Get() == nullptr || supertype->IsVerified()) {
        mirror::Class::SetStatus(klass, mirror::Class::kStatusVerified, self);
      } else {
        CHECK_EQ(supertype->GetStatus(), mirror::Class::kStatusRetryVerificationAtRuntime);
        mirror::Class::SetStatus(klass, mirror::Class::kStatusRetryVerificationAtRuntime, self);
        // Pretend a soft failure occurred so that we don't consider the class verified below.
        verifier_failure = verifier::MethodVerifier::kSoftFailure;
      }
    } else {
      CHECK_EQ(verifier_failure, verifier::MethodVerifier::kSoftFailure);
      // Soft failures at compile time should be retried at runtime. Soft
      // failures at runtime will be handled by slow paths in the generated
      // code. Set status accordingly.
      if (Runtime::Current()->IsAotCompiler()) {
        mirror::Class::SetStatus(klass, mirror::Class::kStatusRetryVerificationAtRuntime, self);
      } else {
        mirror::Class::SetStatus(klass, mirror::Class::kStatusVerified, self);
        // As this is a fake verified status, make sure the methods are _not_ marked
        // kAccSkipAccessChecks later.
        klass->SetVerificationAttempted();
      }
    }
  } else {
    VLOG(verifier) << "Verification failed on class " << PrettyDescriptor(klass.Get())
                  << " in " << klass->GetDexCache()->GetLocation()->ToModifiedUtf8()
                  << " because: " << error_msg;
    self->AssertNoPendingException();
    ThrowVerifyError(klass.Get(), "%s", error_msg.c_str());
    mirror::Class::SetStatus(klass, mirror::Class::kStatusError, self);
  }
  if (preverified || verifier_failure == verifier::MethodVerifier::kNoFailure) {
    // Class is verified so we don't need to do any access check on its methods.
    // Let the interpreter know it by setting the kAccSkipAccessChecks flag onto each
    // method.
    // Note: we're going here during compilation and at runtime. When we set the
    // kAccSkipAccessChecks flag when compiling image classes, the flag is recorded
    // in the image and is set when loading the image.

    if (UNLIKELY(Runtime::Current()->IsVerificationSoftFail())) {
      // Never skip access checks if the verification soft fail is forced.
      // Mark the class as having a verification attempt to avoid re-running the verifier.
      klass->SetVerificationAttempted();
    } else {
      EnsureSkipAccessChecksMethods(klass);
    }
  }
}

void ClassLinker::EnsureSkipAccessChecksMethods(Handle<mirror::Class> klass) {
  if (!klass->WasVerificationAttempted()) {
    klass->SetSkipAccessChecksFlagOnAllMethods(image_pointer_size_);
    klass->SetVerificationAttempted();
  }
}

bool ClassLinker::VerifyClassUsingOatFile(const DexFile& dex_file,
                                          mirror::Class* klass,
                                          mirror::Class::Status& oat_file_class_status) {
  // If we're compiling, we can only verify the class using the oat file if
  // we are not compiling the image or if the class we're verifying is not part of
  // the app.  In other words, we will only check for preverification of bootclasspath
  // classes.
  if (Runtime::Current()->IsAotCompiler()) {
    // Are we compiling the bootclasspath?
    if (Runtime::Current()->GetCompilerCallbacks()->IsBootImage()) {
      return false;
    }
    // We are compiling an app (not the image).

    // Is this an app class? (I.e. not a bootclasspath class)
    if (klass->GetClassLoader() != nullptr) {
      return false;
    }
  }

  const OatFile::OatDexFile* oat_dex_file = dex_file.GetOatDexFile();
  // In case we run without an image there won't be a backing oat file.
  if (oat_dex_file == nullptr) {
    return false;
  }

  // We may be running with a preopted oat file but without image. In this case,
  // we don't skip verification of skip_access_checks classes to ensure we initialize
  // dex caches with all types resolved during verification.
  // We need to trust image classes, as these might be coming out of a pre-opted, quickened boot
  // image (that we just failed loading), and the verifier can't be run on quickened opcodes when
  // the runtime isn't started. On the other hand, app classes can be re-verified even if they are
  // already pre-opted, as then the runtime is started.
  if (!Runtime::Current()->IsAotCompiler() &&
      !Runtime::Current()->GetHeap()->HasBootImageSpace() &&
      klass->GetClassLoader() != nullptr) {
    return false;
  }

  uint16_t class_def_index = klass->GetDexClassDefIndex();
  oat_file_class_status = oat_dex_file->GetOatClass(class_def_index).GetStatus();
  if (oat_file_class_status == mirror::Class::kStatusVerified ||
      oat_file_class_status == mirror::Class::kStatusInitialized) {
    return true;
  }
  // If we only verified a subset of the classes at compile time, we can end up with classes that
  // were resolved by the verifier.
  if (oat_file_class_status == mirror::Class::kStatusResolved) {
    return false;
  }
  if (oat_file_class_status == mirror::Class::kStatusRetryVerificationAtRuntime) {
    // Compile time verification failed with a soft error. Compile time verification can fail
    // because we have incomplete type information. Consider the following:
    // class ... {
    //   Foo x;
    //   .... () {
    //     if (...) {
    //       v1 gets assigned a type of resolved class Foo
    //     } else {
    //       v1 gets assigned a type of unresolved class Bar
    //     }
    //     iput x = v1
    // } }
    // when we merge v1 following the if-the-else it results in Conflict
    // (see verifier::RegType::Merge) as we can't know the type of Bar and we could possibly be
    // allowing an unsafe assignment to the field x in the iput (javac may have compiled this as
    // it knew Bar was a sub-class of Foo, but for us this may have been moved into a separate apk
    // at compile time).
    return false;
  }
  if (oat_file_class_status == mirror::Class::kStatusError) {
    // Compile time verification failed with a hard error. This is caused by invalid instructions
    // in the class. These errors are unrecoverable.
    return false;
  }
  if (oat_file_class_status == mirror::Class::kStatusNotReady) {
    // Status is uninitialized if we couldn't determine the status at compile time, for example,
    // not loading the class.
    // TODO: when the verifier doesn't rely on Class-es failing to resolve/load the type hierarchy
    // isn't a problem and this case shouldn't occur
    return false;
  }
  std::string temp;
  LOG(FATAL) << "Unexpected class status: " << oat_file_class_status
             << " " << dex_file.GetLocation() << " " << PrettyClass(klass) << " "
             << klass->GetDescriptor(&temp);
  UNREACHABLE();
}

void ClassLinker::ResolveClassExceptionHandlerTypes(Handle<mirror::Class> klass) {
  for (ArtMethod& method : klass->GetMethods(image_pointer_size_)) {
    ResolveMethodExceptionHandlerTypes(&method);
  }
}

void ClassLinker::ResolveMethodExceptionHandlerTypes(ArtMethod* method) {
  // similar to DexVerifier::ScanTryCatchBlocks and dex2oat's ResolveExceptionsForMethod.
  const DexFile::CodeItem* code_item =
      method->GetDexFile()->GetCodeItem(method->GetCodeItemOffset());
  if (code_item == nullptr) {
    return;  // native or abstract method
  }
  if (code_item->tries_size_ == 0) {
    return;  // nothing to process
  }
  const uint8_t* handlers_ptr = DexFile::GetCatchHandlerData(*code_item, 0);
  uint32_t handlers_size = DecodeUnsignedLeb128(&handlers_ptr);
  for (uint32_t idx = 0; idx < handlers_size; idx++) {
    CatchHandlerIterator iterator(handlers_ptr);
    for (; iterator.HasNext(); iterator.Next()) {
      // Ensure exception types are resolved so that they don't need resolution to be delivered,
      // unresolved exception types will be ignored by exception delivery
      if (iterator.GetHandlerTypeIndex() != DexFile::kDexNoIndex16) {
        mirror::Class* exception_type = ResolveType(iterator.GetHandlerTypeIndex(), method);
        if (exception_type == nullptr) {
          DCHECK(Thread::Current()->IsExceptionPending());
          Thread::Current()->ClearException();
        }
      }
    }
    handlers_ptr = iterator.EndDataPointer();
  }
}

mirror::Class* ClassLinker::CreateProxyClass(ScopedObjectAccessAlreadyRunnable& soa,
                                             jstring name,
                                             jobjectArray interfaces,
                                             jobject loader,
                                             jobjectArray methods,
                                             jobjectArray throws) {
  Thread* self = soa.Self();
  StackHandleScope<10> hs(self);
  MutableHandle<mirror::Class> klass(hs.NewHandle(
      AllocClass(self, GetClassRoot(kJavaLangClass), sizeof(mirror::Class))));
  if (klass.Get() == nullptr) {
    CHECK(self->IsExceptionPending());  // OOME.
    return nullptr;
  }
  DCHECK(klass->GetClass() != nullptr);
  klass->SetObjectSize(sizeof(mirror::Proxy));
  // Set the class access flags incl. VerificationAttempted, so we do not try to set the flag on
  // the methods.
  klass->SetAccessFlags(kAccClassIsProxy | kAccPublic | kAccFinal | kAccVerificationAttempted);
  klass->SetClassLoader(soa.Decode<mirror::ClassLoader*>(loader));
  DCHECK_EQ(klass->GetPrimitiveType(), Primitive::kPrimNot);
  klass->SetName(soa.Decode<mirror::String*>(name));
  klass->SetDexCache(GetClassRoot(kJavaLangReflectProxy)->GetDexCache());
  mirror::Class::SetStatus(klass, mirror::Class::kStatusIdx, self);
  std::string descriptor(GetDescriptorForProxy(klass.Get()));
  const size_t hash = ComputeModifiedUtf8Hash(descriptor.c_str());

  // Needs to be before we insert the class so that the allocator field is set.
  LinearAlloc* const allocator = GetOrCreateAllocatorForClassLoader(klass->GetClassLoader());

  // Insert the class before loading the fields as the field roots
  // (ArtField::declaring_class_) are only visited from the class
  // table. There can't be any suspend points between inserting the
  // class and setting the field arrays below.
  mirror::Class* existing = InsertClass(descriptor.c_str(), klass.Get(), hash);
  CHECK(existing == nullptr);

  // Instance fields are inherited, but we add a couple of static fields...
  const size_t num_fields = 2;
  LengthPrefixedArray<ArtField>* sfields = AllocArtFieldArray(self, allocator, num_fields);
  klass->SetSFieldsPtr(sfields);

  // 1. Create a static field 'interfaces' that holds the _declared_ interfaces implemented by
  // our proxy, so Class.getInterfaces doesn't return the flattened set.
  ArtField& interfaces_sfield = sfields->At(0);
  interfaces_sfield.SetDexFieldIndex(0);
  interfaces_sfield.SetDeclaringClass(klass.Get());
  interfaces_sfield.SetAccessFlags(kAccStatic | kAccPublic | kAccFinal);

  // 2. Create a static field 'throws' that holds exceptions thrown by our methods.
  ArtField& throws_sfield = sfields->At(1);
  throws_sfield.SetDexFieldIndex(1);
  throws_sfield.SetDeclaringClass(klass.Get());
  throws_sfield.SetAccessFlags(kAccStatic | kAccPublic | kAccFinal);

  // Proxies have 1 direct method, the constructor
  const size_t num_direct_methods = 1;

  // They have as many virtual methods as the array
  auto h_methods = hs.NewHandle(soa.Decode<mirror::ObjectArray<mirror::Method>*>(methods));
  DCHECK_EQ(h_methods->GetClass(), mirror::Method::ArrayClass())
      << PrettyClass(h_methods->GetClass());
  const size_t num_virtual_methods = h_methods->GetLength();

  // Create the methods array.
  LengthPrefixedArray<ArtMethod>* proxy_class_methods = AllocArtMethodArray(
        self, allocator, num_direct_methods + num_virtual_methods);
  // Currently AllocArtMethodArray cannot return null, but the OOM logic is left there in case we
  // want to throw OOM in the future.
  if (UNLIKELY(proxy_class_methods == nullptr)) {
    self->AssertPendingOOMException();
    return nullptr;
  }
  klass->SetMethodsPtr(proxy_class_methods, num_direct_methods, num_virtual_methods);

  // Create the single direct method.
  CreateProxyConstructor(klass, klass->GetDirectMethodUnchecked(0, image_pointer_size_));

  // Create virtual method using specified prototypes.
  // TODO These should really use the iterators.
  for (size_t i = 0; i < num_virtual_methods; ++i) {
    auto* virtual_method = klass->GetVirtualMethodUnchecked(i, image_pointer_size_);
    auto* prototype = h_methods->Get(i)->GetArtMethod();
    CreateProxyMethod(klass, prototype, virtual_method);
    DCHECK(virtual_method->GetDeclaringClass() != nullptr);
    DCHECK(prototype->GetDeclaringClass() != nullptr);
  }

  // The super class is java.lang.reflect.Proxy
  klass->SetSuperClass(GetClassRoot(kJavaLangReflectProxy));
  if (Runtime::Current()->UseCHA()) {
    mirror::Class* parent = GetClassRoot(kJavaLangReflectProxy);
    mirror::Class* child = klass.Get();
    AddCHA(parent, child);
  }
  // Now effectively in the loaded state.
  mirror::Class::SetStatus(klass, mirror::Class::kStatusLoaded, self);
  self->AssertNoPendingException();

  MutableHandle<mirror::Class> new_class = hs.NewHandle<mirror::Class>(nullptr);
  {
    // Must hold lock on object when resolved.
    ObjectLock<mirror::Class> resolution_lock(self, klass);
    // Link the fields and virtual methods, creating vtable and iftables.
    // The new class will replace the old one in the class table.
    Handle<mirror::ObjectArray<mirror::Class>> h_interfaces(
        hs.NewHandle(soa.Decode<mirror::ObjectArray<mirror::Class>*>(interfaces)));
    if (!LinkClass(self, descriptor.c_str(), klass, h_interfaces, &new_class)) {
      mirror::Class::SetStatus(klass, mirror::Class::kStatusError, self);
      return nullptr;
    }
  }
  CHECK(klass->IsRetired());
  CHECK_NE(klass.Get(), new_class.Get());
  klass.Assign(new_class.Get());

  CHECK_EQ(interfaces_sfield.GetDeclaringClass(), klass.Get());
  interfaces_sfield.SetObject<false>(klass.Get(),
                                     soa.Decode<mirror::ObjectArray<mirror::Class>*>(interfaces));
  CHECK_EQ(throws_sfield.GetDeclaringClass(), klass.Get());
  throws_sfield.SetObject<false>(
      klass.Get(), soa.Decode<mirror::ObjectArray<mirror::ObjectArray<mirror::Class> >*>(throws));

  {
    // Lock on klass is released. Lock new class object.
    ObjectLock<mirror::Class> initialization_lock(self, klass);
    mirror::Class::SetStatus(klass, mirror::Class::kStatusInitialized, self);
  }

  // sanity checks
  if (kIsDebugBuild) {
    CHECK(klass->GetIFieldsPtr() == nullptr);
    CheckProxyConstructor(klass->GetDirectMethod(0, image_pointer_size_));

    for (size_t i = 0; i < num_virtual_methods; ++i) {
      auto* virtual_method = klass->GetVirtualMethodUnchecked(i, image_pointer_size_);
      auto* prototype = h_methods->Get(i++)->GetArtMethod();
      CheckProxyMethod(virtual_method, prototype);
    }

    StackHandleScope<1> hs2(self);
    Handle<mirror::String> decoded_name = hs2.NewHandle(soa.Decode<mirror::String*>(name));
    std::string interfaces_field_name(StringPrintf("java.lang.Class[] %s.interfaces",
                                                   decoded_name->ToModifiedUtf8().c_str()));
    CHECK_EQ(PrettyField(klass->GetStaticField(0)), interfaces_field_name);

    std::string throws_field_name(StringPrintf("java.lang.Class[][] %s.throws",
                                               decoded_name->ToModifiedUtf8().c_str()));
    CHECK_EQ(PrettyField(klass->GetStaticField(1)), throws_field_name);

    CHECK_EQ(klass.Get()->GetInterfaces(),
             soa.Decode<mirror::ObjectArray<mirror::Class>*>(interfaces));
    CHECK_EQ(klass.Get()->GetThrows(),
             soa.Decode<mirror::ObjectArray<mirror::ObjectArray<mirror::Class>>*>(throws));
  }
  return klass.Get();
}

std::string ClassLinker::GetDescriptorForProxy(mirror::Class* proxy_class) {
  DCHECK(proxy_class->IsProxyClass());
  mirror::String* name = proxy_class->GetName();
  DCHECK(name != nullptr);
  return DotToDescriptor(name->ToModifiedUtf8().c_str());
}

ArtMethod* ClassLinker::FindMethodForProxy(mirror::Class* proxy_class, ArtMethod* proxy_method) {
  DCHECK(proxy_class->IsProxyClass());
  DCHECK(proxy_method->IsProxyMethod());
  {
    Thread* const self = Thread::Current();
    ReaderMutexLock mu(self, dex_lock_);
    // Locate the dex cache of the original interface/Object
    for (const DexCacheData& data : dex_caches_) {
      if (!self->IsJWeakCleared(data.weak_root) &&
          proxy_method->HasSameDexCacheResolvedTypes(data.resolved_types,
                                                     image_pointer_size_)) {
        mirror::DexCache* dex_cache = down_cast<mirror::DexCache*>(
            self->DecodeJObject(data.weak_root));
        if (dex_cache != nullptr) {
          ArtMethod* resolved_method = dex_cache->GetResolvedMethod(
              proxy_method->GetDexMethodIndex(), image_pointer_size_);
          CHECK(resolved_method != nullptr);
          return resolved_method;
        }
      }
    }
  }
  LOG(FATAL) << "Didn't find dex cache for " << PrettyClass(proxy_class) << " "
      << PrettyMethod(proxy_method);
  UNREACHABLE();
}

void ClassLinker::CreateProxyConstructor(Handle<mirror::Class> klass, ArtMethod* out) {
  // Create constructor for Proxy that must initialize the method.
  CHECK_EQ(GetClassRoot(kJavaLangReflectProxy)->NumDirectMethods(), 18u);
  ArtMethod* proxy_constructor = GetClassRoot(kJavaLangReflectProxy)->GetDirectMethodUnchecked(
      2, image_pointer_size_);
  DCHECK_EQ(std::string(proxy_constructor->GetName()), "<init>");
  // Ensure constructor is in dex cache so that we can use the dex cache to look up the overridden
  // constructor method.
  GetClassRoot(kJavaLangReflectProxy)->GetDexCache()->SetResolvedMethod(
      proxy_constructor->GetDexMethodIndex(), proxy_constructor, image_pointer_size_);
  // Clone the existing constructor of Proxy (our constructor would just invoke it so steal its
  // code_ too)
  DCHECK(out != nullptr);
  out->CopyFrom(proxy_constructor, image_pointer_size_);
  // Make this constructor public and fix the class to be our Proxy version
  out->SetAccessFlags((out->GetAccessFlags() & ~kAccProtected) | kAccPublic);
  out->SetDeclaringClass(klass.Get());
}

void ClassLinker::CheckProxyConstructor(ArtMethod* constructor) const {
  CHECK(constructor->IsConstructor());
  auto* np = constructor->GetInterfaceMethodIfProxy(image_pointer_size_);
  CHECK_STREQ(np->GetName(), "<init>");
  CHECK_STREQ(np->GetSignature().ToString().c_str(), "(Ljava/lang/reflect/InvocationHandler;)V");
  DCHECK(constructor->IsPublic());
}

void ClassLinker::CreateProxyMethod(Handle<mirror::Class> klass, ArtMethod* prototype,
                                    ArtMethod* out) {
  // Ensure prototype is in dex cache so that we can use the dex cache to look up the overridden
  // prototype method
  auto* dex_cache = prototype->GetDeclaringClass()->GetDexCache();
  // Avoid dirtying the dex cache unless we need to.
  if (dex_cache->GetResolvedMethod(prototype->GetDexMethodIndex(), image_pointer_size_) !=
      prototype) {
    dex_cache->SetResolvedMethod(
        prototype->GetDexMethodIndex(), prototype, image_pointer_size_);
  }
  // We steal everything from the prototype (such as DexCache, invoke stub, etc.) then specialize
  // as necessary
  DCHECK(out != nullptr);
  out->CopyFrom(prototype, image_pointer_size_);

  // Set class to be the concrete proxy class.
  out->SetDeclaringClass(klass.Get());
  // Clear the abstract, default and conflict flags to ensure that defaults aren't picked in
  // preference to the invocation handler.
  const uint32_t kRemoveFlags = kAccAbstract | kAccDefault | kAccDefaultConflict;
  // Make the method final.
  const uint32_t kAddFlags = kAccFinal;
  out->SetAccessFlags((out->GetAccessFlags() & ~kRemoveFlags) | kAddFlags);

  // Clear the dex_code_item_offset_. It needs to be 0 since proxy methods have no CodeItems but the
  // method they copy might (if it's a default method).
  out->SetCodeItemOffset(0);

  // At runtime the method looks like a reference and argument saving method, clone the code
  // related parameters from this method.
  out->SetEntryPointFromQuickCompiledCode(GetQuickProxyInvokeHandler());
}

void ClassLinker::CheckProxyMethod(ArtMethod* method, ArtMethod* prototype) const {
  // Basic sanity
  CHECK(!prototype->IsFinal());
  CHECK(method->IsFinal());
  CHECK(method->IsInvokable());

  // The proxy method doesn't have its own dex cache or dex file and so it steals those of its
  // interface prototype. The exception to this are Constructors and the Class of the Proxy itself.
  CHECK(prototype->HasSameDexCacheResolvedMethods(method, image_pointer_size_));
  CHECK(prototype->HasSameDexCacheResolvedTypes(method, image_pointer_size_));
  auto* np = method->GetInterfaceMethodIfProxy(image_pointer_size_);
  CHECK_EQ(prototype->GetDeclaringClass()->GetDexCache(), np->GetDexCache());
  CHECK_EQ(prototype->GetDexMethodIndex(), method->GetDexMethodIndex());

  CHECK_STREQ(np->GetName(), prototype->GetName());
  CHECK_STREQ(np->GetShorty(), prototype->GetShorty());
  // More complex sanity - via dex cache
  CHECK_EQ(np->GetReturnType(true /* resolve */, image_pointer_size_),
           prototype->GetReturnType(true /* resolve */, image_pointer_size_));
}

bool ClassLinker::CanWeInitializeClass(mirror::Class* klass, bool can_init_statics,
                                       bool can_init_parents) {
  if (can_init_statics && can_init_parents) {
    return true;
  }
  if (!can_init_statics) {
    // Check if there's a class initializer.
    ArtMethod* clinit = klass->FindClassInitializer(image_pointer_size_);
    if (clinit != nullptr) {
      return false;
    }
    // Check if there are encoded static values needing initialization.
    if (klass->NumStaticFields() != 0) {
      const DexFile::ClassDef* dex_class_def = klass->GetClassDef();
      DCHECK(dex_class_def != nullptr);
      if (dex_class_def->static_values_off_ != 0) {
        return false;
      }
    }
    // If we are a class we need to initialize all interfaces with default methods when we are
    // initialized. Check all of them.
    if (!klass->IsInterface()) {
      size_t num_interfaces = klass->GetIfTableCount();
      for (size_t i = 0; i < num_interfaces; i++) {
        mirror::Class* iface = klass->GetIfTable()->GetInterface(i);
        if (iface->HasDefaultMethods() &&
            !CanWeInitializeClass(iface, can_init_statics, can_init_parents)) {
          return false;
        }
      }
    }
  }
  if (klass->IsInterface() || !klass->HasSuperClass()) {
    return true;
  }
  mirror::Class* super_class = klass->GetSuperClass();
  if (!can_init_parents && !super_class->IsInitialized()) {
    return false;
  }
  return CanWeInitializeClass(super_class, can_init_statics, can_init_parents);
}

bool ClassLinker::InitializeClass(Thread* self, Handle<mirror::Class> klass,
                                  bool can_init_statics, bool can_init_parents) {
  // see JLS 3rd edition, 12.4.2 "Detailed Initialization Procedure" for the locking protocol

  // Are we already initialized and therefore done?
  // Note: we differ from the JLS here as we don't do this under the lock, this is benign as
  // an initialized class will never change its state.
  if (klass->IsInitialized()) {
    return true;
  }

  // Fast fail if initialization requires a full runtime. Not part of the JLS.
  if (!CanWeInitializeClass(klass.Get(), can_init_statics, can_init_parents)) {
    return false;
  }

  self->AllowThreadSuspension();
  uint64_t t0;
  {
    ObjectLock<mirror::Class> lock(self, klass);

    // Re-check under the lock in case another thread initialized ahead of us.
    if (klass->IsInitialized()) {
      return true;
    }

    // Was the class already found to be erroneous? Done under the lock to match the JLS.
    if (klass->IsErroneous()) {
      ThrowEarlierClassFailure(klass.Get(), true);
      VlogClassInitializationFailure(klass);
      return false;
    }

    CHECK(klass->IsResolved()) << PrettyClass(klass.Get()) << ": state=" << klass->GetStatus();

    if (!klass->IsVerified()) {
      VerifyClass(self, klass);
      if (!klass->IsVerified()) {
        // We failed to verify, expect either the klass to be erroneous or verification failed at
        // compile time.
        if (klass->IsErroneous()) {
          // The class is erroneous. This may be a verifier error, or another thread attempted
          // verification and/or initialization and failed. We can distinguish those cases by
          // whether an exception is already pending.
          if (self->IsExceptionPending()) {
            // Check that it's a VerifyError.
            DCHECK_EQ("java.lang.Class<java.lang.VerifyError>",
                      PrettyClass(self->GetException()->GetClass()));
          } else {
            // Check that another thread attempted initialization.
            DCHECK_NE(0, klass->GetClinitThreadId());
            DCHECK_NE(self->GetTid(), klass->GetClinitThreadId());
            // Need to rethrow the previous failure now.
            ThrowEarlierClassFailure(klass.Get(), true);
          }
          VlogClassInitializationFailure(klass);
        } else {
          CHECK(Runtime::Current()->IsAotCompiler());
          CHECK_EQ(klass->GetStatus(), mirror::Class::kStatusRetryVerificationAtRuntime);
        }
        return false;
      } else {
        self->AssertNoPendingException();
      }

      // A separate thread could have moved us all the way to initialized. A "simple" example
      // involves a subclass of the current class being initialized at the same time (which
      // will implicitly initialize the superclass, if scheduled that way). b/28254258
      DCHECK_NE(mirror::Class::kStatusError, klass->GetStatus());
      if (klass->IsInitialized()) {
        return true;
      }
    }

    // If the class is kStatusInitializing, either this thread is
    // initializing higher up the stack or another thread has beat us
    // to initializing and we need to wait. Either way, this
    // invocation of InitializeClass will not be responsible for
    // running <clinit> and will return.
    if (klass->GetStatus() == mirror::Class::kStatusInitializing) {
      // Could have got an exception during verification.
      if (self->IsExceptionPending()) {
        VlogClassInitializationFailure(klass);
        return false;
      }
      // We caught somebody else in the act; was it us?
      if (klass->GetClinitThreadId() == self->GetTid()) {
        // Yes. That's fine. Return so we can continue initializing.
        return true;
      }
      // No. That's fine. Wait for another thread to finish initializing.
      return WaitForInitializeClass(klass, self, lock);
    }

    if (!ValidateSuperClassDescriptors(klass)) {
      mirror::Class::SetStatus(klass, mirror::Class::kStatusError, self);
      return false;
    }
    self->AllowThreadSuspension();

    CHECK_EQ(klass->GetStatus(), mirror::Class::kStatusVerified) << PrettyClass(klass.Get())
        << " self.tid=" << self->GetTid() << " clinit.tid=" << klass->GetClinitThreadId();

    // From here out other threads may observe that we're initializing and so changes of state
    // require the a notification.
    klass->SetClinitThreadId(self->GetTid());
    mirror::Class::SetStatus(klass, mirror::Class::kStatusInitializing, self);

    t0 = NanoTime();
  }

  // Initialize super classes, must be done while initializing for the JLS.
  if (!klass->IsInterface() && klass->HasSuperClass()) {
    mirror::Class* super_class = klass->GetSuperClass();
    if (!super_class->IsInitialized()) {
      CHECK(!super_class->IsInterface());
      CHECK(can_init_parents);
      StackHandleScope<1> hs(self);
      Handle<mirror::Class> handle_scope_super(hs.NewHandle(super_class));
      bool super_initialized = InitializeClass(self, handle_scope_super, can_init_statics, true);
      if (!super_initialized) {
        // The super class was verified ahead of entering initializing, we should only be here if
        // the super class became erroneous due to initialization.
        CHECK(handle_scope_super->IsErroneous() && self->IsExceptionPending())
            << "Super class initialization failed for "
            << PrettyDescriptor(handle_scope_super.Get())
            << " that has unexpected status " << handle_scope_super->GetStatus()
            << "\nPending exception:\n"
            << (self->GetException() != nullptr ? self->GetException()->Dump() : "");
        ObjectLock<mirror::Class> lock(self, klass);
        // Initialization failed because the super-class is erroneous.
        mirror::Class::SetStatus(klass, mirror::Class::kStatusError, self);
        return false;
      }
    }
  }

  if (!klass->IsInterface()) {
    // Initialize interfaces with default methods for the JLS.
    size_t num_direct_interfaces = klass->NumDirectInterfaces();
    // Only setup the (expensive) handle scope if we actually need to.
    if (UNLIKELY(num_direct_interfaces > 0)) {
      StackHandleScope<1> hs_iface(self);
      MutableHandle<mirror::Class> handle_scope_iface(hs_iface.NewHandle<mirror::Class>(nullptr));
      for (size_t i = 0; i < num_direct_interfaces; i++) {
        handle_scope_iface.Assign(mirror::Class::GetDirectInterface(self, klass, i));
        CHECK(handle_scope_iface.Get() != nullptr);
        CHECK(handle_scope_iface->IsInterface());
        if (handle_scope_iface->HasBeenRecursivelyInitialized()) {
          // We have already done this for this interface. Skip it.
          continue;
        }
        // We cannot just call initialize class directly because we need to ensure that ALL
        // interfaces with default methods are initialized. Non-default interface initialization
        // will not affect other non-default super-interfaces.
        bool iface_initialized = InitializeDefaultInterfaceRecursive(self,
                                                                     handle_scope_iface,
                                                                     can_init_statics,
                                                                     can_init_parents);
        if (!iface_initialized) {
          ObjectLock<mirror::Class> lock(self, klass);
          // Initialization failed because one of our interfaces with default methods is erroneous.
          mirror::Class::SetStatus(klass, mirror::Class::kStatusError, self);
          return false;
        }
      }
    }
  }

  const size_t num_static_fields = klass->NumStaticFields();
  if (num_static_fields > 0) {
    const DexFile::ClassDef* dex_class_def = klass->GetClassDef();
    CHECK(dex_class_def != nullptr);
    const DexFile& dex_file = klass->GetDexFile();
    StackHandleScope<3> hs(self);
    Handle<mirror::ClassLoader> class_loader(hs.NewHandle(klass->GetClassLoader()));
    Handle<mirror::DexCache> dex_cache(hs.NewHandle(klass->GetDexCache()));

    // Eagerly fill in static fields so that the we don't have to do as many expensive
    // Class::FindStaticField in ResolveField.
    for (size_t i = 0; i < num_static_fields; ++i) {
      ArtField* field = klass->GetStaticField(i);
      const uint32_t field_idx = field->GetDexFieldIndex();
      ArtField* resolved_field = dex_cache->GetResolvedField(field_idx, image_pointer_size_);
      if (resolved_field == nullptr) {
        dex_cache->SetResolvedField(field_idx, field, image_pointer_size_);
      } else {
        DCHECK_EQ(field, resolved_field);
      }
    }

    EncodedStaticFieldValueIterator value_it(dex_file, &dex_cache, &class_loader,
                                             this, *dex_class_def);
    const uint8_t* class_data = dex_file.GetClassData(*dex_class_def);
    ClassDataItemIterator field_it(dex_file, class_data);
    if (value_it.HasNext()) {
      DCHECK(field_it.HasNextStaticField());
      CHECK(can_init_statics);
      for ( ; value_it.HasNext(); value_it.Next(), field_it.Next()) {
        ArtField* field = ResolveField(
            dex_file, field_it.GetMemberIndex(), dex_cache, class_loader, true);
        if (Runtime::Current()->IsActiveTransaction()) {
          value_it.ReadValueToField<true>(field);
        } else {
          value_it.ReadValueToField<false>(field);
        }
        DCHECK(!value_it.HasNext() || field_it.HasNextStaticField());
      }
    }
  }

  ArtMethod* clinit = klass->FindClassInitializer(image_pointer_size_);
  if (clinit != nullptr) {
    CHECK(can_init_statics);
    JValue result;
    clinit->Invoke(self, nullptr, 0, &result, "V");
  }

  self->AllowThreadSuspension();
  uint64_t t1 = NanoTime();

  bool success = true;
  {
    ObjectLock<mirror::Class> lock(self, klass);

    if (self->IsExceptionPending()) {
      WrapExceptionInInitializer(klass);
      mirror::Class::SetStatus(klass, mirror::Class::kStatusError, self);
      success = false;
    } else if (Runtime::Current()->IsTransactionAborted()) {
      // The exception thrown when the transaction aborted has been caught and cleared
      // so we need to throw it again now.
      VLOG(compiler) << "Return from class initializer of " << PrettyDescriptor(klass.Get())
                     << " without exception while transaction was aborted: re-throw it now.";
      Runtime::Current()->ThrowTransactionAbortError(self);
      mirror::Class::SetStatus(klass, mirror::Class::kStatusError, self);
      success = false;
    } else {
      RuntimeStats* global_stats = Runtime::Current()->GetStats();
      RuntimeStats* thread_stats = self->GetStats();
      ++global_stats->class_init_count;
      ++thread_stats->class_init_count;
      global_stats->class_init_time_ns += (t1 - t0);
      thread_stats->class_init_time_ns += (t1 - t0);
      // Set the class as initialized except if failed to initialize static fields.
      mirror::Class::SetStatus(klass, mirror::Class::kStatusInitialized, self);
      if (VLOG_IS_ON(class_linker)) {
        std::string temp;
        LOG(INFO) << "Initialized class " << klass->GetDescriptor(&temp) << " from " <<
            klass->GetLocation();
      }
      // Opportunistically set static method trampolines to their destination.
      FixupStaticTrampolines(klass.Get());
    }
  }
  return success;
}

// We recursively run down the tree of interfaces. We need to do this in the order they are declared
// and perform the initialization only on those interfaces that contain default methods.
bool ClassLinker::InitializeDefaultInterfaceRecursive(Thread* self,
                                                      Handle<mirror::Class> iface,
                                                      bool can_init_statics,
                                                      bool can_init_parents) {
  CHECK(iface->IsInterface());
  size_t num_direct_ifaces = iface->NumDirectInterfaces();
  // Only create the (expensive) handle scope if we need it.
  if (UNLIKELY(num_direct_ifaces > 0)) {
    StackHandleScope<1> hs(self);
    MutableHandle<mirror::Class> handle_super_iface(hs.NewHandle<mirror::Class>(nullptr));
    // First we initialize all of iface's super-interfaces recursively.
    for (size_t i = 0; i < num_direct_ifaces; i++) {
      mirror::Class* super_iface = mirror::Class::GetDirectInterface(self, iface, i);
      if (!super_iface->HasBeenRecursivelyInitialized()) {
        // Recursive step
        handle_super_iface.Assign(super_iface);
        if (!InitializeDefaultInterfaceRecursive(self,
                                                 handle_super_iface,
                                                 can_init_statics,
                                                 can_init_parents)) {
          return false;
        }
      }
    }
  }

  bool result = true;
  // Then we initialize 'iface' if it has default methods. We do not need to (and in fact must not)
  // initialize if we don't have default methods.
  if (iface->HasDefaultMethods()) {
    result = EnsureInitialized(self, iface, can_init_statics, can_init_parents);
  }

  // Mark that this interface has undergone recursive default interface initialization so we know we
  // can skip it on any later class initializations. We do this even if we are not a default
  // interface since we can still avoid the traversal. This is purely a performance optimization.
  if (result) {
    // TODO This should be done in a better way
    ObjectLock<mirror::Class> lock(self, iface);
    iface->SetRecursivelyInitialized();
  }
  return result;
}

bool ClassLinker::WaitForInitializeClass(Handle<mirror::Class> klass,
                                         Thread* self,
                                         ObjectLock<mirror::Class>& lock)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  while (true) {
    self->AssertNoPendingException();
    CHECK(!klass->IsInitialized());
    lock.WaitIgnoringInterrupts();

    // When we wake up, repeat the test for init-in-progress.  If
    // there's an exception pending (only possible if
    // we were not using WaitIgnoringInterrupts), bail out.
    if (self->IsExceptionPending()) {
      WrapExceptionInInitializer(klass);
      mirror::Class::SetStatus(klass, mirror::Class::kStatusError, self);
      return false;
    }
    // Spurious wakeup? Go back to waiting.
    if (klass->GetStatus() == mirror::Class::kStatusInitializing) {
      continue;
    }
    if (klass->GetStatus() == mirror::Class::kStatusVerified &&
        Runtime::Current()->IsAotCompiler()) {
      // Compile time initialization failed.
      return false;
    }
    if (klass->IsErroneous()) {
      // The caller wants an exception, but it was thrown in a
      // different thread.  Synthesize one here.
      ThrowNoClassDefFoundError("<clinit> failed for class %s; see exception in other thread",
                                PrettyDescriptor(klass.Get()).c_str());
      VlogClassInitializationFailure(klass);
      return false;
    }
    if (klass->IsInitialized()) {
      return true;
    }
    LOG(FATAL) << "Unexpected class status. " << PrettyClass(klass.Get()) << " is "
        << klass->GetStatus();
  }
  UNREACHABLE();
}

static void ThrowSignatureCheckResolveReturnTypeException(Handle<mirror::Class> klass,
                                                          Handle<mirror::Class> super_klass,
                                                          ArtMethod* method,
                                                          ArtMethod* m)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  DCHECK(Thread::Current()->IsExceptionPending());
  DCHECK(!m->IsProxyMethod());
  const DexFile* dex_file = m->GetDexFile();
  const DexFile::MethodId& method_id = dex_file->GetMethodId(m->GetDexMethodIndex());
  const DexFile::ProtoId& proto_id = dex_file->GetMethodPrototype(method_id);
  uint16_t return_type_idx = proto_id.return_type_idx_;
  std::string return_type = PrettyType(return_type_idx, *dex_file);
  std::string class_loader = PrettyTypeOf(m->GetDeclaringClass()->GetClassLoader());
  ThrowWrappedLinkageError(klass.Get(),
                           "While checking class %s method %s signature against %s %s: "
                           "Failed to resolve return type %s with %s",
                           PrettyDescriptor(klass.Get()).c_str(),
                           PrettyMethod(method).c_str(),
                           super_klass->IsInterface() ? "interface" : "superclass",
                           PrettyDescriptor(super_klass.Get()).c_str(),
                           return_type.c_str(), class_loader.c_str());
}

static void ThrowSignatureCheckResolveArgException(Handle<mirror::Class> klass,
                                                   Handle<mirror::Class> super_klass,
                                                   ArtMethod* method,
                                                   ArtMethod* m,
                                                   uint32_t index,
                                                   uint32_t arg_type_idx)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  DCHECK(Thread::Current()->IsExceptionPending());
  DCHECK(!m->IsProxyMethod());
  const DexFile* dex_file = m->GetDexFile();
  std::string arg_type = PrettyType(arg_type_idx, *dex_file);
  std::string class_loader = PrettyTypeOf(m->GetDeclaringClass()->GetClassLoader());
  ThrowWrappedLinkageError(klass.Get(),
                           "While checking class %s method %s signature against %s %s: "
                           "Failed to resolve arg %u type %s with %s",
                           PrettyDescriptor(klass.Get()).c_str(),
                           PrettyMethod(method).c_str(),
                           super_klass->IsInterface() ? "interface" : "superclass",
                           PrettyDescriptor(super_klass.Get()).c_str(),
                           index, arg_type.c_str(), class_loader.c_str());
}

static void ThrowSignatureMismatch(Handle<mirror::Class> klass,
                                   Handle<mirror::Class> super_klass,
                                   ArtMethod* method,
                                   const std::string& error_msg)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  ThrowLinkageError(klass.Get(),
                    "Class %s method %s resolves differently in %s %s: %s",
                    PrettyDescriptor(klass.Get()).c_str(),
                    PrettyMethod(method).c_str(),
                    super_klass->IsInterface() ? "interface" : "superclass",
                    PrettyDescriptor(super_klass.Get()).c_str(),
                    error_msg.c_str());
}

static bool HasSameSignatureWithDifferentClassLoaders(Thread* self,
                                                      size_t pointer_size,
                                                      Handle<mirror::Class> klass,
                                                      Handle<mirror::Class> super_klass,
                                                      ArtMethod* method1,
                                                      ArtMethod* method2)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  {
    StackHandleScope<1> hs(self);
    Handle<mirror::Class> return_type(hs.NewHandle(method1->GetReturnType(true /* resolve */,
                                                                          pointer_size)));
    if (UNLIKELY(return_type.Get() == nullptr)) {
      ThrowSignatureCheckResolveReturnTypeException(klass, super_klass, method1, method1);
      return false;
    }
    mirror::Class* other_return_type = method2->GetReturnType(true /* resolve */,
                                                              pointer_size);
    if (UNLIKELY(other_return_type == nullptr)) {
      ThrowSignatureCheckResolveReturnTypeException(klass, super_klass, method1, method2);
      return false;
    }
    if (UNLIKELY(other_return_type != return_type.Get())) {
      ThrowSignatureMismatch(klass, super_klass, method1,
                             StringPrintf("Return types mismatch: %s(%p) vs %s(%p)",
                                          PrettyClassAndClassLoader(return_type.Get()).c_str(),
                                          return_type.Get(),
                                          PrettyClassAndClassLoader(other_return_type).c_str(),
                                          other_return_type));
      return false;
    }
  }
  const DexFile::TypeList* types1 = method1->GetParameterTypeList();
  const DexFile::TypeList* types2 = method2->GetParameterTypeList();
  if (types1 == nullptr) {
    if (types2 != nullptr && types2->Size() != 0) {
      ThrowSignatureMismatch(klass, super_klass, method1,
                             StringPrintf("Type list mismatch with %s",
                                          PrettyMethod(method2, true).c_str()));
      return false;
    }
    return true;
  } else if (UNLIKELY(types2 == nullptr)) {
    if (types1->Size() != 0) {
      ThrowSignatureMismatch(klass, super_klass, method1,
                             StringPrintf("Type list mismatch with %s",
                                          PrettyMethod(method2, true).c_str()));
      return false;
    }
    return true;
  }
  uint32_t num_types = types1->Size();
  if (UNLIKELY(num_types != types2->Size())) {
    ThrowSignatureMismatch(klass, super_klass, method1,
                           StringPrintf("Type list mismatch with %s",
                                        PrettyMethod(method2, true).c_str()));
    return false;
  }
  for (uint32_t i = 0; i < num_types; ++i) {
    StackHandleScope<1> hs(self);
    uint32_t param_type_idx = types1->GetTypeItem(i).type_idx_;
    Handle<mirror::Class> param_type(hs.NewHandle(
        method1->GetClassFromTypeIndex(param_type_idx, true /* resolve */, pointer_size)));
    if (UNLIKELY(param_type.Get() == nullptr)) {
      ThrowSignatureCheckResolveArgException(klass, super_klass, method1,
                                             method1, i, param_type_idx);
      return false;
    }
    uint32_t other_param_type_idx = types2->GetTypeItem(i).type_idx_;
    mirror::Class* other_param_type =
        method2->GetClassFromTypeIndex(other_param_type_idx, true /* resolve */, pointer_size);
    if (UNLIKELY(other_param_type == nullptr)) {
      ThrowSignatureCheckResolveArgException(klass, super_klass, method1,
                                             method2, i, other_param_type_idx);
      return false;
    }
    if (UNLIKELY(param_type.Get() != other_param_type)) {
      ThrowSignatureMismatch(klass, super_klass, method1,
                             StringPrintf("Parameter %u type mismatch: %s(%p) vs %s(%p)",
                                          i,
                                          PrettyClassAndClassLoader(param_type.Get()).c_str(),
                                          param_type.Get(),
                                          PrettyClassAndClassLoader(other_param_type).c_str(),
                                          other_param_type));
      return false;
    }
  }
  return true;
}


bool ClassLinker::ValidateSuperClassDescriptors(Handle<mirror::Class> klass) {
  if (klass->IsInterface()) {
    return true;
  }
  // Begin with the methods local to the superclass.
  Thread* self = Thread::Current();
  StackHandleScope<1> hs(self);
  MutableHandle<mirror::Class> super_klass(hs.NewHandle<mirror::Class>(nullptr));
  if (klass->HasSuperClass() &&
      klass->GetClassLoader() != klass->GetSuperClass()->GetClassLoader()) {
    super_klass.Assign(klass->GetSuperClass());
    for (int i = klass->GetSuperClass()->GetVTableLength() - 1; i >= 0; --i) {
      auto* m = klass->GetVTableEntry(i, image_pointer_size_);
      auto* super_m = klass->GetSuperClass()->GetVTableEntry(i, image_pointer_size_);
      if (m != super_m) {
        if (UNLIKELY(!HasSameSignatureWithDifferentClassLoaders(self, image_pointer_size_,
                                                                klass, super_klass,
                                                                m, super_m))) {
          self->AssertPendingException();
          return false;
        }
      }
    }
  }
  for (int32_t i = 0; i < klass->GetIfTableCount(); ++i) {
    super_klass.Assign(klass->GetIfTable()->GetInterface(i));
    if (klass->GetClassLoader() != super_klass->GetClassLoader()) {
      uint32_t num_methods = super_klass->NumVirtualMethods();
      for (uint32_t j = 0; j < num_methods; ++j) {
        auto* m = klass->GetIfTable()->GetMethodArray(i)->GetElementPtrSize<ArtMethod*>(
            j, image_pointer_size_);
        auto* super_m = super_klass->GetVirtualMethod(j, image_pointer_size_);
        if (m != super_m) {
          if (UNLIKELY(!HasSameSignatureWithDifferentClassLoaders(self, image_pointer_size_,
                                                                  klass, super_klass,
                                                                  m, super_m))) {
            self->AssertPendingException();
            return false;
          }
        }
      }
    }
  }
  return true;
}

bool ClassLinker::EnsureInitialized(Thread* self, Handle<mirror::Class> c, bool can_init_fields,
                                    bool can_init_parents) {
  DCHECK(c.Get() != nullptr);
  if (c->IsInitialized()) {
    EnsureSkipAccessChecksMethods(c);
    return true;
  }
  const bool success = InitializeClass(self, c, can_init_fields, can_init_parents);
  if (!success) {
    if (can_init_fields && can_init_parents) {
      CHECK(self->IsExceptionPending()) << PrettyClass(c.Get());
    }
  } else {
    self->AssertNoPendingException();
  }
  return success;
}

void ClassLinker::FixupTemporaryDeclaringClass(mirror::Class* temp_class,
                                               mirror::Class* new_class) {
  DCHECK_EQ(temp_class->NumInstanceFields(), 0u);
  for (ArtField& field : new_class->GetIFields()) {
    if (field.GetDeclaringClass() == temp_class) {
      field.SetDeclaringClass(new_class);
    }
  }

  DCHECK_EQ(temp_class->NumStaticFields(), 0u);
  for (ArtField& field : new_class->GetSFields()) {
    if (field.GetDeclaringClass() == temp_class) {
      field.SetDeclaringClass(new_class);
    }
  }

  DCHECK_EQ(temp_class->NumDirectMethods(), 0u);
  DCHECK_EQ(temp_class->NumVirtualMethods(), 0u);
  for (auto& method : new_class->GetMethods(image_pointer_size_)) {
    if (method.GetDeclaringClass() == temp_class) {
      method.SetDeclaringClass(new_class);
    }
  }

  // Make sure the remembered set and mod-union tables know that we updated some of the native
  // roots.
  Runtime::Current()->GetHeap()->WriteBarrierEveryFieldOf(new_class);
}

void ClassLinker::RegisterClassLoader(mirror::ClassLoader* class_loader) {
  CHECK(class_loader->GetAllocator() == nullptr);
  CHECK(class_loader->GetClassTable() == nullptr);
  Thread* const self = Thread::Current();
  ClassLoaderData data;
  data.weak_root = self->GetJniEnv()->vm->AddWeakGlobalRef(self, class_loader);
  // Create and set the class table.
  data.class_table = new ClassTable;
  class_loader->SetClassTable(data.class_table);
  // Create and set the linear allocator.
  data.allocator = Runtime::Current()->CreateLinearAlloc();
  class_loader->SetAllocator(data.allocator);
  // Add to the list so that we know to free the data later.
  class_loaders_.push_back(data);
}

ClassTable* ClassLinker::InsertClassTableForClassLoader(mirror::ClassLoader* class_loader) {
  if (class_loader == nullptr) {
    return &boot_class_table_;
  }
  ClassTable* class_table = class_loader->GetClassTable();
  if (class_table == nullptr) {
    RegisterClassLoader(class_loader);
    class_table = class_loader->GetClassTable();
    DCHECK(class_table != nullptr);
  }
  return class_table;
}

ClassTable* ClassLinker::ClassTableForClassLoader(mirror::ClassLoader* class_loader) {
  return class_loader == nullptr ? &boot_class_table_ : class_loader->GetClassTable();
}

bool ClassLinker::LinkClass(Thread* self,
                            const char* descriptor,
                            Handle<mirror::Class> klass,
                            Handle<mirror::ObjectArray<mirror::Class>> interfaces,
                            MutableHandle<mirror::Class>* h_new_class_out) {
  CHECK_EQ(mirror::Class::kStatusLoaded, klass->GetStatus());

  if (!LinkSuperClass(klass)) {
    return false;
  }
  ArtMethod* imt[mirror::Class::kImtSize];
  std::fill_n(imt, arraysize(imt), Runtime::Current()->GetImtUnimplementedMethod());
  if (!LinkMethods(self, klass, interfaces, imt)) {
    return false;
  }
  if (!LinkInstanceFields(self, klass)) {
    return false;
  }
  size_t class_size;
  if (!LinkStaticFields(self, klass, &class_size)) {
    return false;
  }
  CreateReferenceInstanceOffsets(klass);
  CHECK_EQ(mirror::Class::kStatusLoaded, klass->GetStatus());

  if (!klass->IsTemp() || (!init_done_ && klass->GetClassSize() == class_size)) {
    // We don't need to retire this class as it has no embedded tables or it was created the
    // correct size during class linker initialization.
    CHECK_EQ(klass->GetClassSize(), class_size) << PrettyDescriptor(klass.Get());

    if (klass->ShouldHaveEmbeddedImtAndVTable()) {
      klass->PopulateEmbeddedImtAndVTable(imt, image_pointer_size_);
    }

    // This will notify waiters on klass that saw the not yet resolved
    // class in the class_table_ during EnsureResolved.
    mirror::Class::SetStatus(klass, mirror::Class::kStatusResolved, self);
    h_new_class_out->Assign(klass.Get());
  } else {
    CHECK(!klass->IsResolved());
    // Retire the temporary class and create the correctly sized resolved class.
    StackHandleScope<1> hs(self);
    auto h_new_class = hs.NewHandle(klass->CopyOf(self, class_size, imt, image_pointer_size_));
    // Set arrays to null since we don't want to have multiple classes with the same ArtField or
    // ArtMethod array pointers. If this occurs, it causes bugs in remembered sets since the GC
    // may not see any references to the target space and clean the card for a class if another
    // class had the same array pointer.
    klass->SetMethodsPtrUnchecked(nullptr, 0, 0);
    klass->SetSFieldsPtrUnchecked(nullptr);
    klass->SetIFieldsPtrUnchecked(nullptr);
    if (UNLIKELY(h_new_class.Get() == nullptr)) {
      self->AssertPendingOOMException();
      mirror::Class::SetStatus(klass, mirror::Class::kStatusError, self);
      return false;
    }

    CHECK_EQ(h_new_class->GetClassSize(), class_size);
    ObjectLock<mirror::Class> lock(self, h_new_class);
    FixupTemporaryDeclaringClass(klass.Get(), h_new_class.Get());

    {
      WriterMutexLock mu(self, *Locks::classlinker_classes_lock_);
      mirror::ClassLoader* const class_loader = h_new_class.Get()->GetClassLoader();
      ClassTable* const table = InsertClassTableForClassLoader(class_loader);
      mirror::Class* existing = table->UpdateClass(descriptor, h_new_class.Get(),
                                                   ComputeModifiedUtf8Hash(descriptor));
      if (class_loader != nullptr) {
        // We updated the class in the class table, perform the write barrier so that the GC knows
        // about the change.
        Runtime::Current()->GetHeap()->WriteBarrierEveryFieldOf(class_loader);
      }
      CHECK_EQ(existing, klass.Get());
      if (kIsDebugBuild && class_loader == nullptr && dex_cache_boot_image_class_lookup_required_) {
        // Check a class loaded with the system class loader matches one in the image if the class
        // is in the image.
        mirror::Class* const image_class = LookupClassFromBootImage(descriptor);
        if (image_class != nullptr) {
          CHECK_EQ(klass.Get(), existing) << descriptor;
        }
      }
      if (log_new_class_table_roots_) {
        new_class_roots_.push_back(GcRoot<mirror::Class>(h_new_class.Get()));
      }
    }

    // This will notify waiters on temp class that saw the not yet resolved class in the
    // class_table_ during EnsureResolved.
    mirror::Class::SetStatus(klass, mirror::Class::kStatusRetired, self);

    CHECK_EQ(h_new_class->GetStatus(), mirror::Class::kStatusResolving);
    // This will notify waiters on new_class that saw the not yet resolved
    // class in the class_table_ during EnsureResolved.
    mirror::Class::SetStatus(h_new_class, mirror::Class::kStatusResolved, self);
    // Return the new class.
    h_new_class_out->Assign(h_new_class.Get());
  }
  return true;
}

static void CountMethodsAndFields(ClassDataItemIterator& dex_data,
                                  size_t* virtual_methods,
                                  size_t* direct_methods,
                                  size_t* static_fields,
                                  size_t* instance_fields) {
  *virtual_methods = *direct_methods = *static_fields = *instance_fields = 0;

  while (dex_data.HasNextStaticField()) {
    dex_data.Next();
    (*static_fields)++;
  }
  while (dex_data.HasNextInstanceField()) {
    dex_data.Next();
    (*instance_fields)++;
  }
  while (dex_data.HasNextDirectMethod()) {
    (*direct_methods)++;
    dex_data.Next();
  }
  while (dex_data.HasNextVirtualMethod()) {
    (*virtual_methods)++;
    dex_data.Next();
  }
  DCHECK(!dex_data.HasNext());
}

static void DumpClass(std::ostream& os,
                      const DexFile& dex_file, const DexFile::ClassDef& dex_class_def,
                      const char* suffix) {
  ClassDataItemIterator dex_data(dex_file, dex_file.GetClassData(dex_class_def));
  os << dex_file.GetClassDescriptor(dex_class_def) << suffix << ":\n";
  os << " Static fields:\n";
  while (dex_data.HasNextStaticField()) {
    const DexFile::FieldId& id = dex_file.GetFieldId(dex_data.GetMemberIndex());
    os << "  " << dex_file.GetFieldTypeDescriptor(id) << " " << dex_file.GetFieldName(id) << "\n";
    dex_data.Next();
  }
  os << " Instance fields:\n";
  while (dex_data.HasNextInstanceField()) {
    const DexFile::FieldId& id = dex_file.GetFieldId(dex_data.GetMemberIndex());
    os << "  " << dex_file.GetFieldTypeDescriptor(id) << " " << dex_file.GetFieldName(id) << "\n";
    dex_data.Next();
  }
  os << " Direct methods:\n";
  while (dex_data.HasNextDirectMethod()) {
    const DexFile::MethodId& id = dex_file.GetMethodId(dex_data.GetMemberIndex());
    os << "  " << dex_file.GetMethodName(id) << dex_file.GetMethodSignature(id).ToString() << "\n";
    dex_data.Next();
  }
  os << " Virtual methods:\n";
  while (dex_data.HasNextVirtualMethod()) {
    const DexFile::MethodId& id = dex_file.GetMethodId(dex_data.GetMemberIndex());
    os << "  " << dex_file.GetMethodName(id) << dex_file.GetMethodSignature(id).ToString() << "\n";
    dex_data.Next();
  }
}

static std::string DumpClasses(const DexFile& dex_file1,
                               const DexFile::ClassDef& dex_class_def1,
                               const DexFile& dex_file2,
                               const DexFile::ClassDef& dex_class_def2) {
  std::ostringstream os;
  DumpClass(os, dex_file1, dex_class_def1, " (Compile time)");
  DumpClass(os, dex_file2, dex_class_def2, " (Runtime)");
  return os.str();
}


// Very simple structural check on whether the classes match. Only compares the number of
// methods and fields.
static bool SimpleStructuralCheck(const DexFile& dex_file1,
                                  const DexFile::ClassDef& dex_class_def1,
                                  const DexFile& dex_file2,
                                  const DexFile::ClassDef& dex_class_def2,
                                  std::string* error_msg) {
  ClassDataItemIterator dex_data1(dex_file1, dex_file1.GetClassData(dex_class_def1));
  ClassDataItemIterator dex_data2(dex_file2, dex_file2.GetClassData(dex_class_def2));

  // Counters for current dex file.
  size_t dex_virtual_methods1, dex_direct_methods1, dex_static_fields1, dex_instance_fields1;
  CountMethodsAndFields(dex_data1,
                        &dex_virtual_methods1,
                        &dex_direct_methods1,
                        &dex_static_fields1,
                        &dex_instance_fields1);
  // Counters for compile-time dex file.
  size_t dex_virtual_methods2, dex_direct_methods2, dex_static_fields2, dex_instance_fields2;
  CountMethodsAndFields(dex_data2,
                        &dex_virtual_methods2,
                        &dex_direct_methods2,
                        &dex_static_fields2,
                        &dex_instance_fields2);

  if (dex_virtual_methods1 != dex_virtual_methods2) {
    std::string class_dump = DumpClasses(dex_file1, dex_class_def1, dex_file2, dex_class_def2);
    *error_msg = StringPrintf("Virtual method count off: %zu vs %zu\n%s",
                              dex_virtual_methods1,
                              dex_virtual_methods2,
                              class_dump.c_str());
    return false;
  }
  if (dex_direct_methods1 != dex_direct_methods2) {
    std::string class_dump = DumpClasses(dex_file1, dex_class_def1, dex_file2, dex_class_def2);
    *error_msg = StringPrintf("Direct method count off: %zu vs %zu\n%s",
                              dex_direct_methods1,
                              dex_direct_methods2,
                              class_dump.c_str());
    return false;
  }
  if (dex_static_fields1 != dex_static_fields2) {
    std::string class_dump = DumpClasses(dex_file1, dex_class_def1, dex_file2, dex_class_def2);
    *error_msg = StringPrintf("Static field count off: %zu vs %zu\n%s",
                              dex_static_fields1,
                              dex_static_fields2,
                              class_dump.c_str());
    return false;
  }
  if (dex_instance_fields1 != dex_instance_fields2) {
    std::string class_dump = DumpClasses(dex_file1, dex_class_def1, dex_file2, dex_class_def2);
    *error_msg = StringPrintf("Instance field count off: %zu vs %zu\n%s",
                              dex_instance_fields1,
                              dex_instance_fields2,
                              class_dump.c_str());
    return false;
  }

  return true;
}

// Checks whether a the super-class changed from what we had at compile-time. This would
// invalidate quickening.
static bool CheckSuperClassChange(Handle<mirror::Class> klass,
                                  const DexFile& dex_file,
                                  const DexFile::ClassDef& class_def,
                                  mirror::Class* super_class)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  // Check for unexpected changes in the superclass.
  // Quick check 1) is the super_class class-loader the boot class loader? This always has
  // precedence.
  if (super_class->GetClassLoader() != nullptr &&
      // Quick check 2) different dex cache? Breaks can only occur for different dex files,
      // which is implied by different dex cache.
      klass->GetDexCache() != super_class->GetDexCache()) {
    // Now comes the expensive part: things can be broken if (a) the klass' dex file has a
    // definition for the super-class, and (b) the files are in separate oat files. The oat files
    // are referenced from the dex file, so do (b) first. Only relevant if we have oat files.
    const OatDexFile* class_oat_dex_file = dex_file.GetOatDexFile();
    const OatFile* class_oat_file = nullptr;
    if (class_oat_dex_file != nullptr) {
      class_oat_file = class_oat_dex_file->GetOatFile();
    }

    if (class_oat_file != nullptr) {
      const OatDexFile* loaded_super_oat_dex_file = super_class->GetDexFile().GetOatDexFile();
      const OatFile* loaded_super_oat_file = nullptr;
      if (loaded_super_oat_dex_file != nullptr) {
        loaded_super_oat_file = loaded_super_oat_dex_file->GetOatFile();
      }

      if (loaded_super_oat_file != nullptr && class_oat_file != loaded_super_oat_file) {
        // Now check (a).
        const DexFile::ClassDef* super_class_def = dex_file.FindClassDef(class_def.superclass_idx_);
        if (super_class_def != nullptr) {
          // Uh-oh, we found something. Do our check.
          std::string error_msg;
          if (!SimpleStructuralCheck(dex_file, *super_class_def,
                                     super_class->GetDexFile(), *super_class->GetClassDef(),
                                     &error_msg)) {
            // Print a warning to the log. This exception might be caught, e.g., as common in test
            // drivers. When the class is later tried to be used, we re-throw a new instance, as we
            // only save the type of the exception.
            LOG(WARNING) << "Incompatible structural change detected: " <<
                StringPrintf(
                    "Structural change of %s is hazardous (%s at compile time, %s at runtime): %s",
                    PrettyType(super_class_def->class_idx_, dex_file).c_str(),
                    class_oat_file->GetLocation().c_str(),
                    loaded_super_oat_file->GetLocation().c_str(),
                    error_msg.c_str());
            ThrowIncompatibleClassChangeError(klass.Get(),
                "Structural change of %s is hazardous (%s at compile time, %s at runtime): %s",
                PrettyType(super_class_def->class_idx_, dex_file).c_str(),
                class_oat_file->GetLocation().c_str(),
                loaded_super_oat_file->GetLocation().c_str(),
                error_msg.c_str());
            return false;
          }
        }
      }
    }
  }
  return true;
}

bool ClassLinker::LoadSuperAndInterfaces(Handle<mirror::Class> klass, const DexFile& dex_file) {
  CHECK_EQ(mirror::Class::kStatusIdx, klass->GetStatus());
  const DexFile::ClassDef& class_def = dex_file.GetClassDef(klass->GetDexClassDefIndex());
  uint16_t super_class_idx = class_def.superclass_idx_;
  if (super_class_idx != DexFile::kDexNoIndex16) {
    // Check that a class does not inherit from itself directly.
    //
    // TODO: This is a cheap check to detect the straightforward case
    // of a class extending itself (b/28685551), but we should do a
    // proper cycle detection on loaded classes, to detect all cases
    // of class circularity errors (b/28830038).
    if (super_class_idx == class_def.class_idx_) {
      ThrowClassCircularityError(klass.Get(),
                                 "Class %s extends itself",
                                 PrettyDescriptor(klass.Get()).c_str());
      return false;
    }

    mirror::Class* super_class = ResolveType(dex_file, super_class_idx, klass.Get());
    if (super_class == nullptr) {
      DCHECK(Thread::Current()->IsExceptionPending());
      return false;
    }
    // Verify
    if (!klass->CanAccess(super_class)) {
      ThrowIllegalAccessError(klass.Get(), "Class %s extended by class %s is inaccessible",
                              PrettyDescriptor(super_class).c_str(),
                              PrettyDescriptor(klass.Get()).c_str());
      return false;
    }
    CHECK(super_class->IsResolved());
    klass->SetSuperClass(super_class);
    if (Runtime::Current()->UseCHA()) {
      mirror::Class* parent = super_class;
      mirror::Class* child = klass.Get();
      AddCHA(parent, child);
    }

    if (!CheckSuperClassChange(klass, dex_file, class_def, super_class)) {
      DCHECK(Thread::Current()->IsExceptionPending());
      return false;
    }
  }
  const DexFile::TypeList* interfaces = dex_file.GetInterfacesList(class_def);
  if (interfaces != nullptr) {
    for (size_t i = 0; i < interfaces->Size(); i++) {
      uint16_t idx = interfaces->GetTypeItem(i).type_idx_;
      mirror::Class* interface = ResolveType(dex_file, idx, klass.Get());
      if (interface == nullptr) {
        DCHECK(Thread::Current()->IsExceptionPending());
        return false;
      }
      // Verify
      if (!klass->CanAccess(interface)) {
        // TODO: the RI seemed to ignore this in my testing.
        ThrowIllegalAccessError(klass.Get(),
                                "Interface %s implemented by class %s is inaccessible",
                                PrettyDescriptor(interface).c_str(),
                                PrettyDescriptor(klass.Get()).c_str());
        return false;
      }
    }
  }
  // Mark the class as loaded.
  mirror::Class::SetStatus(klass, mirror::Class::kStatusLoaded, nullptr);
  return true;
}

bool ClassLinker::LinkSuperClass(Handle<mirror::Class> klass) {
  CHECK(!klass->IsPrimitive());
  mirror::Class* super = klass->GetSuperClass();
  if (klass.Get() == GetClassRoot(kJavaLangObject)) {
    if (super != nullptr) {
      ThrowClassFormatError(klass.Get(), "java.lang.Object must not have a superclass");
      return false;
    }
    return true;
  }
  if (super == nullptr) {
    ThrowLinkageError(klass.Get(), "No superclass defined for class %s",
                      PrettyDescriptor(klass.Get()).c_str());
    return false;
  }
  // Verify
  if (super->IsFinal() || super->IsInterface()) {
    ThrowIncompatibleClassChangeError(klass.Get(),
                                      "Superclass %s of %s is %s",
                                      PrettyDescriptor(super).c_str(),
                                      PrettyDescriptor(klass.Get()).c_str(),
                                      super->IsFinal() ? "declared final" : "an interface");
    return false;
  }
  if (!klass->CanAccess(super)) {
    ThrowIllegalAccessError(klass.Get(), "Superclass %s is inaccessible to class %s",
                            PrettyDescriptor(super).c_str(),
                            PrettyDescriptor(klass.Get()).c_str());
    return false;
  }

  // Inherit kAccClassIsFinalizable from the superclass in case this
  // class doesn't override finalize.
  if (super->IsFinalizable()) {
    klass->SetFinalizable();
  }

  // Inherit class loader flag form super class.
  if (super->IsClassLoaderClass()) {
    klass->SetClassLoaderClass();
  }

  // Inherit reference flags (if any) from the superclass.
  uint32_t reference_flags = (super->GetClassFlags() & mirror::kClassFlagReference);
  if (reference_flags != 0) {
    CHECK_EQ(klass->GetClassFlags(), 0u);
    klass->SetClassFlags(klass->GetClassFlags() | reference_flags);
  }
  // Disallow custom direct subclasses of java.lang.ref.Reference.
  if (init_done_ && super == GetClassRoot(kJavaLangRefReference)) {
    ThrowLinkageError(klass.Get(),
                      "Class %s attempts to subclass java.lang.ref.Reference, which is not allowed",
                      PrettyDescriptor(klass.Get()).c_str());
    return false;
  }

  if (kIsDebugBuild) {
    // Ensure super classes are fully resolved prior to resolving fields..
    while (super != nullptr) {
      CHECK(super->IsResolved());
      super = super->GetSuperClass();
    }
  }
  return true;
}

// Populate the class vtable and itable. Compute return type indices.
bool ClassLinker::LinkMethods(Thread* self,
                              Handle<mirror::Class> klass,
                              Handle<mirror::ObjectArray<mirror::Class>> interfaces,
                              ArtMethod** out_imt) {
  self->AllowThreadSuspension();
  // A map from vtable indexes to the method they need to be updated to point to. Used because we
  // need to have default methods be in the virtuals array of each class but we don't set that up
  // until LinkInterfaceMethods.
  std::unordered_map<size_t, ClassLinker::MethodTranslation> default_translations;
  // Link virtual methods then interface methods.
  // We set up the interface lookup table first because we need it to determine if we need to update
  // any vtable entries with new default method implementations.
  return SetupInterfaceLookupTable(self, klass, interfaces)
          && LinkVirtualMethods(self, klass, /*out*/ &default_translations)
          && LinkInterfaceMethods(self, klass, default_translations, out_imt);
}

// Comparator for name and signature of a method, used in finding overriding methods. Implementation
// avoids the use of handles, if it didn't then rather than compare dex files we could compare dex
// caches in the implementation below.
class MethodNameAndSignatureComparator FINAL : public ValueObject {
 public:
  explicit MethodNameAndSignatureComparator(ArtMethod* method)
      SHARED_REQUIRES(Locks::mutator_lock_) :
      dex_file_(method->GetDexFile()), mid_(&dex_file_->GetMethodId(method->GetDexMethodIndex())),
      name_(nullptr), name_len_(0) {
    DCHECK(!method->IsProxyMethod()) << PrettyMethod(method);
  }

  const char* GetName() {
    if (name_ == nullptr) {
      name_ = dex_file_->StringDataAndUtf16LengthByIdx(mid_->name_idx_, &name_len_);
    }
    return name_;
  }

  bool HasSameNameAndSignature(ArtMethod* other)
      SHARED_REQUIRES(Locks::mutator_lock_) {
    DCHECK(!other->IsProxyMethod()) << PrettyMethod(other);
    const DexFile* other_dex_file = other->GetDexFile();
    const DexFile::MethodId& other_mid = other_dex_file->GetMethodId(other->GetDexMethodIndex());
    if (dex_file_ == other_dex_file) {
      return mid_->name_idx_ == other_mid.name_idx_ && mid_->proto_idx_ == other_mid.proto_idx_;
    }
    GetName();  // Only used to make sure its calculated.
    uint32_t other_name_len;
    const char* other_name = other_dex_file->StringDataAndUtf16LengthByIdx(other_mid.name_idx_,
                                                                           &other_name_len);
    if (name_len_ != other_name_len || strcmp(name_, other_name) != 0) {
      return false;
    }
    return dex_file_->GetMethodSignature(*mid_) == other_dex_file->GetMethodSignature(other_mid);
  }

 private:
  // Dex file for the method to compare against.
  const DexFile* const dex_file_;
  // MethodId for the method to compare against.
  const DexFile::MethodId* const mid_;
  // Lazily computed name from the dex file's strings.
  const char* name_;
  // Lazily computed name length.
  uint32_t name_len_;
};

class LinkVirtualHashTable {
 public:
  LinkVirtualHashTable(Handle<mirror::Class> klass,
                       size_t hash_size,
                       uint32_t* hash_table,
                       size_t image_pointer_size)
     : klass_(klass),
       hash_size_(hash_size),
       hash_table_(hash_table),
       image_pointer_size_(image_pointer_size) {
    std::fill(hash_table_, hash_table_ + hash_size_, invalid_index_);
  }

  void Add(uint32_t virtual_method_index) SHARED_REQUIRES(Locks::mutator_lock_) {
    ArtMethod* local_method = klass_->GetVirtualMethodDuringLinking(
        virtual_method_index, image_pointer_size_);
    const char* name = local_method->GetInterfaceMethodIfProxy(image_pointer_size_)->GetName();
    uint32_t hash = ComputeModifiedUtf8Hash(name);
    uint32_t index = hash % hash_size_;
    // Linear probe until we have an empty slot.
    while (hash_table_[index] != invalid_index_) {
      if (++index == hash_size_) {
        index = 0;
      }
    }
    hash_table_[index] = virtual_method_index;
  }

  uint32_t FindAndRemove(MethodNameAndSignatureComparator* comparator)
      SHARED_REQUIRES(Locks::mutator_lock_) {
    const char* name = comparator->GetName();
    uint32_t hash = ComputeModifiedUtf8Hash(name);
    size_t index = hash % hash_size_;
    while (true) {
      const uint32_t value = hash_table_[index];
      // Since linear probe makes continuous blocks, hitting an invalid index means we are done
      // the block and can safely assume not found.
      if (value == invalid_index_) {
        break;
      }
      if (value != removed_index_) {  // This signifies not already overriden.
        ArtMethod* virtual_method =
            klass_->GetVirtualMethodDuringLinking(value, image_pointer_size_);
        if (comparator->HasSameNameAndSignature(
            virtual_method->GetInterfaceMethodIfProxy(image_pointer_size_))) {
          hash_table_[index] = removed_index_;
          return value;
        }
      }
      if (++index == hash_size_) {
        index = 0;
      }
    }
    return GetNotFoundIndex();
  }

  static uint32_t GetNotFoundIndex() {
    return invalid_index_;
  }

 private:
  static const uint32_t invalid_index_;
  static const uint32_t removed_index_;

  Handle<mirror::Class> klass_;
  const size_t hash_size_;
  uint32_t* const hash_table_;
  const size_t image_pointer_size_;
};

const uint32_t LinkVirtualHashTable::invalid_index_ = std::numeric_limits<uint32_t>::max();
const uint32_t LinkVirtualHashTable::removed_index_ = std::numeric_limits<uint32_t>::max() - 1;

bool ClassLinker::LinkVirtualMethods(
    Thread* self,
    Handle<mirror::Class> klass,
    /*out*/std::unordered_map<size_t, ClassLinker::MethodTranslation>* default_translations) {
  const size_t num_virtual_methods = klass->NumVirtualMethods();
  if (klass->IsInterface()) {
    // No vtable.
    if (!IsUint<16>(num_virtual_methods)) {
      ThrowClassFormatError(klass.Get(), "Too many methods on interface: %zu", num_virtual_methods);
      return false;
    }
    bool has_defaults = false;
    // Assign each method an IMT index and set the default flag.
    for (size_t i = 0; i < num_virtual_methods; ++i) {
      ArtMethod* m = klass->GetVirtualMethodDuringLinking(i, image_pointer_size_);
      m->SetMethodIndex(i);
      if (!m->IsAbstract()) {
        m->SetAccessFlags(m->GetAccessFlags() | kAccDefault);
        has_defaults = true;
      }
    }
    // Mark that we have default methods so that we won't need to scan the virtual_methods_ array
    // during initialization. This is a performance optimization. We could simply traverse the
    // virtual_methods_ array again during initialization.
    if (has_defaults) {
      klass->SetHasDefaultMethods();
    }
    return true;
  } else if (klass->HasSuperClass()) {
    const size_t super_vtable_length = klass->GetSuperClass()->GetVTableLength();
    const size_t max_count = num_virtual_methods + super_vtable_length;
    StackHandleScope<2> hs(self);
    Handle<mirror::Class> super_class(hs.NewHandle(klass->GetSuperClass()));
    MutableHandle<mirror::PointerArray> vtable;
    if (super_class->ShouldHaveEmbeddedImtAndVTable()) {
      vtable = hs.NewHandle(AllocPointerArray(self, max_count));
      if (UNLIKELY(vtable.Get() == nullptr)) {
        self->AssertPendingOOMException();
        return false;
      }
      for (size_t i = 0; i < super_vtable_length; i++) {
        vtable->SetElementPtrSize(
            i, super_class->GetEmbeddedVTableEntry(i, image_pointer_size_), image_pointer_size_);
      }
      // We might need to change vtable if we have new virtual methods or new interfaces (since that
      // might give us new default methods). If no new interfaces then we can skip the rest since
      // the class cannot override any of the super-class's methods. This is required for
      // correctness since without it we might not update overridden default method vtable entries
      // correctly.
      if (num_virtual_methods == 0 && super_class->GetIfTableCount() == klass->GetIfTableCount()) {
        klass->SetVTable(vtable.Get());
        return true;
      }
    } else {
      DCHECK(super_class->IsAbstract() && !super_class->IsArrayClass());
      auto* super_vtable = super_class->GetVTable();
      CHECK(super_vtable != nullptr) << PrettyClass(super_class.Get());
      // We might need to change vtable if we have new virtual methods or new interfaces (since that
      // might give us new default methods). See comment above.
      if (num_virtual_methods == 0 && super_class->GetIfTableCount() == klass->GetIfTableCount()) {
        klass->SetVTable(super_vtable);
        return true;
      }
      vtable = hs.NewHandle(down_cast<mirror::PointerArray*>(
          super_vtable->CopyOf(self, max_count)));
      if (UNLIKELY(vtable.Get() == nullptr)) {
        self->AssertPendingOOMException();
        return false;
      }
    }
    // How the algorithm works:
    // 1. Populate hash table by adding num_virtual_methods from klass. The values in the hash
    // table are: invalid_index for unused slots, index super_vtable_length + i for a virtual
    // method which has not been matched to a vtable method, and j if the virtual method at the
    // index overrode the super virtual method at index j.
    // 2. Loop through super virtual methods, if they overwrite, update hash table to j
    // (j < super_vtable_length) to avoid redundant checks. (TODO maybe use this info for reducing
    // the need for the initial vtable which we later shrink back down).
    // 3. Add non overridden methods to the end of the vtable.
    static constexpr size_t kMaxStackHash = 250;
    // + 1 so that even if we only have new default methods we will still be able to use this hash
    // table (i.e. it will never have 0 size).
    const size_t hash_table_size = num_virtual_methods * 3 + 1;
    uint32_t* hash_table_ptr;
    std::unique_ptr<uint32_t[]> hash_heap_storage;
    if (hash_table_size <= kMaxStackHash) {
      hash_table_ptr = reinterpret_cast<uint32_t*>(
          alloca(hash_table_size * sizeof(*hash_table_ptr)));
    } else {
      hash_heap_storage.reset(new uint32_t[hash_table_size]);
      hash_table_ptr = hash_heap_storage.get();
    }
    LinkVirtualHashTable hash_table(klass, hash_table_size, hash_table_ptr, image_pointer_size_);
    // Add virtual methods to the hash table.
    for (size_t i = 0; i < num_virtual_methods; ++i) {
      DCHECK(klass->GetVirtualMethodDuringLinking(
          i, image_pointer_size_)->GetDeclaringClass() != nullptr);
      hash_table.Add(i);
    }
    // Loop through each super vtable method and see if they are overridden by a method we added to
    // the hash table.
    for (size_t j = 0; j < super_vtable_length; ++j) {
      // Search the hash table to see if we are overridden by any method.
      ArtMethod* super_method = vtable->GetElementPtrSize<ArtMethod*>(j, image_pointer_size_);
      MethodNameAndSignatureComparator super_method_name_comparator(
          super_method->GetInterfaceMethodIfProxy(image_pointer_size_));
      uint32_t hash_index = hash_table.FindAndRemove(&super_method_name_comparator);
      if (hash_index != hash_table.GetNotFoundIndex()) {
        ArtMethod* virtual_method = klass->GetVirtualMethodDuringLinking(
            hash_index, image_pointer_size_);
        if (klass->CanAccessMember(super_method->GetDeclaringClass(),
                                   super_method->GetAccessFlags())) {
          if (super_method->IsFinal()) {
            ThrowLinkageError(klass.Get(), "Method %s overrides final method in class %s",
                              PrettyMethod(virtual_method).c_str(),
                              super_method->GetDeclaringClassDescriptor());
            return false;
          }
          vtable->SetElementPtrSize(j, virtual_method, image_pointer_size_);
          virtual_method->SetMethodIndex(j);
        } else {
          LOG(WARNING) << "Before Android 4.1, method " << PrettyMethod(virtual_method)
                       << " would have incorrectly overridden the package-private method in "
                       << PrettyDescriptor(super_method->GetDeclaringClassDescriptor());
        }
      } else if (super_method->IsOverridableByDefaultMethod()) {
        // We didn't directly override this method but we might through default methods...
        // Check for default method update.
        ArtMethod* default_method = nullptr;
        switch (FindDefaultMethodImplementation(self,
                                                super_method,
                                                klass,
                                                /*out*/&default_method)) {
          case DefaultMethodSearchResult::kDefaultConflict: {
            // A conflict was found looking for default methods. Note this (assuming it wasn't
            // pre-existing) in the translations map.
            if (UNLIKELY(!super_method->IsDefaultConflicting())) {
              // Don't generate another conflict method to reduce memory use as an optimization.
              default_translations->insert(
                  {j, ClassLinker::MethodTranslation::CreateConflictingMethod()});
            }
            break;
          }
          case DefaultMethodSearchResult::kAbstractFound: {
            // No conflict but method is abstract.
            // We note that this vtable entry must be made abstract.
            if (UNLIKELY(!super_method->IsAbstract())) {
              default_translations->insert(
                  {j, ClassLinker::MethodTranslation::CreateAbstractMethod()});
            }
            break;
          }
          case DefaultMethodSearchResult::kDefaultFound: {
            if (UNLIKELY(super_method->IsDefaultConflicting() ||
                        default_method->GetDeclaringClass() != super_method->GetDeclaringClass())) {
              // Found a default method implementation that is new.
              // TODO Refactor this add default methods to virtuals here and not in
              //      LinkInterfaceMethods maybe.
              //      The problem is default methods might override previously present
              //      default-method or miranda-method vtable entries from the superclass.
              //      Unfortunately we need these to be entries in this class's virtuals. We do not
              //      give these entries there until LinkInterfaceMethods so we pass this map around
              //      to let it know which vtable entries need to be updated.
              // Make a note that vtable entry j must be updated, store what it needs to be updated
              // to. We will allocate a virtual method slot in LinkInterfaceMethods and fix it up
              // then.
              default_translations->insert(
                  {j, ClassLinker::MethodTranslation::CreateTranslatedMethod(default_method)});
              VLOG(class_linker) << "Method " << PrettyMethod(super_method)
                                 << " overridden by default " << PrettyMethod(default_method)
                                 << " in " << PrettyClass(klass.Get());
            }
            break;
          }
        }
      }
    }
    size_t actual_count = super_vtable_length;
    // Add the non-overridden methods at the end.
    for (size_t i = 0; i < num_virtual_methods; ++i) {
      ArtMethod* local_method = klass->GetVirtualMethodDuringLinking(i, image_pointer_size_);
      size_t method_idx = local_method->GetMethodIndexDuringLinking();
      if (method_idx < super_vtable_length &&
          local_method == vtable->GetElementPtrSize<ArtMethod*>(method_idx, image_pointer_size_)) {
        continue;
      }
      vtable->SetElementPtrSize(actual_count, local_method, image_pointer_size_);
      local_method->SetMethodIndex(actual_count);
      ++actual_count;
    }
    if (!IsUint<16>(actual_count)) {
      ThrowClassFormatError(klass.Get(), "Too many methods defined on class: %zd", actual_count);
      return false;
    }
    // Shrink vtable if possible
    CHECK_LE(actual_count, max_count);
    if (actual_count < max_count) {
      vtable.Assign(down_cast<mirror::PointerArray*>(vtable->CopyOf(self, actual_count)));
      if (UNLIKELY(vtable.Get() == nullptr)) {
        self->AssertPendingOOMException();
        return false;
      }
    }
    klass->SetVTable(vtable.Get());
  } else {
    CHECK_EQ(klass.Get(), GetClassRoot(kJavaLangObject));
    if (!IsUint<16>(num_virtual_methods)) {
      ThrowClassFormatError(klass.Get(), "Too many methods: %d",
                            static_cast<int>(num_virtual_methods));
      return false;
    }
    auto* vtable = AllocPointerArray(self, num_virtual_methods);
    if (UNLIKELY(vtable == nullptr)) {
      self->AssertPendingOOMException();
      return false;
    }
    for (size_t i = 0; i < num_virtual_methods; ++i) {
      ArtMethod* virtual_method = klass->GetVirtualMethodDuringLinking(i, image_pointer_size_);
      vtable->SetElementPtrSize(i, virtual_method, image_pointer_size_);
      virtual_method->SetMethodIndex(i & 0xFFFF);
    }
    klass->SetVTable(vtable);
  }
  return true;
}

// Determine if the given iface has any subinterface in the given list that declares the method
// specified by 'target'.
//
// Arguments
// - self:    The thread we are running on
// - target:  A comparator that will match any method that overrides the method we are checking for
// - iftable: The iftable we are searching for an overriding method on.
// - ifstart: The index of the interface we are checking to see if anything overrides
// - iface:   The interface we are checking to see if anything overrides.
// - image_pointer_size:
//            The image pointer size.
//
// Returns
// - True:  There is some method that matches the target comparator defined in an interface that
//          is a subtype of iface.
// - False: There is no method that matches the target comparator in any interface that is a subtype
//          of iface.
static bool ContainsOverridingMethodOf(Thread* self,
                                       MethodNameAndSignatureComparator& target,
                                       Handle<mirror::IfTable> iftable,
                                       size_t ifstart,
                                       Handle<mirror::Class> iface,
                                       size_t image_pointer_size)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  DCHECK(self != nullptr);
  DCHECK(iface.Get() != nullptr);
  DCHECK(iftable.Get() != nullptr);
  DCHECK_GE(ifstart, 0u);
  DCHECK_LT(ifstart, iftable->Count());
  DCHECK_EQ(iface.Get(), iftable->GetInterface(ifstart));
  DCHECK(iface->IsInterface());

  size_t iftable_count = iftable->Count();
  StackHandleScope<1> hs(self);
  MutableHandle<mirror::Class> current_iface(hs.NewHandle<mirror::Class>(nullptr));
  for (size_t k = ifstart + 1; k < iftable_count; k++) {
    // Skip ifstart since our current interface obviously cannot override itself.
    current_iface.Assign(iftable->GetInterface(k));
    // Iterate through every method on this interface. The order does not matter.
    for (ArtMethod& current_method : current_iface->GetDeclaredVirtualMethods(image_pointer_size)) {
      if (UNLIKELY(target.HasSameNameAndSignature(
                      current_method.GetInterfaceMethodIfProxy(image_pointer_size)))) {
        // Check if the i'th interface is a subtype of this one.
        if (iface->IsAssignableFrom(current_iface.Get())) {
          return true;
        }
        break;
      }
    }
  }
  return false;
}

// Find the default method implementation for 'interface_method' in 'klass'. Stores it into
// out_default_method and returns kDefaultFound on success. If no default method was found return
// kAbstractFound and store nullptr into out_default_method. If an error occurs (such as a
// default_method conflict) it will return kDefaultConflict.
ClassLinker::DefaultMethodSearchResult ClassLinker::FindDefaultMethodImplementation(
    Thread* self,
    ArtMethod* target_method,
    Handle<mirror::Class> klass,
    /*out*/ArtMethod** out_default_method) const {
  DCHECK(self != nullptr);
  DCHECK(target_method != nullptr);
  DCHECK(out_default_method != nullptr);

  *out_default_method = nullptr;

  // We organize the interface table so that, for interface I any subinterfaces J follow it in the
  // table. This lets us walk the table backwards when searching for default methods.  The first one
  // we encounter is the best candidate since it is the most specific. Once we have found it we keep
  // track of it and then continue checking all other interfaces, since we need to throw an error if
  // we encounter conflicting default method implementations (one is not a subtype of the other).
  //
  // The order of unrelated interfaces does not matter and is not defined.
  size_t iftable_count = klass->GetIfTableCount();
  if (iftable_count == 0) {
    // No interfaces. We have already reset out to null so just return kAbstractFound.
    return DefaultMethodSearchResult::kAbstractFound;
  }

  StackHandleScope<3> hs(self);
  MutableHandle<mirror::Class> chosen_iface(hs.NewHandle<mirror::Class>(nullptr));
  MutableHandle<mirror::IfTable> iftable(hs.NewHandle(klass->GetIfTable()));
  MutableHandle<mirror::Class> iface(hs.NewHandle<mirror::Class>(nullptr));
  MethodNameAndSignatureComparator target_name_comparator(
      target_method->GetInterfaceMethodIfProxy(image_pointer_size_));
  // Iterates over the klass's iftable in reverse
  for (size_t k = iftable_count; k != 0; ) {
    --k;

    DCHECK_LT(k, iftable->Count());

    iface.Assign(iftable->GetInterface(k));
    // Iterate through every declared method on this interface. The order does not matter.
    for (auto& method_iter : iface->GetDeclaredVirtualMethods(image_pointer_size_)) {
      ArtMethod* current_method = &method_iter;
      // Skip abstract methods and methods with different names.
      if (current_method->IsAbstract() ||
          !target_name_comparator.HasSameNameAndSignature(
              current_method->GetInterfaceMethodIfProxy(image_pointer_size_))) {
        continue;
      } else if (!current_method->IsPublic()) {
        // The verifier should have caught the non-public method for dex version 37. Just warn and
        // skip it since this is from before default-methods so we don't really need to care that it
        // has code.
        LOG(WARNING) << "Interface method " << PrettyMethod(current_method) << " is not public! "
                     << "This will be a fatal error in subsequent versions of android. "
                     << "Continuing anyway.";
      }
      if (UNLIKELY(chosen_iface.Get() != nullptr)) {
        // We have multiple default impls of the same method. This is a potential default conflict.
        // We need to check if this possibly conflicting method is either a superclass of the chosen
        // default implementation or is overridden by a non-default interface method. In either case
        // there is no conflict.
        if (!iface->IsAssignableFrom(chosen_iface.Get()) &&
            !ContainsOverridingMethodOf(self,
                                        target_name_comparator,
                                        iftable,
                                        k,
                                        iface,
                                        image_pointer_size_)) {
          VLOG(class_linker) << "Conflicting default method implementations found: "
                             << PrettyMethod(current_method) << " and "
                             << PrettyMethod(*out_default_method) << " in class "
                             << PrettyClass(klass.Get()) << " conflict.";
          *out_default_method = nullptr;
          return DefaultMethodSearchResult::kDefaultConflict;
        } else {
          break;  // Continue checking at the next interface.
        }
      } else {
        // chosen_iface == null
        if (!ContainsOverridingMethodOf(self,
                                        target_name_comparator,
                                        iftable,
                                        k,
                                        iface,
                                        image_pointer_size_)) {
          // Don't set this as the chosen interface if something else is overriding it (because that
          // other interface would be potentially chosen instead if it was default). If the other
          // interface was abstract then we wouldn't select this interface as chosen anyway since
          // the abstract method masks it.
          *out_default_method = current_method;
          chosen_iface.Assign(iface.Get());
          // We should now finish traversing the graph to find if we have default methods that
          // conflict.
        } else {
          VLOG(class_linker) << "A default method '" << PrettyMethod(current_method) << "' was "
                            << "skipped because it was overridden by an abstract method in a "
                            << "subinterface on class '" << PrettyClass(klass.Get()) << "'";
        }
      }
      break;
    }
  }
  if (*out_default_method != nullptr) {
    VLOG(class_linker) << "Default method '" << PrettyMethod(*out_default_method) << "' selected "
                       << "as the implementation for '" << PrettyMethod(target_method) << "' "
                       << "in '" << PrettyClass(klass.Get()) << "'";
    return DefaultMethodSearchResult::kDefaultFound;
  } else {
    return DefaultMethodSearchResult::kAbstractFound;
  }
}

ArtMethod* ClassLinker::AddMethodToConflictTable(mirror::Class* klass,
                                                 ArtMethod* conflict_method,
                                                 ArtMethod* interface_method,
                                                 ArtMethod* method,
                                                 bool force_new_conflict_method) {
  ImtConflictTable* current_table = conflict_method->GetImtConflictTable(sizeof(void*));
  Runtime* const runtime = Runtime::Current();
  LinearAlloc* linear_alloc = GetAllocatorForClassLoader(klass->GetClassLoader());
  bool new_entry = conflict_method == runtime->GetImtConflictMethod() || force_new_conflict_method;

  // Create a new entry if the existing one is the shared conflict method.
  ArtMethod* new_conflict_method = new_entry
      ? runtime->CreateImtConflictMethod(linear_alloc)
      : conflict_method;

  // Allocate a new table. Note that we will leak this table at the next conflict,
  // but that's a tradeoff compared to making the table fixed size.
  void* data = linear_alloc->Alloc(
      Thread::Current(), ImtConflictTable::ComputeSizeWithOneMoreEntry(current_table,
                                                                       image_pointer_size_));
  if (data == nullptr) {
    LOG(ERROR) << "Failed to allocate conflict table";
    return conflict_method;
  }
  ImtConflictTable* new_table = new (data) ImtConflictTable(current_table,
                                                            interface_method,
                                                            method,
                                                            image_pointer_size_);

  // Do a fence to ensure threads see the data in the table before it is assigned
  // to the conflict method.
  // Note that there is a race in the presence of multiple threads and we may leak
  // memory from the LinearAlloc, but that's a tradeoff compared to using
  // atomic operations.
  QuasiAtomic::ThreadFenceRelease();
  new_conflict_method->SetImtConflictTable(new_table, image_pointer_size_);
  return new_conflict_method;
}

void ClassLinker::SetIMTRef(ArtMethod* unimplemented_method,
                            ArtMethod* imt_conflict_method,
                            ArtMethod* current_method,
                            /*out*/ArtMethod** imt_ref) {
  // Place method in imt if entry is empty, place conflict otherwise.
  if (*imt_ref == unimplemented_method) {
    *imt_ref = current_method;
  } else if (!(*imt_ref)->IsRuntimeMethod()) {
    // If we are not a conflict and we have the same signature and name as the imt
    // entry, it must be that we overwrote a superclass vtable entry.
    // Note that we have checked IsRuntimeMethod, as there may be multiple different
    // conflict methods.
    MethodNameAndSignatureComparator imt_comparator(
        (*imt_ref)->GetInterfaceMethodIfProxy(image_pointer_size_));
    if (imt_comparator.HasSameNameAndSignature(
          current_method->GetInterfaceMethodIfProxy(image_pointer_size_))) {
      *imt_ref = current_method;
    } else {
      *imt_ref = imt_conflict_method;
    }
  } else {
    // Place the default conflict method. Note that there may be an existing conflict
    // method in the IMT, but it could be one tailored to the super class, with a
    // specific ImtConflictTable.
    *imt_ref = imt_conflict_method;
  }
}

void ClassLinker::FillIMTAndConflictTables(mirror::Class* klass) {
  DCHECK(klass->ShouldHaveEmbeddedImtAndVTable()) << PrettyClass(klass);
  DCHECK(!klass->IsTemp()) << PrettyClass(klass);
  ArtMethod* imt[mirror::Class::kImtSize];
  Runtime* const runtime = Runtime::Current();
  ArtMethod* const unimplemented_method = runtime->GetImtUnimplementedMethod();
  ArtMethod* const conflict_method = runtime->GetImtConflictMethod();
  std::fill_n(imt, arraysize(imt), unimplemented_method);
  if (klass->GetIfTable() != nullptr) {
    FillIMTFromIfTable(klass->GetIfTable(),
                       unimplemented_method,
                       conflict_method,
                       klass,
                       true,
                       false,
                       &imt[0]);
  }
  for (size_t i = 0; i < mirror::Class::kImtSize; ++i) {
    klass->SetEmbeddedImTableEntry(i, imt[i], image_pointer_size_);
  }
}

static inline uint32_t GetIMTIndex(ArtMethod* interface_method)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  return interface_method->GetDexMethodIndex() % mirror::Class::kImtSize;
}

ImtConflictTable* ClassLinker::CreateImtConflictTable(size_t count,
                                                      LinearAlloc* linear_alloc,
                                                      size_t image_pointer_size) {
  void* data = linear_alloc->Alloc(Thread::Current(),
                                   ImtConflictTable::ComputeSize(count,
                                                                 image_pointer_size));
  return (data != nullptr) ? new (data) ImtConflictTable(count, image_pointer_size) : nullptr;
}

ImtConflictTable* ClassLinker::CreateImtConflictTable(size_t count, LinearAlloc* linear_alloc) {
  return CreateImtConflictTable(count, linear_alloc, image_pointer_size_);
}

void ClassLinker::FillIMTFromIfTable(mirror::IfTable* if_table,
                                     ArtMethod* unimplemented_method,
                                     ArtMethod* imt_conflict_method,
                                     mirror::Class* klass,
                                     bool create_conflict_tables,
                                     bool ignore_copied_methods,
                                     ArtMethod** imt) {
  uint32_t conflict_counts[mirror::Class::kImtSize] = {};
  for (size_t i = 0, length = if_table->Count(); i < length; ++i) {
    mirror::Class* interface = if_table->GetInterface(i);
    const size_t num_virtuals = interface->NumVirtualMethods();
    const size_t method_array_count = if_table->GetMethodArrayCount(i);
    // Virtual methods can be larger than the if table methods if there are default methods.
    DCHECK_GE(num_virtuals, method_array_count);
    if (kIsDebugBuild) {
      if (klass->IsInterface()) {
        DCHECK_EQ(method_array_count, 0u);
      } else {
        DCHECK_EQ(interface->NumDeclaredVirtualMethods(), method_array_count);
      }
    }
    if (method_array_count == 0) {
      continue;
    }
    auto* method_array = if_table->GetMethodArray(i);
    for (size_t j = 0; j < method_array_count; ++j) {
      ArtMethod* implementation_method =
          method_array->GetElementPtrSize<ArtMethod*>(j, image_pointer_size_);
      if (ignore_copied_methods && implementation_method->IsCopied()) {
        continue;
      }
      DCHECK(implementation_method != nullptr);
      // Miranda methods cannot be used to implement an interface method, but they are safe to put
      // in the IMT since their entrypoint is the interface trampoline. If we put any copied methods
      // or interface methods in the IMT here they will not create extra conflicts since we compare
      // names and signatures in SetIMTRef.
      ArtMethod* interface_method = interface->GetVirtualMethod(j, image_pointer_size_);
      const uint32_t imt_index = GetIMTIndex(interface_method);

      // There is only any conflicts if all of the interface methods for an IMT slot don't have
      // the same implementation method, keep track of this to avoid creating a conflict table in
      // this case.

      // Conflict table size for each IMT slot.
      ++conflict_counts[imt_index];

      SetIMTRef(unimplemented_method,
                imt_conflict_method,
                implementation_method,
                /*out*/&imt[imt_index]);
    }
  }

  if (create_conflict_tables) {
    // Create the conflict tables.
    LinearAlloc* linear_alloc = GetAllocatorForClassLoader(klass->GetClassLoader());
    for (size_t i = 0; i < mirror::Class::kImtSize; ++i) {
      size_t conflicts = conflict_counts[i];
      if (imt[i] == imt_conflict_method) {
        ImtConflictTable* new_table = CreateImtConflictTable(conflicts, linear_alloc);
        if (new_table != nullptr) {
          ArtMethod* new_conflict_method =
              Runtime::Current()->CreateImtConflictMethod(linear_alloc);
          new_conflict_method->SetImtConflictTable(new_table, image_pointer_size_);
          imt[i] = new_conflict_method;
        } else {
          LOG(ERROR) << "Failed to allocate conflict table";
          imt[i] = imt_conflict_method;
        }
      } else {
        DCHECK_NE(imt[i], imt_conflict_method);
      }
    }

    for (size_t i = 0, length = if_table->Count(); i < length; ++i) {
      mirror::Class* interface = if_table->GetInterface(i);
      const size_t method_array_count = if_table->GetMethodArrayCount(i);
      // Virtual methods can be larger than the if table methods if there are default methods.
      if (method_array_count == 0) {
        continue;
      }
      auto* method_array = if_table->GetMethodArray(i);
      for (size_t j = 0; j < method_array_count; ++j) {
        ArtMethod* implementation_method =
            method_array->GetElementPtrSize<ArtMethod*>(j, image_pointer_size_);
        if (ignore_copied_methods && implementation_method->IsCopied()) {
          continue;
        }
        DCHECK(implementation_method != nullptr);
        ArtMethod* interface_method = interface->GetVirtualMethod(j, image_pointer_size_);
        const uint32_t imt_index = GetIMTIndex(interface_method);
        if (!imt[imt_index]->IsRuntimeMethod() ||
            imt[imt_index] == unimplemented_method ||
            imt[imt_index] == imt_conflict_method) {
          continue;
        }
        ImtConflictTable* table = imt[imt_index]->GetImtConflictTable(image_pointer_size_);
        const size_t num_entries = table->NumEntries(image_pointer_size_);
        table->SetInterfaceMethod(num_entries, image_pointer_size_, interface_method);
        table->SetImplementationMethod(num_entries, image_pointer_size_, implementation_method);
      }
    }
  }
}

// Simple helper function that checks that no subtypes of 'val' are contained within the 'classes'
// set.
static bool NotSubinterfaceOfAny(const std::unordered_set<mirror::Class*>& classes,
                                 mirror::Class* val)
    REQUIRES(Roles::uninterruptible_)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  DCHECK(val != nullptr);
  for (auto c : classes) {
    if (val->IsAssignableFrom(&*c)) {
      return false;
    }
  }
  return true;
}

// Fills in and flattens the interface inheritance hierarchy.
//
// By the end of this function all interfaces in the transitive closure of to_process are added to
// the iftable and every interface precedes all of its sub-interfaces in this list.
//
// all I, J: Interface | I <: J implies J precedes I
//
// (note A <: B means that A is a subtype of B)
//
// This returns the total number of items in the iftable. The iftable might be resized down after
// this call.
//
// We order this backwards so that we do not need to reorder superclass interfaces when new
// interfaces are added in subclass's interface tables.
//
// Upon entry into this function iftable is a copy of the superclass's iftable with the first
// super_ifcount entries filled in with the transitive closure of the interfaces of the superclass.
// The other entries are uninitialized.  We will fill in the remaining entries in this function. The
// iftable must be large enough to hold all interfaces without changing its size.
static size_t FillIfTable(mirror::IfTable* iftable,
                          size_t super_ifcount,
                          std::vector<mirror::Class*> to_process)
    REQUIRES(Roles::uninterruptible_)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  // This is the set of all class's already in the iftable. Used to make checking if a class has
  // already been added quicker.
  std::unordered_set<mirror::Class*> classes_in_iftable;
  // The first super_ifcount elements are from the superclass. We note that they are already added.
  for (size_t i = 0; i < super_ifcount; i++) {
    mirror::Class* iface = iftable->GetInterface(i);
    DCHECK(NotSubinterfaceOfAny(classes_in_iftable, iface)) << "Bad ordering.";
    classes_in_iftable.insert(iface);
  }
  size_t filled_ifcount = super_ifcount;
  for (mirror::Class* interface : to_process) {
    // Let us call the first filled_ifcount elements of iftable the current-iface-list.
    // At this point in the loop current-iface-list has the invariant that:
    //    for every pair of interfaces I,J within it:
    //      if index_of(I) < index_of(J) then I is not a subtype of J

    // If we have already seen this element then all of its super-interfaces must already be in the
    // current-iface-list so we can skip adding it.
    if (!ContainsElement(classes_in_iftable, interface)) {
      // We haven't seen this interface so add all of its super-interfaces onto the
      // current-iface-list, skipping those already on it.
      int32_t ifcount = interface->GetIfTableCount();
      for (int32_t j = 0; j < ifcount; j++) {
        mirror::Class* super_interface = interface->GetIfTable()->GetInterface(j);
        if (!ContainsElement(classes_in_iftable, super_interface)) {
          DCHECK(NotSubinterfaceOfAny(classes_in_iftable, super_interface)) << "Bad ordering.";
          classes_in_iftable.insert(super_interface);
          iftable->SetInterface(filled_ifcount, super_interface);
          filled_ifcount++;
        }
      }
      DCHECK(NotSubinterfaceOfAny(classes_in_iftable, interface)) << "Bad ordering";
      // Place this interface onto the current-iface-list after all of its super-interfaces.
      classes_in_iftable.insert(interface);
      iftable->SetInterface(filled_ifcount, interface);
      filled_ifcount++;
    } else if (kIsDebugBuild) {
      // Check all super-interfaces are already in the list.
      int32_t ifcount = interface->GetIfTableCount();
      for (int32_t j = 0; j < ifcount; j++) {
        mirror::Class* super_interface = interface->GetIfTable()->GetInterface(j);
        DCHECK(ContainsElement(classes_in_iftable, super_interface))
            << "Iftable does not contain " << PrettyClass(super_interface)
            << ", a superinterface of " << PrettyClass(interface);
      }
    }
  }
  if (kIsDebugBuild) {
    // Check that the iftable is ordered correctly.
    for (size_t i = 0; i < filled_ifcount; i++) {
      mirror::Class* if_a = iftable->GetInterface(i);
      for (size_t j = i + 1; j < filled_ifcount; j++) {
        mirror::Class* if_b = iftable->GetInterface(j);
        // !(if_a <: if_b)
        CHECK(!if_b->IsAssignableFrom(if_a))
            << "Bad interface order: " << PrettyClass(if_a) << " (index " << i << ") extends "
            << PrettyClass(if_b) << " (index " << j << ") and so should be after it in the "
            << "interface list.";
      }
    }
  }
  return filled_ifcount;
}

bool ClassLinker::SetupInterfaceLookupTable(Thread* self, Handle<mirror::Class> klass,
                                            Handle<mirror::ObjectArray<mirror::Class>> interfaces) {
  StackHandleScope<1> hs(self);
  const size_t super_ifcount =
      klass->HasSuperClass() ? klass->GetSuperClass()->GetIfTableCount() : 0U;
  const bool have_interfaces = interfaces.Get() != nullptr;
  const size_t num_interfaces =
      have_interfaces ? interfaces->GetLength() : klass->NumDirectInterfaces();
  if (num_interfaces == 0) {
    if (super_ifcount == 0) {
      // Class implements no interfaces.
      DCHECK_EQ(klass->GetIfTableCount(), 0);
      DCHECK(klass->GetIfTable() == nullptr);
      return true;
    }
    // Class implements same interfaces as parent, are any of these not marker interfaces?
    bool has_non_marker_interface = false;
    mirror::IfTable* super_iftable = klass->GetSuperClass()->GetIfTable();
    for (size_t i = 0; i < super_ifcount; ++i) {
      if (super_iftable->GetMethodArrayCount(i) > 0) {
        has_non_marker_interface = true;
        break;
      }
    }
    // Class just inherits marker interfaces from parent so recycle parent's iftable.
    if (!has_non_marker_interface) {
      klass->SetIfTable(super_iftable);
      return true;
    }
  }
  size_t ifcount = super_ifcount + num_interfaces;
  // Check that every class being implemented is an interface.
  for (size_t i = 0; i < num_interfaces; i++) {
    mirror::Class* interface = have_interfaces
        ? interfaces->GetWithoutChecks(i)
        : mirror::Class::GetDirectInterface(self, klass, i);
    DCHECK(interface != nullptr);
    if (UNLIKELY(!interface->IsInterface())) {
      std::string temp;
      ThrowIncompatibleClassChangeError(klass.Get(),
                                        "Class %s implements non-interface class %s",
                                        PrettyDescriptor(klass.Get()).c_str(),
                                        PrettyDescriptor(interface->GetDescriptor(&temp)).c_str());
      return false;
    }
    ifcount += interface->GetIfTableCount();
  }
  // Create the interface function table.
  MutableHandle<mirror::IfTable> iftable(hs.NewHandle(AllocIfTable(self, ifcount)));
  if (UNLIKELY(iftable.Get() == nullptr)) {
    self->AssertPendingOOMException();
    return false;
  }
  // Fill in table with superclass's iftable.
  if (super_ifcount != 0) {
    mirror::IfTable* super_iftable = klass->GetSuperClass()->GetIfTable();
    for (size_t i = 0; i < super_ifcount; i++) {
      mirror::Class* super_interface = super_iftable->GetInterface(i);
      iftable->SetInterface(i, super_interface);
    }
  }

  // Note that AllowThreadSuspension is to thread suspension as pthread_testcancel is to pthread
  // cancellation. That is it will suspend if one has a pending suspend request but otherwise
  // doesn't really do anything.
  self->AllowThreadSuspension();

  size_t new_ifcount;
  {
    ScopedAssertNoThreadSuspension nts(self, "Copying mirror::Class*'s for FillIfTable");
    std::vector<mirror::Class*> to_add;
    for (size_t i = 0; i < num_interfaces; i++) {
      mirror::Class* interface = have_interfaces ? interfaces->Get(i) :
          mirror::Class::GetDirectInterface(self, klass, i);
      to_add.push_back(interface);
    }

    new_ifcount = FillIfTable(iftable.Get(), super_ifcount, std::move(to_add));
  }

  self->AllowThreadSuspension();

  // Shrink iftable in case duplicates were found
  if (new_ifcount < ifcount) {
    DCHECK_NE(num_interfaces, 0U);
    iftable.Assign(down_cast<mirror::IfTable*>(
        iftable->CopyOf(self, new_ifcount * mirror::IfTable::kMax)));
    if (UNLIKELY(iftable.Get() == nullptr)) {
      self->AssertPendingOOMException();
      return false;
    }
    ifcount = new_ifcount;
  } else {
    DCHECK_EQ(new_ifcount, ifcount);
  }
  klass->SetIfTable(iftable.Get());
  return true;
}

// Finds the method with a name/signature that matches cmp in the given list of methods. The list of
// methods must be unique.
static ArtMethod* FindSameNameAndSignature(MethodNameAndSignatureComparator& cmp,
                                           const ScopedArenaVector<ArtMethod*>& list)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  for (ArtMethod* method : list) {
    if (cmp.HasSameNameAndSignature(method)) {
      return method;
    }
  }
  return nullptr;
}

static void SanityCheckVTable(Handle<mirror::Class> klass, uint32_t pointer_size)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  mirror::PointerArray* check_vtable = klass->GetVTableDuringLinking();
  mirror::Class* superclass = (klass->HasSuperClass()) ? klass->GetSuperClass() : nullptr;
  int32_t super_vtable_length = (superclass != nullptr) ? superclass->GetVTableLength() : 0;
  for (int32_t i = 0; i < check_vtable->GetLength(); ++i) {
    ArtMethod* m = check_vtable->GetElementPtrSize<ArtMethod*>(i, pointer_size);
    CHECK(m != nullptr);

    ArraySlice<ArtMethod> virtuals = klass->GetVirtualMethodsSliceUnchecked(pointer_size);
    auto is_same_method = [m] (const ArtMethod& meth) {
      return &meth == m;
    };
    CHECK((super_vtable_length > i && superclass->GetVTableEntry(i, pointer_size) == m) ||
          std::find_if(virtuals.begin(), virtuals.end(), is_same_method) != virtuals.end())
        << "While linking class '" << PrettyClass(klass.Get()) << "' unable to find owning class "
        << "of '" << PrettyMethod(m) << "' (vtable index: " << i << ").";
  }
}

void ClassLinker::FillImtFromSuperClass(Handle<mirror::Class> klass,
                                        ArtMethod* unimplemented_method,
                                        ArtMethod* imt_conflict_method,
                                        ArtMethod** imt) {
  DCHECK(klass->HasSuperClass());
  mirror::Class* super_class = klass->GetSuperClass();
  if (super_class->ShouldHaveEmbeddedImtAndVTable()) {
    for (size_t i = 0; i < mirror::Class::kImtSize; ++i) {
      imt[i] = super_class->GetEmbeddedImTableEntry(i, image_pointer_size_);
    }
  } else {
    // No imt in the super class, need to reconstruct from the iftable.
    mirror::IfTable* if_table = super_class->GetIfTable();
    if (if_table != nullptr) {
      // Ignore copied methods since we will handle these in LinkInterfaceMethods.
      FillIMTFromIfTable(if_table,
                         unimplemented_method,
                         imt_conflict_method,
                         klass.Get(),
                         /*create_conflict_table*/false,
                         /*ignore_copied_methods*/true,
                         /*out*/imt);
    }
  }
}

// TODO This method needs to be split up into several smaller methods.
bool ClassLinker::LinkInterfaceMethods(
    Thread* self,
    Handle<mirror::Class> klass,
    const std::unordered_map<size_t, ClassLinker::MethodTranslation>& default_translations,
    ArtMethod** out_imt) {
  StackHandleScope<3> hs(self);
  Runtime* const runtime = Runtime::Current();

  const bool is_interface = klass->IsInterface();
  const bool has_superclass = klass->HasSuperClass();
  const bool fill_tables = !is_interface;
  const size_t super_ifcount = has_superclass ? klass->GetSuperClass()->GetIfTableCount() : 0U;
  const size_t method_alignment = ArtMethod::Alignment(image_pointer_size_);
  const size_t method_size = ArtMethod::Size(image_pointer_size_);
  const size_t ifcount = klass->GetIfTableCount();

  MutableHandle<mirror::IfTable> iftable(hs.NewHandle(klass->GetIfTable()));

  // These are allocated on the heap to begin, we then transfer to linear alloc when we re-create
  // the virtual methods array.
  // Need to use low 4GB arenas for compiler or else the pointers wont fit in 32 bit method array
  // during cross compilation.
  // Use the linear alloc pool since this one is in the low 4gb for the compiler.
  ArenaStack stack(runtime->GetLinearAlloc()->GetArenaPool());
  ScopedArenaAllocator allocator(&stack);

  ScopedArenaVector<ArtMethod*> default_conflict_methods(allocator.Adapter());
  ScopedArenaVector<ArtMethod*> miranda_methods(allocator.Adapter());
  ScopedArenaVector<ArtMethod*> default_methods(allocator.Adapter());

  MutableHandle<mirror::PointerArray> vtable(hs.NewHandle(klass->GetVTableDuringLinking()));
  ArtMethod* const unimplemented_method = runtime->GetImtUnimplementedMethod();
  ArtMethod* const imt_conflict_method = runtime->GetImtConflictMethod();
  // Copy the IMT from the super class if possible.
  const bool extend_super_iftable = has_superclass;
  if (has_superclass && fill_tables) {
    FillImtFromSuperClass(klass,
                          unimplemented_method,
                          imt_conflict_method,
                          out_imt);
  }
  // Allocate method arrays before since we don't want miss visiting miranda method roots due to
  // thread suspension.
  if (fill_tables) {
    for (size_t i = 0; i < ifcount; ++i) {
      size_t num_methods = iftable->GetInterface(i)->NumDeclaredVirtualMethods();
      if (num_methods > 0) {
        const bool is_super = i < super_ifcount;
        // This is an interface implemented by a super-class. Therefore we can just copy the method
        // array from the superclass.
        const bool super_interface = is_super && extend_super_iftable;
        mirror::PointerArray* method_array;
        if (super_interface) {
          mirror::IfTable* if_table = klass->GetSuperClass()->GetIfTable();
          DCHECK(if_table != nullptr);
          DCHECK(if_table->GetMethodArray(i) != nullptr);
          // If we are working on a super interface, try extending the existing method array.
          method_array = down_cast<mirror::PointerArray*>(if_table->GetMethodArray(i)->Clone(self));
        } else {
          method_array = AllocPointerArray(self, num_methods);
        }
        if (UNLIKELY(method_array == nullptr)) {
          self->AssertPendingOOMException();
          return false;
        }
        iftable->SetMethodArray(i, method_array);
      }
    }
  }

  auto* old_cause = self->StartAssertNoThreadSuspension(
      "Copying ArtMethods for LinkInterfaceMethods");
  // Going in reverse to ensure that we will hit abstract methods that override defaults before the
  // defaults. This means we don't need to do any trickery when creating the Miranda methods, since
  // they will already be null. This has the additional benefit that the declarer of a miranda
  // method will actually declare an abstract method.
  for (size_t i = ifcount; i != 0; ) {
    --i;

    DCHECK_GE(i, 0u);
    DCHECK_LT(i, ifcount);

    size_t num_methods = iftable->GetInterface(i)->NumDeclaredVirtualMethods();
    if (num_methods > 0) {
      StackHandleScope<2> hs2(self);
      const bool is_super = i < super_ifcount;
      const bool super_interface = is_super && extend_super_iftable;
      // We don't actually create or fill these tables for interfaces, we just copy some methods for
      // conflict methods. Just set this as nullptr in those cases.
      Handle<mirror::PointerArray> method_array(fill_tables
                                                ? hs2.NewHandle(iftable->GetMethodArray(i))
                                                : hs2.NewHandle<mirror::PointerArray>(nullptr));

      ArraySlice<ArtMethod> input_virtual_methods;
      ScopedNullHandle<mirror::PointerArray> null_handle;
      Handle<mirror::PointerArray> input_vtable_array(null_handle);
      int32_t input_array_length = 0;

      // TODO Cleanup Needed: In the presence of default methods this optimization is rather dirty
      //      and confusing. Default methods should always look through all the superclasses
      //      because they are the last choice of an implementation. We get around this by looking
      //      at the super-classes iftable methods (copied into method_array previously) when we are
      //      looking for the implementation of a super-interface method but that is rather dirty.
      bool using_virtuals;
      if (super_interface || is_interface) {
        // If we are overwriting a super class interface, try to only virtual methods instead of the
        // whole vtable.
        using_virtuals = true;
        input_virtual_methods = klass->GetDeclaredMethodsSlice(image_pointer_size_);
        input_array_length = input_virtual_methods.size();
      } else {
        // For a new interface, however, we need the whole vtable in case a new
        // interface method is implemented in the whole superclass.
        using_virtuals = false;
        DCHECK(vtable.Get() != nullptr);
        input_vtable_array = vtable;
        input_array_length = input_vtable_array->GetLength();
      }

      // For each method in interface
      for (size_t j = 0; j < num_methods; ++j) {
        auto* interface_method = iftable->GetInterface(i)->GetVirtualMethod(j, image_pointer_size_);
        MethodNameAndSignatureComparator interface_name_comparator(
            interface_method->GetInterfaceMethodIfProxy(image_pointer_size_));
        uint32_t imt_index = GetIMTIndex(interface_method);
        ArtMethod** imt_ptr = &out_imt[imt_index];
        // For each method listed in the interface's method list, find the
        // matching method in our class's method list.  We want to favor the
        // subclass over the superclass, which just requires walking
        // back from the end of the vtable.  (This only matters if the
        // superclass defines a private method and this class redefines
        // it -- otherwise it would use the same vtable slot.  In .dex files
        // those don't end up in the virtual method table, so it shouldn't
        // matter which direction we go.  We walk it backward anyway.)
        //
        // To find defaults we need to do the same but also go over interfaces.
        bool found_impl = false;
        ArtMethod* vtable_impl = nullptr;
        for (int32_t k = input_array_length - 1; k >= 0; --k) {
          ArtMethod* vtable_method = using_virtuals ?
              &input_virtual_methods[k] :
              input_vtable_array->GetElementPtrSize<ArtMethod*>(k, image_pointer_size_);
          ArtMethod* vtable_method_for_name_comparison =
              vtable_method->GetInterfaceMethodIfProxy(image_pointer_size_);
          if (interface_name_comparator.HasSameNameAndSignature(
              vtable_method_for_name_comparison)) {
            if (!vtable_method->IsAbstract() && !vtable_method->IsPublic()) {
              // Must do EndAssertNoThreadSuspension before throw since the throw can cause
              // allocations.
              self->EndAssertNoThreadSuspension(old_cause);
              ThrowIllegalAccessError(klass.Get(),
                  "Method '%s' implementing interface method '%s' is not public",
                  PrettyMethod(vtable_method).c_str(), PrettyMethod(interface_method).c_str());
              return false;
            } else if (UNLIKELY(vtable_method->IsOverridableByDefaultMethod())) {
              // We might have a newer, better, default method for this, so we just skip it. If we
              // are still using this we will select it again when scanning for default methods. To
              // obviate the need to copy the method again we will make a note that we already found
              // a default here.
              // TODO This should be much cleaner.
              vtable_impl = vtable_method;
              break;
            } else {
              found_impl = true;
              if (LIKELY(fill_tables)) {
                method_array->SetElementPtrSize(j, vtable_method, image_pointer_size_);
                // Place method in imt if entry is empty, place conflict otherwise.
                SetIMTRef(unimplemented_method,
                          imt_conflict_method,
                          vtable_method,
                          /*out*/imt_ptr);
              }
              break;
            }
          }
        }
        // Continue on to the next method if we are done.
        if (LIKELY(found_impl)) {
          continue;
        } else if (LIKELY(super_interface)) {
          // Don't look for a default implementation when the super-method is implemented directly
          // by the class.
          //
          // See if we can use the superclasses method and skip searching everything else.
          // Note: !found_impl && super_interface
          CHECK(extend_super_iftable);
          // If this is a super_interface method it is possible we shouldn't override it because a
          // superclass could have implemented it directly.  We get the method the superclass used
          // to implement this to know if we can override it with a default method. Doing this is
          // safe since we know that the super_iftable is filled in so we can simply pull it from
          // there. We don't bother if this is not a super-classes interface since in that case we
          // have scanned the entire vtable anyway and would have found it.
          // TODO This is rather dirty but it is faster than searching through the entire vtable
          //      every time.
          ArtMethod* supers_method =
              method_array->GetElementPtrSize<ArtMethod*>(j, image_pointer_size_);
          DCHECK(supers_method != nullptr);
          DCHECK(interface_name_comparator.HasSameNameAndSignature(supers_method));
          if (LIKELY(!supers_method->IsOverridableByDefaultMethod())) {
            // The method is not overridable by a default method (i.e. it is directly implemented
            // in some class). Therefore move onto the next interface method.
            continue;
          } else {
            // If the super-classes method is override-able by a default method we need to keep
            // track of it since though it is override-able it is not guaranteed to be 'overridden'.
            // If it turns out not to be overridden and we did not keep track of it we might add it
            // to the vtable twice, causing corruption in this class and possibly any subclasses.
            DCHECK(vtable_impl == nullptr || vtable_impl == supers_method)
                << "vtable_impl was " << PrettyMethod(vtable_impl) << " and not 'nullptr' or "
                << PrettyMethod(supers_method) << " as expected. IFTable appears to be corrupt!";
            vtable_impl = supers_method;
          }
        }
        // If we haven't found it yet we should search through the interfaces for default methods.
        ArtMethod* current_method = nullptr;
        switch (FindDefaultMethodImplementation(self,
                                                interface_method,
                                                klass,
                                                /*out*/&current_method)) {
          case DefaultMethodSearchResult::kDefaultConflict: {
            // Default method conflict.
            DCHECK(current_method == nullptr);
            ArtMethod* default_conflict_method = nullptr;
            if (vtable_impl != nullptr && vtable_impl->IsDefaultConflicting()) {
              // We can reuse the method from the superclass, don't bother adding it to virtuals.
              default_conflict_method = vtable_impl;
            } else {
              // See if we already have a conflict method for this method.
              ArtMethod* preexisting_conflict = FindSameNameAndSignature(interface_name_comparator,
                                                                         default_conflict_methods);
              if (LIKELY(preexisting_conflict != nullptr)) {
                // We already have another conflict we can reuse.
                default_conflict_method = preexisting_conflict;
              } else {
                // Note that we do this even if we are an interface since we need to create this and
                // cannot reuse another classes.
                // Create a new conflict method for this to use.
                default_conflict_method =
                    reinterpret_cast<ArtMethod*>(allocator.Alloc(method_size));
                new(default_conflict_method) ArtMethod(interface_method, image_pointer_size_);
                default_conflict_methods.push_back(default_conflict_method);
              }
            }
            current_method = default_conflict_method;
            break;
          }  // case kDefaultConflict
          case DefaultMethodSearchResult::kDefaultFound: {
            DCHECK(current_method != nullptr);
            // Found a default method.
            if (vtable_impl != nullptr &&
                current_method->GetDeclaringClass() == vtable_impl->GetDeclaringClass()) {
              // We found a default method but it was the same one we already have from our
              // superclass. Don't bother adding it to our vtable again.
              current_method = vtable_impl;
            } else if (LIKELY(fill_tables)) {
              // Interfaces don't need to copy default methods since they don't have vtables.
              // Only record this default method if it is new to save space.
              // TODO It might be worthwhile to copy default methods on interfaces anyway since it
              //      would make lookup for interface super much faster. (We would only need to scan
              //      the iftable to find if there is a NSME or AME.)
              ArtMethod* old = FindSameNameAndSignature(interface_name_comparator, default_methods);
              if (old == nullptr) {
                // We found a default method implementation and there were no conflicts.
                // Save the default method. We need to add it to the vtable.
                default_methods.push_back(current_method);
              } else {
                CHECK(old == current_method) << "Multiple default implementations selected!";
              }
            }
            break;
          }  // case kDefaultFound
          case DefaultMethodSearchResult::kAbstractFound: {
            DCHECK(current_method == nullptr);
            // Abstract method masks all defaults.
            if (vtable_impl != nullptr &&
                vtable_impl->IsAbstract() &&
                !vtable_impl->IsDefaultConflicting()) {
              // We need to make this an abstract method but the version in the vtable already is so
              // don't do anything.
              current_method = vtable_impl;
            }
            break;
          }  // case kAbstractFound
        }
        if (LIKELY(fill_tables)) {
          if (current_method == nullptr && !super_interface) {
            // We could not find an implementation for this method and since it is a brand new
            // interface we searched the entire vtable (and all default methods) for an
            // implementation but couldn't find one. We therefore need to make a miranda method.
            //
            // Find out if there is already a miranda method we can use.
            ArtMethod* miranda_method = FindSameNameAndSignature(interface_name_comparator,
                                                                 miranda_methods);
            if (miranda_method == nullptr) {
              DCHECK(interface_method->IsAbstract()) << PrettyMethod(interface_method);
              miranda_method = reinterpret_cast<ArtMethod*>(allocator.Alloc(method_size));
              CHECK(miranda_method != nullptr);
              // Point the interface table at a phantom slot.
              new(miranda_method) ArtMethod(interface_method, image_pointer_size_);
              miranda_methods.push_back(miranda_method);
            }
            current_method = miranda_method;
          }

          if (current_method != nullptr) {
            // We found a default method implementation. Record it in the iftable and IMT.
            method_array->SetElementPtrSize(j, current_method, image_pointer_size_);
            SetIMTRef(unimplemented_method,
                      imt_conflict_method,
                      current_method,
                      /*out*/imt_ptr);
          }
        }
      }  // For each method in interface end.
    }  // if (num_methods > 0)
  }  // For each interface.
  const bool has_new_virtuals = !(miranda_methods.empty() &&
                                  default_methods.empty() &&
                                  default_conflict_methods.empty());
  // TODO don't extend virtuals of interface unless necessary (when is it?).
  if (has_new_virtuals) {
    DCHECK(!is_interface || (default_methods.empty() && miranda_methods.empty()))
        << "Interfaces should only have default-conflict methods appended to them.";
    VLOG(class_linker) << PrettyClass(klass.Get()) << ": miranda_methods=" << miranda_methods.size()
                       << " default_methods=" << default_methods.size()
                       << " default_conflict_methods=" << default_conflict_methods.size();
    const size_t old_method_count = klass->NumMethods();
    const size_t new_method_count = old_method_count +
                                    miranda_methods.size() +
                                    default_methods.size() +
                                    default_conflict_methods.size();
    // Attempt to realloc to save RAM if possible.
    LengthPrefixedArray<ArtMethod>* old_methods = klass->GetMethodsPtr();
    // The Realloced virtual methods aren't visible from the class roots, so there is no issue
    // where GCs could attempt to mark stale pointers due to memcpy. And since we overwrite the
    // realloced memory with out->CopyFrom, we are guaranteed to have objects in the to space since
    // CopyFrom has internal read barriers.
    //
    // TODO We should maybe move some of this into mirror::Class or at least into another method.
    const size_t old_size = LengthPrefixedArray<ArtMethod>::ComputeSize(old_method_count,
                                                                        method_size,
                                                                        method_alignment);
    const size_t new_size = LengthPrefixedArray<ArtMethod>::ComputeSize(new_method_count,
                                                                        method_size,
                                                                        method_alignment);
    const size_t old_methods_ptr_size = (old_methods != nullptr) ? old_size : 0;
    auto* methods = reinterpret_cast<LengthPrefixedArray<ArtMethod>*>(
        runtime->GetLinearAlloc()->Realloc(self, old_methods, old_methods_ptr_size, new_size));
    if (UNLIKELY(methods == nullptr)) {
      self->AssertPendingOOMException();
      self->EndAssertNoThreadSuspension(old_cause);
      return false;
    }
    ScopedArenaUnorderedMap<ArtMethod*, ArtMethod*> move_table(allocator.Adapter());
    if (methods != old_methods) {
      // Maps from heap allocated miranda method to linear alloc miranda method.
      StrideIterator<ArtMethod> out = methods->begin(method_size, method_alignment);
      // Copy over the old methods.
      for (auto& m : klass->GetMethods(image_pointer_size_)) {
        move_table.emplace(&m, &*out);
        // The CopyFrom is only necessary to not miss read barriers since Realloc won't do read
        // barriers when it copies.
        out->CopyFrom(&m, image_pointer_size_);
        ++out;
      }
    }
    StrideIterator<ArtMethod> out(methods->begin(method_size, method_alignment) + old_method_count);
    // Copy over miranda methods before copying vtable since CopyOf may cause thread suspension and
    // we want the roots of the miranda methods to get visited.
    for (ArtMethod* mir_method : miranda_methods) {
      ArtMethod& new_method = *out;
      new_method.CopyFrom(mir_method, image_pointer_size_);
      new_method.SetAccessFlags(new_method.GetAccessFlags() | kAccMiranda | kAccCopied);
      DCHECK_NE(new_method.GetAccessFlags() & kAccAbstract, 0u)
          << "Miranda method should be abstract!";
      move_table.emplace(mir_method, &new_method);
      ++out;
    }
    // We need to copy the default methods into our own method table since the runtime requires that
    // every method on a class's vtable be in that respective class's virtual method table.
    // NOTE This means that two classes might have the same implementation of a method from the same
    // interface but will have different ArtMethod*s for them. This also means we cannot compare a
    // default method found on a class with one found on the declaring interface directly and must
    // look at the declaring class to determine if they are the same.
    for (ArtMethod* def_method : default_methods) {
      ArtMethod& new_method = *out;
      new_method.CopyFrom(def_method, image_pointer_size_);
      // Clear the kAccSkipAccessChecks flag if it is present. Since this class hasn't been verified
      // yet it shouldn't have methods that are skipping access checks.
      // TODO This is rather arbitrary. We should maybe support classes where only some of its
      // methods are skip_access_checks.
      constexpr uint32_t kSetFlags = kAccDefault | kAccCopied;
      constexpr uint32_t kMaskFlags = ~kAccSkipAccessChecks;
      new_method.SetAccessFlags((new_method.GetAccessFlags() | kSetFlags) & kMaskFlags);
      move_table.emplace(def_method, &new_method);
      ++out;
    }
    for (ArtMethod* conf_method : default_conflict_methods) {
      ArtMethod& new_method = *out;
      new_method.CopyFrom(conf_method, image_pointer_size_);
      // This is a type of default method (there are default method impls, just a conflict) so mark
      // this as a default, non-abstract method, since thats what it is. Also clear the
      // kAccSkipAccessChecks bit since this class hasn't been verified yet it shouldn't have
      // methods that are skipping access checks.
      constexpr uint32_t kSetFlags = kAccDefault | kAccDefaultConflict | kAccCopied;
      constexpr uint32_t kMaskFlags = ~(kAccAbstract | kAccSkipAccessChecks);
      new_method.SetAccessFlags((new_method.GetAccessFlags() | kSetFlags) & kMaskFlags);
      DCHECK(new_method.IsDefaultConflicting());
      // The actual method might or might not be marked abstract since we just copied it from a
      // (possibly default) interface method. We need to set it entry point to be the bridge so that
      // the compiler will not invoke the implementation of whatever method we copied from.
      EnsureThrowsInvocationError(&new_method);
      move_table.emplace(conf_method, &new_method);
      ++out;
    }
    methods->SetSize(new_method_count);
    UpdateClassMethods(klass.Get(), methods);
    // Done copying methods, they are all roots in the class now, so we can end the no thread
    // suspension assert.
    self->EndAssertNoThreadSuspension(old_cause);

    if (fill_tables) {
      // Update the vtable to the new method structures. We can skip this for interfaces since they
      // do not have vtables.
      const size_t old_vtable_count = vtable->GetLength();
      const size_t new_vtable_count = old_vtable_count +
                                      miranda_methods.size() +
                                      default_methods.size() +
                                      default_conflict_methods.size();
      vtable.Assign(down_cast<mirror::PointerArray*>(vtable->CopyOf(self, new_vtable_count)));
      if (UNLIKELY(vtable.Get() == nullptr)) {
        self->AssertPendingOOMException();
        return false;
      }
      out = methods->begin(method_size, method_alignment) + old_method_count;
      size_t vtable_pos = old_vtable_count;
      // Update all the newly copied method's indexes so they denote their placement in the vtable.
      for (size_t i = old_method_count; i < new_method_count; ++i) {
        // Leave the declaring class alone the method's dex_code_item_offset_ and dex_method_index_
        // fields are references into the dex file the method was defined in. Since the ArtMethod
        // does not store that information it uses declaring_class_->dex_cache_.
        out->SetMethodIndex(0xFFFF & vtable_pos);
        vtable->SetElementPtrSize(vtable_pos, &*out, image_pointer_size_);
        ++out;
        ++vtable_pos;
      }
      CHECK_EQ(vtable_pos, new_vtable_count);
      // Update old vtable methods. We use the default_translations map to figure out what each
      // vtable entry should be updated to, if they need to be at all.
      for (size_t i = 0; i < old_vtable_count; ++i) {
        ArtMethod* translated_method = vtable->GetElementPtrSize<ArtMethod*>(
              i, image_pointer_size_);
        // Try and find what we need to change this method to.
        auto translation_it = default_translations.find(i);
        bool found_translation = false;
        if (translation_it != default_translations.end()) {
          if (translation_it->second.IsInConflict()) {
            // Find which conflict method we are to use for this method.
            MethodNameAndSignatureComparator old_method_comparator(
                translated_method->GetInterfaceMethodIfProxy(image_pointer_size_));
            ArtMethod* new_conflict_method = FindSameNameAndSignature(old_method_comparator,
                                                                      default_conflict_methods);
            CHECK(new_conflict_method != nullptr) << "Expected a conflict method!";
            translated_method = new_conflict_method;
          } else if (translation_it->second.IsAbstract()) {
            // Find which miranda method we are to use for this method.
            MethodNameAndSignatureComparator old_method_comparator(
                translated_method->GetInterfaceMethodIfProxy(image_pointer_size_));
            ArtMethod* miranda_method = FindSameNameAndSignature(old_method_comparator,
                                                                miranda_methods);
            DCHECK(miranda_method != nullptr);
            translated_method = miranda_method;
          } else {
            // Normal default method (changed from an older default or abstract interface method).
            DCHECK(translation_it->second.IsTranslation());
            translated_method = translation_it->second.GetTranslation();
          }
          found_translation = true;
        }
        DCHECK(translated_method != nullptr);
        auto it = move_table.find(translated_method);
        if (it != move_table.end()) {
          auto* new_method = it->second;
          DCHECK(new_method != nullptr);
          vtable->SetElementPtrSize(i, new_method, image_pointer_size_);
        } else {
          // If it was not going to be updated we wouldn't have put it into the default_translations
          // map.
          CHECK(!found_translation) << "We were asked to update this vtable entry. Must not fail.";
        }
      }
      klass->SetVTable(vtable.Get());

      // Go fix up all the stale iftable pointers.
      for (size_t i = 0; i < ifcount; ++i) {
        for (size_t j = 0, count = iftable->GetMethodArrayCount(i); j < count; ++j) {
          auto* method_array = iftable->GetMethodArray(i);
          auto* m = method_array->GetElementPtrSize<ArtMethod*>(j, image_pointer_size_);
          DCHECK(m != nullptr) << PrettyClass(klass.Get());
          auto it = move_table.find(m);
          if (it != move_table.end()) {
            auto* new_m = it->second;
            DCHECK(new_m != nullptr) << PrettyClass(klass.Get());
            method_array->SetElementPtrSize(j, new_m, image_pointer_size_);
          }
        }
      }

      // Fix up IMT next
      for (size_t i = 0; i < mirror::Class::kImtSize; ++i) {
        auto it = move_table.find(out_imt[i]);
        if (it != move_table.end()) {
          out_imt[i] = it->second;
        }
      }
    }

    // Check that there are no stale methods are in the dex cache array.
    if (kIsDebugBuild) {
      auto* resolved_methods = klass->GetDexCache()->GetResolvedMethods();
      for (size_t i = 0, count = klass->GetDexCache()->NumResolvedMethods(); i < count; ++i) {
        auto* m = mirror::DexCache::GetElementPtrSize(resolved_methods, i, image_pointer_size_);
        CHECK(move_table.find(m) == move_table.end() ||
              // The original versions of copied methods will still be present so allow those too.
              // Note that if the first check passes this might fail to GetDeclaringClass().
              std::find_if(m->GetDeclaringClass()->GetMethods(image_pointer_size_).begin(),
                           m->GetDeclaringClass()->GetMethods(image_pointer_size_).end(),
                           [m] (ArtMethod& meth) {
                             return &meth == m;
                           }) != m->GetDeclaringClass()->GetMethods(image_pointer_size_).end())
            << "Obsolete methods " << PrettyMethod(m) << " is in dex cache!";
      }
    }
    // Put some random garbage in old methods to help find stale pointers.
    if (methods != old_methods && old_methods != nullptr && kIsDebugBuild) {
      // Need to make sure the GC is not running since it could be scanning the methods we are
      // about to overwrite.
      ScopedThreadStateChange tsc(self, kSuspended);
      gc::ScopedGCCriticalSection gcs(self,
                                      gc::kGcCauseClassLinker,
                                      gc::kCollectorTypeClassLinker);
      memset(old_methods, 0xFEu, old_size);
    }
  } else {
    self->EndAssertNoThreadSuspension(old_cause);
  }
  if (kIsDebugBuild && !is_interface) {
    SanityCheckVTable(klass, image_pointer_size_);
  }
  return true;
}

bool ClassLinker::LinkInstanceFields(Thread* self, Handle<mirror::Class> klass) {
  CHECK(klass.Get() != nullptr);
  return LinkFields(self, klass, false, nullptr);
}

bool ClassLinker::LinkStaticFields(Thread* self, Handle<mirror::Class> klass, size_t* class_size) {
  CHECK(klass.Get() != nullptr);
  return LinkFields(self, klass, true, class_size);
}

struct LinkFieldsComparator {
  explicit LinkFieldsComparator() SHARED_REQUIRES(Locks::mutator_lock_) {
  }
  // No thread safety analysis as will be called from STL. Checked lock held in constructor.
  bool operator()(ArtField* field1, ArtField* field2)
      NO_THREAD_SAFETY_ANALYSIS {
    // First come reference fields, then 64-bit, then 32-bit, and then 16-bit, then finally 8-bit.
    Primitive::Type type1 = field1->GetTypeAsPrimitiveType();
    Primitive::Type type2 = field2->GetTypeAsPrimitiveType();
    if (type1 != type2) {
      if (type1 == Primitive::kPrimNot) {
        // Reference always goes first.
        return true;
      }
      if (type2 == Primitive::kPrimNot) {
        // Reference always goes first.
        return false;
      }
      size_t size1 = Primitive::ComponentSize(type1);
      size_t size2 = Primitive::ComponentSize(type2);
      if (size1 != size2) {
        // Larger primitive types go first.
        return size1 > size2;
      }
      // Primitive types differ but sizes match. Arbitrarily order by primitive type.
      return type1 < type2;
    }
    // Same basic group? Then sort by dex field index. This is guaranteed to be sorted
    // by name and for equal names by type id index.
    // NOTE: This works also for proxies. Their static fields are assigned appropriate indexes.
    return field1->GetDexFieldIndex() < field2->GetDexFieldIndex();
  }
};

bool ClassLinker::LinkFields(Thread* self,
                             Handle<mirror::Class> klass,
                             bool is_static,
                             size_t* class_size) {
  self->AllowThreadSuspension();
  const size_t num_fields = is_static ? klass->NumStaticFields() : klass->NumInstanceFields();
  LengthPrefixedArray<ArtField>* const fields = is_static ? klass->GetSFieldsPtr() :
      klass->GetIFieldsPtr();

  // Initialize field_offset
  MemberOffset field_offset(0);
  if (is_static) {
    field_offset = klass->GetFirstReferenceStaticFieldOffsetDuringLinking(image_pointer_size_);
  } else {
    mirror::Class* super_class = klass->GetSuperClass();
    if (super_class != nullptr) {
      CHECK(super_class->IsResolved())
          << PrettyClass(klass.Get()) << " " << PrettyClass(super_class);
      field_offset = MemberOffset(super_class->GetObjectSize());
    }
  }

  CHECK_EQ(num_fields == 0, fields == nullptr) << PrettyClass(klass.Get());

  // we want a relatively stable order so that adding new fields
  // minimizes disruption of C++ version such as Class and Method.
  //
  // The overall sort order order is:
  // 1) All object reference fields, sorted alphabetically.
  // 2) All java long (64-bit) integer fields, sorted alphabetically.
  // 3) All java double (64-bit) floating point fields, sorted alphabetically.
  // 4) All java int (32-bit) integer fields, sorted alphabetically.
  // 5) All java float (32-bit) floating point fields, sorted alphabetically.
  // 6) All java char (16-bit) integer fields, sorted alphabetically.
  // 7) All java short (16-bit) integer fields, sorted alphabetically.
  // 8) All java boolean (8-bit) integer fields, sorted alphabetically.
  // 9) All java byte (8-bit) integer fields, sorted alphabetically.
  //
  // Once the fields are sorted in this order we will attempt to fill any gaps that might be present
  // in the memory layout of the structure. See ShuffleForward for how this is done.
  std::deque<ArtField*> grouped_and_sorted_fields;
  const char* old_no_suspend_cause = self->StartAssertNoThreadSuspension(
      "Naked ArtField references in deque");
  for (size_t i = 0; i < num_fields; i++) {
    grouped_and_sorted_fields.push_back(&fields->At(i));
  }
  std::sort(grouped_and_sorted_fields.begin(), grouped_and_sorted_fields.end(),
            LinkFieldsComparator());

  // References should be at the front.
  size_t current_field = 0;
  size_t num_reference_fields = 0;
  FieldGaps gaps;

  for (; current_field < num_fields; current_field++) {
    ArtField* field = grouped_and_sorted_fields.front();
    Primitive::Type type = field->GetTypeAsPrimitiveType();
    bool isPrimitive = type != Primitive::kPrimNot;
    if (isPrimitive) {
      break;  // past last reference, move on to the next phase
    }
    if (UNLIKELY(!IsAligned<sizeof(mirror::HeapReference<mirror::Object>)>(
        field_offset.Uint32Value()))) {
      MemberOffset old_offset = field_offset;
      field_offset = MemberOffset(RoundUp(field_offset.Uint32Value(), 4));
      AddFieldGap(old_offset.Uint32Value(), field_offset.Uint32Value(), &gaps);
    }
    DCHECK_ALIGNED(field_offset.Uint32Value(), sizeof(mirror::HeapReference<mirror::Object>));
    grouped_and_sorted_fields.pop_front();
    num_reference_fields++;
    field->SetOffset(field_offset);
    field_offset = MemberOffset(field_offset.Uint32Value() +
                                sizeof(mirror::HeapReference<mirror::Object>));
  }
  // Gaps are stored as a max heap which means that we must shuffle from largest to smallest
  // otherwise we could end up with suboptimal gap fills.
  ShuffleForward<8>(&current_field, &field_offset, &grouped_and_sorted_fields, &gaps);
  ShuffleForward<4>(&current_field, &field_offset, &grouped_and_sorted_fields, &gaps);
  ShuffleForward<2>(&current_field, &field_offset, &grouped_and_sorted_fields, &gaps);
  ShuffleForward<1>(&current_field, &field_offset, &grouped_and_sorted_fields, &gaps);
  CHECK(grouped_and_sorted_fields.empty()) << "Missed " << grouped_and_sorted_fields.size() <<
      " fields.";
  self->EndAssertNoThreadSuspension(old_no_suspend_cause);

  // We lie to the GC about the java.lang.ref.Reference.referent field, so it doesn't scan it.
  if (!is_static && klass->DescriptorEquals("Ljava/lang/ref/Reference;")) {
    // We know there are no non-reference fields in the Reference classes, and we know
    // that 'referent' is alphabetically last, so this is easy...
    CHECK_EQ(num_reference_fields, num_fields) << PrettyClass(klass.Get());
    CHECK_STREQ(fields->At(num_fields - 1).GetName(), "referent")
        << PrettyClass(klass.Get());
    --num_reference_fields;
  }

  size_t size = field_offset.Uint32Value();
  // Update klass
  if (is_static) {
    klass->SetNumReferenceStaticFields(num_reference_fields);
    *class_size = size;
  } else {
    klass->SetNumReferenceInstanceFields(num_reference_fields);
    mirror::Class* super_class = klass->GetSuperClass();
    if (num_reference_fields == 0 || super_class == nullptr) {
      // object has one reference field, klass, but we ignore it since we always visit the class.
      // super_class is null iff the class is java.lang.Object.
      if (super_class == nullptr ||
          (super_class->GetClassFlags() & mirror::kClassFlagNoReferenceFields) != 0) {
        klass->SetClassFlags(klass->GetClassFlags() | mirror::kClassFlagNoReferenceFields);
      }
    }
    if (kIsDebugBuild) {
      DCHECK_EQ(super_class == nullptr, klass->DescriptorEquals("Ljava/lang/Object;"));
      size_t total_reference_instance_fields = 0;
      mirror::Class* cur_super = klass.Get();
      while (cur_super != nullptr) {
        total_reference_instance_fields += cur_super->NumReferenceInstanceFieldsDuringLinking();
        cur_super = cur_super->GetSuperClass();
      }
      if (super_class == nullptr) {
        CHECK_EQ(total_reference_instance_fields, 1u) << PrettyDescriptor(klass.Get());
      } else {
        // Check that there is at least num_reference_fields other than Object.class.
        CHECK_GE(total_reference_instance_fields, 1u + num_reference_fields)
            << PrettyClass(klass.Get());
      }
    }
    if (!klass->IsVariableSize()) {
      std::string temp;
      DCHECK_GE(size, sizeof(mirror::Object)) << klass->GetDescriptor(&temp);
      size_t previous_size = klass->GetObjectSize();
      if (previous_size != 0) {
        // Make sure that we didn't originally have an incorrect size.
        CHECK_EQ(previous_size, size) << klass->GetDescriptor(&temp);
      }
      klass->SetObjectSize(size);
    }
  }

  if (kIsDebugBuild) {
    // Make sure that the fields array is ordered by name but all reference
    // offsets are at the beginning as far as alignment allows.
    MemberOffset start_ref_offset = is_static
        ? klass->GetFirstReferenceStaticFieldOffsetDuringLinking(image_pointer_size_)
        : klass->GetFirstReferenceInstanceFieldOffset();
    MemberOffset end_ref_offset(start_ref_offset.Uint32Value() +
                                num_reference_fields *
                                    sizeof(mirror::HeapReference<mirror::Object>));
    MemberOffset current_ref_offset = start_ref_offset;
    for (size_t i = 0; i < num_fields; i++) {
      ArtField* field = &fields->At(i);
      VLOG(class_linker) << "LinkFields: " << (is_static ? "static" : "instance")
          << " class=" << PrettyClass(klass.Get()) << " field=" << PrettyField(field) << " offset="
          << field->GetOffsetDuringLinking();
      if (i != 0) {
        ArtField* const prev_field = &fields->At(i - 1);
        // NOTE: The field names can be the same. This is not possible in the Java language
        // but it's valid Java/dex bytecode and for example proguard can generate such bytecode.
        DCHECK_LE(strcmp(prev_field->GetName(), field->GetName()), 0);
      }
      Primitive::Type type = field->GetTypeAsPrimitiveType();
      bool is_primitive = type != Primitive::kPrimNot;
      if (klass->DescriptorEquals("Ljava/lang/ref/Reference;") &&
          strcmp("referent", field->GetName()) == 0) {
        is_primitive = true;  // We lied above, so we have to expect a lie here.
      }
      MemberOffset offset = field->GetOffsetDuringLinking();
      if (is_primitive) {
        if (offset.Uint32Value() < end_ref_offset.Uint32Value()) {
          // Shuffled before references.
          size_t type_size = Primitive::ComponentSize(type);
          CHECK_LT(type_size, sizeof(mirror::HeapReference<mirror::Object>));
          CHECK_LT(offset.Uint32Value(), start_ref_offset.Uint32Value());
          CHECK_LE(offset.Uint32Value() + type_size, start_ref_offset.Uint32Value());
          CHECK(!IsAligned<sizeof(mirror::HeapReference<mirror::Object>)>(offset.Uint32Value()));
        }
      } else {
        CHECK_EQ(current_ref_offset.Uint32Value(), offset.Uint32Value());
        current_ref_offset = MemberOffset(current_ref_offset.Uint32Value() +
                                          sizeof(mirror::HeapReference<mirror::Object>));
      }
    }
    CHECK_EQ(current_ref_offset.Uint32Value(), end_ref_offset.Uint32Value());
  }
  return true;
}

//  Set the bitmap of reference instance field offsets.
void ClassLinker::CreateReferenceInstanceOffsets(Handle<mirror::Class> klass) {
  uint32_t reference_offsets = 0;
  mirror::Class* super_class = klass->GetSuperClass();
  // Leave the reference offsets as 0 for mirror::Object (the class field is handled specially).
  if (super_class != nullptr) {
    reference_offsets = super_class->GetReferenceInstanceOffsets();
    // Compute reference offsets unless our superclass overflowed.
    if (reference_offsets != mirror::Class::kClassWalkSuper) {
      size_t num_reference_fields = klass->NumReferenceInstanceFieldsDuringLinking();
      if (num_reference_fields != 0u) {
        // All of the fields that contain object references are guaranteed be grouped in memory
        // starting at an appropriately aligned address after super class object data.
        uint32_t start_offset = RoundUp(super_class->GetObjectSize(),
                                        sizeof(mirror::HeapReference<mirror::Object>));
        uint32_t start_bit = (start_offset - mirror::kObjectHeaderSize) /
            sizeof(mirror::HeapReference<mirror::Object>);
        if (start_bit + num_reference_fields > 32) {
          reference_offsets = mirror::Class::kClassWalkSuper;
        } else {
          reference_offsets |= (0xffffffffu << start_bit) &
                               (0xffffffffu >> (32 - (start_bit + num_reference_fields)));
        }
      }
    }
  }
  klass->SetReferenceInstanceOffsets(reference_offsets);
}

mirror::String* ClassLinker::ResolveString(const DexFile& dex_file,
                                           uint32_t string_idx,
                                           Handle<mirror::DexCache> dex_cache) {
  DCHECK(dex_cache.Get() != nullptr);
  mirror::String* resolved = dex_cache->GetResolvedString(string_idx);
  if (resolved != nullptr) {
    return resolved;
  }
  uint32_t utf16_length;
  const char* utf8_data = dex_file.StringDataAndUtf16LengthByIdx(string_idx, &utf16_length);
  mirror::String* string = intern_table_->InternStrong(utf16_length, utf8_data);
  dex_cache->SetResolvedString(string_idx, string);
  return string;
}

mirror::String* ClassLinker::LookupString(const DexFile& dex_file,
                                          uint32_t string_idx,
                                          Handle<mirror::DexCache> dex_cache) {
  DCHECK(dex_cache.Get() != nullptr);
  mirror::String* resolved = dex_cache->GetResolvedString(string_idx);
  if (resolved != nullptr) {
    return resolved;
  }
  uint32_t utf16_length;
  const char* utf8_data = dex_file.StringDataAndUtf16LengthByIdx(string_idx, &utf16_length);
  mirror::String* string = intern_table_->LookupStrong(Thread::Current(), utf16_length, utf8_data);
  if (string != nullptr) {
    dex_cache->SetResolvedString(string_idx, string);
  }
  return string;
}

mirror::Class* ClassLinker::ResolveType(const DexFile& dex_file,
                                        uint16_t type_idx,
                                        mirror::Class* referrer) {
  StackHandleScope<2> hs(Thread::Current());
  Handle<mirror::DexCache> dex_cache(hs.NewHandle(referrer->GetDexCache()));
  Handle<mirror::ClassLoader> class_loader(hs.NewHandle(referrer->GetClassLoader()));
  return ResolveType(dex_file, type_idx, dex_cache, class_loader);
}

mirror::Class* ClassLinker::ResolveType(const DexFile& dex_file,
                                        uint16_t type_idx,
                                        Handle<mirror::DexCache> dex_cache,
                                        Handle<mirror::ClassLoader> class_loader) {
  DCHECK(dex_cache.Get() != nullptr);
  mirror::Class* resolved = dex_cache->GetResolvedType(type_idx);
  if (resolved == nullptr) {
    Thread* self = Thread::Current();
    const char* descriptor = dex_file.StringByTypeIdx(type_idx);
    resolved = FindClass(self, descriptor, class_loader);
    if (resolved != nullptr) {
      // TODO: we used to throw here if resolved's class loader was not the
      //       boot class loader. This was to permit different classes with the
      //       same name to be loaded simultaneously by different loaders
      dex_cache->SetResolvedType(type_idx, resolved);
    } else {
      CHECK(self->IsExceptionPending())
          << "Expected pending exception for failed resolution of: " << descriptor;
      // Convert a ClassNotFoundException to a NoClassDefFoundError.
      StackHandleScope<1> hs(self);
      Handle<mirror::Throwable> cause(hs.NewHandle(self->GetException()));
      if (cause->InstanceOf(GetClassRoot(kJavaLangClassNotFoundException))) {
        DCHECK(resolved == nullptr);  // No Handle needed to preserve resolved.
        self->ClearException();
        ThrowNoClassDefFoundError("Failed resolution of: %s", descriptor);
        self->GetException()->SetCause(cause.Get());
      }
    }
  }
  DCHECK((resolved == nullptr) || resolved->IsResolved() || resolved->IsErroneous())
      << PrettyDescriptor(resolved) << " " << resolved->GetStatus();
  return resolved;
}

template <ClassLinker::ResolveMode kResolveMode>
ArtMethod* ClassLinker::ResolveMethod(const DexFile& dex_file,
                                      uint32_t method_idx,
                                      Handle<mirror::DexCache> dex_cache,
                                      Handle<mirror::ClassLoader> class_loader,
                                      ArtMethod* referrer,
                                      InvokeType type) {
  DCHECK(dex_cache.Get() != nullptr);
  // Check for hit in the dex cache.
  ArtMethod* resolved = dex_cache->GetResolvedMethod(method_idx, image_pointer_size_);
  if (resolved != nullptr && !resolved->IsRuntimeMethod()) {
    DCHECK(resolved->GetDeclaringClassUnchecked() != nullptr) << resolved->GetDexMethodIndex();
    if (kResolveMode == ClassLinker::kForceICCECheck) {
      if (resolved->CheckIncompatibleClassChange(type)) {
        ThrowIncompatibleClassChangeError(type, resolved->GetInvokeType(), resolved, referrer);
        return nullptr;
      }
    }
    return resolved;
  }
  // Fail, get the declaring class.
  const DexFile::MethodId& method_id = dex_file.GetMethodId(method_idx);
  mirror::Class* klass = ResolveType(dex_file, method_id.class_idx_, dex_cache, class_loader);
  if (klass == nullptr) {
    DCHECK(Thread::Current()->IsExceptionPending());
    return nullptr;
  }
  // Scan using method_idx, this saves string compares but will only hit for matching dex
  // caches/files.
  switch (type) {
    case kDirect:  // Fall-through.
    case kStatic:
      resolved = klass->FindDirectMethod(dex_cache.Get(), method_idx, image_pointer_size_);
      DCHECK(resolved == nullptr || resolved->GetDeclaringClassUnchecked() != nullptr);
      break;
    case kInterface:
      // We have to check whether the method id really belongs to an interface (dex static bytecode
      // constraint A15). Otherwise you must not invoke-interface on it.
      //
      // This is not symmetric to A12-A14 (direct, static, virtual), as using FindInterfaceMethod
      // assumes that the given type is an interface, and will check the interface table if the
      // method isn't declared in the class. So it may find an interface method (usually by name
      // in the handling below, but we do the constraint check early). In that case,
      // CheckIncompatibleClassChange will succeed (as it is called on an interface method)
      // unexpectedly.
      // Example:
      //    interface I {
      //      foo()
      //    }
      //    class A implements I {
      //      ...
      //    }
      //    class B extends A {
      //      ...
      //    }
      //    invoke-interface B.foo
      //      -> FindInterfaceMethod finds I.foo (interface method), not A.foo (miranda method)
      if (UNLIKELY(!klass->IsInterface())) {
        ThrowIncompatibleClassChangeError(klass,
                                          "Found class %s, but interface was expected",
                                          PrettyDescriptor(klass).c_str());
        return nullptr;
      } else {
        resolved = klass->FindInterfaceMethod(dex_cache.Get(), method_idx, image_pointer_size_);
        DCHECK(resolved == nullptr || resolved->GetDeclaringClass()->IsInterface());
      }
      break;
    case kSuper:
      if (klass->IsInterface()) {
        resolved = klass->FindInterfaceMethod(dex_cache.Get(), method_idx, image_pointer_size_);
      } else {
        resolved = klass->FindVirtualMethod(dex_cache.Get(), method_idx, image_pointer_size_);
      }
      break;
    case kVirtual:
      resolved = klass->FindVirtualMethod(dex_cache.Get(), method_idx, image_pointer_size_);
      break;
    default:
      LOG(FATAL) << "Unreachable - invocation type: " << type;
      UNREACHABLE();
  }
  if (resolved == nullptr) {
    // Search by name, which works across dex files.
    const char* name = dex_file.StringDataByIdx(method_id.name_idx_);
    const Signature signature = dex_file.GetMethodSignature(method_id);
    switch (type) {
      case kDirect:  // Fall-through.
      case kStatic:
        resolved = klass->FindDirectMethod(name, signature, image_pointer_size_);
        DCHECK(resolved == nullptr || resolved->GetDeclaringClassUnchecked() != nullptr);
        break;
      case kInterface:
        resolved = klass->FindInterfaceMethod(name, signature, image_pointer_size_);
        DCHECK(resolved == nullptr || resolved->GetDeclaringClass()->IsInterface());
        break;
      case kSuper:
        if (klass->IsInterface()) {
          resolved = klass->FindInterfaceMethod(name, signature, image_pointer_size_);
        } else {
          resolved = klass->FindVirtualMethod(name, signature, image_pointer_size_);
        }
        break;
      case kVirtual:
        resolved = klass->FindVirtualMethod(name, signature, image_pointer_size_);
        break;
    }
  }
  // If we found a method, check for incompatible class changes.
  if (LIKELY(resolved != nullptr && !resolved->CheckIncompatibleClassChange(type))) {
    // Be a good citizen and update the dex cache to speed subsequent calls.
    dex_cache->SetResolvedMethod(method_idx, resolved, image_pointer_size_);
    return resolved;
  } else {
    // If we had a method, it's an incompatible-class-change error.
    if (resolved != nullptr) {
      ThrowIncompatibleClassChangeError(type, resolved->GetInvokeType(), resolved, referrer);
    } else {
      // We failed to find the method which means either an access error, an incompatible class
      // change, or no such method. First try to find the method among direct and virtual methods.
      const char* name = dex_file.StringDataByIdx(method_id.name_idx_);
      const Signature signature = dex_file.GetMethodSignature(method_id);
      switch (type) {
        case kDirect:
        case kStatic:
          resolved = klass->FindVirtualMethod(name, signature, image_pointer_size_);
          // Note: kDirect and kStatic are also mutually exclusive, but in that case we would
          //       have had a resolved method before, which triggers the "true" branch above.
          break;
        case kInterface:
        case kVirtual:
        case kSuper:
          resolved = klass->FindDirectMethod(name, signature, image_pointer_size_);
          break;
      }

      // If we found something, check that it can be accessed by the referrer.
      bool exception_generated = false;
      if (resolved != nullptr && referrer != nullptr) {
        mirror::Class* methods_class = resolved->GetDeclaringClass();
        mirror::Class* referring_class = referrer->GetDeclaringClass();
        if (!referring_class->CanAccess(methods_class)) {
          ThrowIllegalAccessErrorClassForMethodDispatch(referring_class,
                                                        methods_class,
                                                        resolved,
                                                        type);
          exception_generated = true;
        } else if (!referring_class->CanAccessMember(methods_class, resolved->GetAccessFlags())) {
          ThrowIllegalAccessErrorMethod(referring_class, resolved);
          exception_generated = true;
        }
      }
      if (!exception_generated) {
        // Otherwise, throw an IncompatibleClassChangeError if we found something, and check
        // interface methods and throw if we find the method there. If we find nothing, throw a
        // NoSuchMethodError.
        switch (type) {
          case kDirect:
          case kStatic:
            if (resolved != nullptr) {
              ThrowIncompatibleClassChangeError(type, kVirtual, resolved, referrer);
            } else {
              resolved = klass->FindInterfaceMethod(name, signature, image_pointer_size_);
              if (resolved != nullptr) {
                ThrowIncompatibleClassChangeError(type, kInterface, resolved, referrer);
              } else {
                ThrowNoSuchMethodError(type, klass, name, signature);
              }
            }
            break;
          case kInterface:
            if (resolved != nullptr) {
              ThrowIncompatibleClassChangeError(type, kDirect, resolved, referrer);
            } else {
              resolved = klass->FindVirtualMethod(name, signature, image_pointer_size_);
              if (resolved != nullptr) {
                ThrowIncompatibleClassChangeError(type, kVirtual, resolved, referrer);
              } else {
                ThrowNoSuchMethodError(type, klass, name, signature);
              }
            }
            break;
          case kSuper:
            if (resolved != nullptr) {
              ThrowIncompatibleClassChangeError(type, kDirect, resolved, referrer);
            } else {
              ThrowNoSuchMethodError(type, klass, name, signature);
            }
            break;
          case kVirtual:
            if (resolved != nullptr) {
              ThrowIncompatibleClassChangeError(type, kDirect, resolved, referrer);
            } else {
              resolved = klass->FindInterfaceMethod(name, signature, image_pointer_size_);
              if (resolved != nullptr) {
                ThrowIncompatibleClassChangeError(type, kInterface, resolved, referrer);
              } else {
                ThrowNoSuchMethodError(type, klass, name, signature);
              }
            }
            break;
        }
      }
    }
    Thread::Current()->AssertPendingException();
    return nullptr;
  }
}

ArtMethod* ClassLinker::ResolveMethodWithoutInvokeType(const DexFile& dex_file,
                                                       uint32_t method_idx,
                                                       Handle<mirror::DexCache> dex_cache,
                                                       Handle<mirror::ClassLoader> class_loader) {
  ArtMethod* resolved = dex_cache->GetResolvedMethod(method_idx, image_pointer_size_);
  if (resolved != nullptr && !resolved->IsRuntimeMethod()) {
    DCHECK(resolved->GetDeclaringClassUnchecked() != nullptr) << resolved->GetDexMethodIndex();
    return resolved;
  }
  // Fail, get the declaring class.
  const DexFile::MethodId& method_id = dex_file.GetMethodId(method_idx);
  mirror::Class* klass = ResolveType(dex_file, method_id.class_idx_, dex_cache, class_loader);
  if (klass == nullptr) {
    Thread::Current()->AssertPendingException();
    return nullptr;
  }
  if (klass->IsInterface()) {
    LOG(FATAL) << "ResolveAmbiguousMethod: unexpected method in interface: " << PrettyClass(klass);
    return nullptr;
  }

  // Search both direct and virtual methods
  resolved = klass->FindDirectMethod(dex_cache.Get(), method_idx, image_pointer_size_);
  if (resolved == nullptr) {
    resolved = klass->FindVirtualMethod(dex_cache.Get(), method_idx, image_pointer_size_);
  }

  return resolved;
}

ArtField* ClassLinker::ResolveField(const DexFile& dex_file,
                                    uint32_t field_idx,
                                    Handle<mirror::DexCache> dex_cache,
                                    Handle<mirror::ClassLoader> class_loader,
                                    bool is_static) {
  DCHECK(dex_cache.Get() != nullptr);
  ArtField* resolved = dex_cache->GetResolvedField(field_idx, image_pointer_size_);
  if (resolved != nullptr) {
    return resolved;
  }
  const DexFile::FieldId& field_id = dex_file.GetFieldId(field_idx);
  Thread* const self = Thread::Current();
  StackHandleScope<1> hs(self);
  Handle<mirror::Class> klass(
      hs.NewHandle(ResolveType(dex_file, field_id.class_idx_, dex_cache, class_loader)));
  if (klass.Get() == nullptr) {
    DCHECK(Thread::Current()->IsExceptionPending());
    return nullptr;
  }

  if (is_static) {
    resolved = mirror::Class::FindStaticField(self, klass, dex_cache.Get(), field_idx);
  } else {
    resolved = klass->FindInstanceField(dex_cache.Get(), field_idx);
  }

  if (resolved == nullptr) {
    const char* name = dex_file.GetFieldName(field_id);
    const char* type = dex_file.GetFieldTypeDescriptor(field_id);
    if (is_static) {
      resolved = mirror::Class::FindStaticField(self, klass, name, type);
    } else {
      resolved = klass->FindInstanceField(name, type);
    }
    if (resolved == nullptr) {
      ThrowNoSuchFieldError(is_static ? "static " : "instance ", klass.Get(), type, name);
      return nullptr;
    }
  }
  dex_cache->SetResolvedField(field_idx, resolved, image_pointer_size_);
  return resolved;
}

ArtField* ClassLinker::ResolveFieldJLS(const DexFile& dex_file,
                                       uint32_t field_idx,
                                       Handle<mirror::DexCache> dex_cache,
                                       Handle<mirror::ClassLoader> class_loader) {
  DCHECK(dex_cache.Get() != nullptr);
  ArtField* resolved = dex_cache->GetResolvedField(field_idx, image_pointer_size_);
  if (resolved != nullptr) {
    return resolved;
  }
  const DexFile::FieldId& field_id = dex_file.GetFieldId(field_idx);
  Thread* self = Thread::Current();
  StackHandleScope<1> hs(self);
  Handle<mirror::Class> klass(
      hs.NewHandle(ResolveType(dex_file, field_id.class_idx_, dex_cache, class_loader)));
  if (klass.Get() == nullptr) {
    DCHECK(Thread::Current()->IsExceptionPending());
    return nullptr;
  }

  StringPiece name(dex_file.StringDataByIdx(field_id.name_idx_));
  StringPiece type(dex_file.StringDataByIdx(
      dex_file.GetTypeId(field_id.type_idx_).descriptor_idx_));
  resolved = mirror::Class::FindField(self, klass, name, type);
  if (resolved != nullptr) {
    dex_cache->SetResolvedField(field_idx, resolved, image_pointer_size_);
  } else {
    ThrowNoSuchFieldError("", klass.Get(), type, name);
  }
  return resolved;
}

const char* ClassLinker::MethodShorty(uint32_t method_idx,
                                      ArtMethod* referrer,
                                      uint32_t* length) {
  mirror::Class* declaring_class = referrer->GetDeclaringClass();
  mirror::DexCache* dex_cache = declaring_class->GetDexCache();
  const DexFile& dex_file = *dex_cache->GetDexFile();
  const DexFile::MethodId& method_id = dex_file.GetMethodId(method_idx);
  return dex_file.GetMethodShorty(method_id, length);
}

class DumpClassVisitor : public ClassVisitor {
 public:
  explicit DumpClassVisitor(int flags) : flags_(flags) {}

  bool operator()(mirror::Class* klass) OVERRIDE SHARED_REQUIRES(Locks::mutator_lock_) {
    klass->DumpClass(LOG(ERROR), flags_);
    return true;
  }

 private:
  const int flags_;
};

void ClassLinker::DumpAllClasses(int flags) {
  DumpClassVisitor visitor(flags);
  VisitClasses(&visitor);
}

static OatFile::OatMethod CreateOatMethod(const void* code) {
  CHECK(code != nullptr);
  const uint8_t* base = reinterpret_cast<const uint8_t*>(code);  // Base of data points at code.
  base -= sizeof(void*);  // Move backward so that code_offset != 0.
  const uint32_t code_offset = sizeof(void*);
  return OatFile::OatMethod(base, code_offset);
}

bool ClassLinker::IsQuickResolutionStub(const void* entry_point) const {
  return (entry_point == GetQuickResolutionStub()) ||
      (quick_resolution_trampoline_ == entry_point);
}

bool ClassLinker::IsQuickToInterpreterBridge(const void* entry_point) const {
  return (entry_point == GetQuickToInterpreterBridge()) ||
      (quick_to_interpreter_bridge_trampoline_ == entry_point);
}

bool ClassLinker::IsQuickGenericJniStub(const void* entry_point) const {
  return (entry_point == GetQuickGenericJniStub()) ||
      (quick_generic_jni_trampoline_ == entry_point);
}

const void* ClassLinker::GetRuntimeQuickGenericJniStub() const {
  return GetQuickGenericJniStub();
}

void ClassLinker::SetEntryPointsToCompiledCode(ArtMethod* method,
                                               const void* method_code) const {
  OatFile::OatMethod oat_method = CreateOatMethod(method_code);
  oat_method.LinkMethod(method);
}

void ClassLinker::SetEntryPointsToInterpreter(ArtMethod* method) const {
  if (!method->IsNative()) {
    method->SetEntryPointFromQuickCompiledCode(GetQuickToInterpreterBridge());
  } else {
    const void* quick_method_code = GetQuickGenericJniStub();
    OatFile::OatMethod oat_method = CreateOatMethod(quick_method_code);
    oat_method.LinkMethod(method);
  }
}

void ClassLinker::DumpForSigQuit(std::ostream& os) {
  ScopedObjectAccess soa(Thread::Current());
  if (dex_cache_boot_image_class_lookup_required_) {
    AddBootImageClassesToClassTable();
  }
  ReaderMutexLock mu(soa.Self(), *Locks::classlinker_classes_lock_);
  os << "Zygote loaded classes=" << NumZygoteClasses() << " post zygote classes="
     << NumNonZygoteClasses() << "\n";
}

class CountClassesVisitor : public ClassLoaderVisitor {
 public:
  CountClassesVisitor() : num_zygote_classes(0), num_non_zygote_classes(0) {}

  void Visit(mirror::ClassLoader* class_loader)
      SHARED_REQUIRES(Locks::classlinker_classes_lock_, Locks::mutator_lock_) OVERRIDE {
    ClassTable* const class_table = class_loader->GetClassTable();
    if (class_table != nullptr) {
      num_zygote_classes += class_table->NumZygoteClasses();
      num_non_zygote_classes += class_table->NumNonZygoteClasses();
    }
  }

  size_t num_zygote_classes;
  size_t num_non_zygote_classes;
};

size_t ClassLinker::NumZygoteClasses() const {
  CountClassesVisitor visitor;
  VisitClassLoaders(&visitor);
  return visitor.num_zygote_classes + boot_class_table_.NumZygoteClasses();
}

size_t ClassLinker::NumNonZygoteClasses() const {
  CountClassesVisitor visitor;
  VisitClassLoaders(&visitor);
  return visitor.num_non_zygote_classes + boot_class_table_.NumNonZygoteClasses();
}

size_t ClassLinker::NumLoadedClasses() {
  if (dex_cache_boot_image_class_lookup_required_) {
    AddBootImageClassesToClassTable();
  }
  ReaderMutexLock mu(Thread::Current(), *Locks::classlinker_classes_lock_);
  // Only return non zygote classes since these are the ones which apps which care about.
  return NumNonZygoteClasses();
}

pid_t ClassLinker::GetClassesLockOwner() {
  return Locks::classlinker_classes_lock_->GetExclusiveOwnerTid();
}

pid_t ClassLinker::GetDexLockOwner() {
  return dex_lock_.GetExclusiveOwnerTid();
}

void ClassLinker::SetClassRoot(ClassRoot class_root, mirror::Class* klass) {
  DCHECK(!init_done_);

  DCHECK(klass != nullptr);
  DCHECK(klass->GetClassLoader() == nullptr);

  mirror::ObjectArray<mirror::Class>* class_roots = class_roots_.Read();
  DCHECK(class_roots != nullptr);
  DCHECK(class_roots->Get(class_root) == nullptr);
  class_roots->Set<false>(class_root, klass);
}

const char* ClassLinker::GetClassRootDescriptor(ClassRoot class_root) {
  static const char* class_roots_descriptors[] = {
    "Ljava/lang/Class;",
    "Ljava/lang/Object;",
    "[Ljava/lang/Class;",
    "[Ljava/lang/Object;",
    "Ljava/lang/String;",
    "Ljava/lang/DexCache;",
    "Ljava/lang/ref/Reference;",
    "Ljava/lang/reflect/Constructor;",
    "Ljava/lang/reflect/Field;",
    "Ljava/lang/reflect/Method;",
    "Ljava/lang/reflect/Proxy;",
    "[Ljava/lang/String;",
    "[Ljava/lang/reflect/Constructor;",
    "[Ljava/lang/reflect/Field;",
    "[Ljava/lang/reflect/Method;",
    "Ljava/lang/ClassLoader;",
    "Ljava/lang/Throwable;",
    "Ljava/lang/ClassNotFoundException;",
    "Ljava/lang/StackTraceElement;",
    "Z",
    "B",
    "C",
    "D",
    "F",
    "I",
    "J",
    "S",
    "V",
    "[Z",
    "[B",
    "[C",
    "[D",
    "[F",
    "[I",
    "[J",
    "[S",
    "[Ljava/lang/StackTraceElement;",
  };
  static_assert(arraysize(class_roots_descriptors) == size_t(kClassRootsMax),
                "Mismatch between class descriptors and class-root enum");

  const char* descriptor = class_roots_descriptors[class_root];
  CHECK(descriptor != nullptr);
  return descriptor;
}

jobject ClassLinker::CreatePathClassLoader(Thread* self,
                                           const std::vector<const DexFile*>& dex_files) {
  // SOAAlreadyRunnable is protected, and we need something to add a global reference.
  // We could move the jobject to the callers, but all call-sites do this...
  ScopedObjectAccessUnchecked soa(self);

  // For now, create a libcore-level DexFile for each ART DexFile. This "explodes" multidex.
  StackHandleScope<10> hs(self);

  ArtField* dex_elements_field =
      soa.DecodeField(WellKnownClasses::dalvik_system_DexPathList_dexElements);

  mirror::Class* dex_elements_class = dex_elements_field->GetType<true>();
  DCHECK(dex_elements_class != nullptr);
  DCHECK(dex_elements_class->IsArrayClass());
  Handle<mirror::ObjectArray<mirror::Object>> h_dex_elements(hs.NewHandle(
      mirror::ObjectArray<mirror::Object>::Alloc(self, dex_elements_class, dex_files.size())));
  Handle<mirror::Class> h_dex_element_class =
      hs.NewHandle(dex_elements_class->GetComponentType());

  ArtField* element_file_field =
      soa.DecodeField(WellKnownClasses::dalvik_system_DexPathList__Element_dexFile);
  DCHECK_EQ(h_dex_element_class.Get(), element_file_field->GetDeclaringClass());

  ArtField* cookie_field = soa.DecodeField(WellKnownClasses::dalvik_system_DexFile_cookie);
  DCHECK_EQ(cookie_field->GetDeclaringClass(), element_file_field->GetType<false>());

  ArtField* file_name_field = soa.DecodeField(WellKnownClasses::dalvik_system_DexFile_fileName);
  DCHECK_EQ(file_name_field->GetDeclaringClass(), element_file_field->GetType<false>());

  // Fill the elements array.
  int32_t index = 0;
  for (const DexFile* dex_file : dex_files) {
    StackHandleScope<4> hs2(self);

    // CreatePathClassLoader is only used by gtests. Index 0 of h_long_array is supposed to be the
    // oat file but we can leave it null.
    Handle<mirror::LongArray> h_long_array = hs2.NewHandle(mirror::LongArray::Alloc(
        self,
        kDexFileIndexStart + 1));
    DCHECK(h_long_array.Get() != nullptr);
    h_long_array->Set(kDexFileIndexStart, reinterpret_cast<intptr_t>(dex_file));

    Handle<mirror::Object> h_dex_file = hs2.NewHandle(
        cookie_field->GetDeclaringClass()->AllocObject(self));
    DCHECK(h_dex_file.Get() != nullptr);
    cookie_field->SetObject<false>(h_dex_file.Get(), h_long_array.Get());

    Handle<mirror::String> h_file_name = hs2.NewHandle(
        mirror::String::AllocFromModifiedUtf8(self, dex_file->GetLocation().c_str()));
    DCHECK(h_file_name.Get() != nullptr);
    file_name_field->SetObject<false>(h_dex_file.Get(), h_file_name.Get());

    Handle<mirror::Object> h_element = hs2.NewHandle(h_dex_element_class->AllocObject(self));
    DCHECK(h_element.Get() != nullptr);
    element_file_field->SetObject<false>(h_element.Get(), h_dex_file.Get());

    h_dex_elements->Set(index, h_element.Get());
    index++;
  }
  DCHECK_EQ(index, h_dex_elements->GetLength());

  // Create DexPathList.
  Handle<mirror::Object> h_dex_path_list = hs.NewHandle(
      dex_elements_field->GetDeclaringClass()->AllocObject(self));
  DCHECK(h_dex_path_list.Get() != nullptr);
  // Set elements.
  dex_elements_field->SetObject<false>(h_dex_path_list.Get(), h_dex_elements.Get());

  // Create PathClassLoader.
  Handle<mirror::Class> h_path_class_class = hs.NewHandle(
      soa.Decode<mirror::Class*>(WellKnownClasses::dalvik_system_PathClassLoader));
  Handle<mirror::Object> h_path_class_loader = hs.NewHandle(
      h_path_class_class->AllocObject(self));
  DCHECK(h_path_class_loader.Get() != nullptr);
  // Set DexPathList.
  ArtField* path_list_field =
      soa.DecodeField(WellKnownClasses::dalvik_system_PathClassLoader_pathList);
  DCHECK(path_list_field != nullptr);
  path_list_field->SetObject<false>(h_path_class_loader.Get(), h_dex_path_list.Get());

  // Make a pretend boot-classpath.
  // TODO: Should we scan the image?
  ArtField* const parent_field =
      mirror::Class::FindField(self, hs.NewHandle(h_path_class_loader->GetClass()), "parent",
                               "Ljava/lang/ClassLoader;");
  DCHECK(parent_field != nullptr);
  mirror::Object* boot_cl =
      soa.Decode<mirror::Class*>(WellKnownClasses::java_lang_BootClassLoader)->AllocObject(self);
  parent_field->SetObject<false>(h_path_class_loader.Get(), boot_cl);

  // Make it a global ref and return.
  ScopedLocalRef<jobject> local_ref(
      soa.Env(), soa.Env()->AddLocalReference<jobject>(h_path_class_loader.Get()));
  return soa.Env()->NewGlobalRef(local_ref.get());
}

ArtMethod* ClassLinker::CreateRuntimeMethod(LinearAlloc* linear_alloc) {
  const size_t method_alignment = ArtMethod::Alignment(image_pointer_size_);
  const size_t method_size = ArtMethod::Size(image_pointer_size_);
  LengthPrefixedArray<ArtMethod>* method_array = AllocArtMethodArray(
      Thread::Current(),
      linear_alloc,
      1);
  ArtMethod* method = &method_array->At(0, method_size, method_alignment);
  CHECK(method != nullptr);
  method->SetDexMethodIndex(DexFile::kDexNoIndex);
  CHECK(method->IsRuntimeMethod());
  return method;
}

void ClassLinker::DropFindArrayClassCache() {
  std::fill_n(find_array_class_cache_, kFindArrayCacheSize, GcRoot<mirror::Class>(nullptr));
  find_array_class_cache_next_victim_ = 0;
}

void ClassLinker::ClearClassTableStrongRoots() const {
  Thread* const self = Thread::Current();
  WriterMutexLock mu(self, *Locks::classlinker_classes_lock_);
  for (const ClassLoaderData& data : class_loaders_) {
    if (data.class_table != nullptr) {
      data.class_table->ClearStrongRoots();
    }
  }
}

void ClassLinker::VisitClassLoaders(ClassLoaderVisitor* visitor) const {
  Thread* const self = Thread::Current();
  for (const ClassLoaderData& data : class_loaders_) {
    // Need to use DecodeJObject so that we get null for cleared JNI weak globals.
    auto* const class_loader = down_cast<mirror::ClassLoader*>(self->DecodeJObject(data.weak_root));
    if (class_loader != nullptr) {
      visitor->Visit(class_loader);
    }
  }
}

void ClassLinker::InsertDexFileInToClassLoader(mirror::Object* dex_file,
                                               mirror::ClassLoader* class_loader) {
  DCHECK(dex_file != nullptr);
  Thread* const self = Thread::Current();
  WriterMutexLock mu(self, *Locks::classlinker_classes_lock_);
  ClassTable* const table = ClassTableForClassLoader(class_loader);
  DCHECK(table != nullptr);
  if (table->InsertStrongRoot(dex_file) && class_loader != nullptr) {
    // It was not already inserted, perform the write barrier to let the GC know the class loader's
    // class table was modified.
    Runtime::Current()->GetHeap()->WriteBarrierEveryFieldOf(class_loader);
  }
}

void ClassLinker::CleanupClassLoaders() {
  Thread* const self = Thread::Current();
  WriterMutexLock mu(self, *Locks::classlinker_classes_lock_);
  for (auto it = class_loaders_.begin(); it != class_loaders_.end(); ) {
    const ClassLoaderData& data = *it;
    // Need to use DecodeJObject so that we get null for cleared JNI weak globals.
    auto* const class_loader = down_cast<mirror::ClassLoader*>(self->DecodeJObject(data.weak_root));
    if (class_loader != nullptr) {
      ++it;
    } else {
      VLOG(class_linker) << "Freeing class loader";
      DeleteClassLoader(self, data);
      it = class_loaders_.erase(it);
    }
  }
}

std::set<DexCacheResolvedClasses> ClassLinker::GetResolvedClasses(bool ignore_boot_classes) {
  ScopedTrace trace(__PRETTY_FUNCTION__);
  ScopedObjectAccess soa(Thread::Current());
  ScopedAssertNoThreadSuspension ants(soa.Self(), __FUNCTION__);
  std::set<DexCacheResolvedClasses> ret;
  VLOG(class_linker) << "Collecting resolved classes";
  const uint64_t start_time = NanoTime();
  ReaderMutexLock mu(soa.Self(), *DexLock());
  // Loop through all the dex caches and inspect resolved classes.
  for (const ClassLinker::DexCacheData& data : GetDexCachesData()) {
    if (soa.Self()->IsJWeakCleared(data.weak_root)) {
      continue;
    }
    mirror::DexCache* dex_cache =
        down_cast<mirror::DexCache*>(soa.Self()->DecodeJObject(data.weak_root));
    if (dex_cache == nullptr) {
      continue;
    }
    const DexFile* dex_file = dex_cache->GetDexFile();
    const std::string& location = dex_file->GetLocation();
    const size_t num_class_defs = dex_file->NumClassDefs();
    // Use the resolved types, this will miss array classes.
    const size_t num_types = dex_file->NumTypeIds();
    VLOG(class_linker) << "Collecting class profile for dex file " << location
                       << " types=" << num_types << " class_defs=" << num_class_defs;
    DexCacheResolvedClasses resolved_classes(dex_file->GetLocation(),
                                             dex_file->GetBaseLocation(),
                                             dex_file->GetLocationChecksum());
    size_t num_resolved = 0;
    std::unordered_set<uint16_t> class_set;
    CHECK_EQ(num_types, dex_cache->NumResolvedTypes());
    for (size_t i = 0; i < num_types; ++i) {
      mirror::Class* klass = dex_cache->GetResolvedType(i);
      // Filter out null class loader since that is the boot class loader.
      if (klass == nullptr || (ignore_boot_classes && klass->GetClassLoader() == nullptr)) {
        continue;
      }
      ++num_resolved;
      DCHECK(!klass->IsProxyClass());
      if (!klass->IsResolved()) {
        DCHECK(klass->IsErroneous());
        continue;
      }
      mirror::DexCache* klass_dex_cache = klass->GetDexCache();
      if (klass_dex_cache == dex_cache) {
        const size_t class_def_idx = klass->GetDexClassDefIndex();
        DCHECK(klass->IsResolved());
        CHECK_LT(class_def_idx, num_class_defs);
        class_set.insert(class_def_idx);
      }
    }

    if (!class_set.empty()) {
      auto it = ret.find(resolved_classes);
      if (it != ret.end()) {
        // Already have the key, union the class def idxs.
        it->AddClasses(class_set.begin(), class_set.end());
      } else {
        resolved_classes.AddClasses(class_set.begin(), class_set.end());
        ret.insert(resolved_classes);
      }
    }

    VLOG(class_linker) << "Dex location " << location << " has " << num_resolved << " / "
                       << num_class_defs << " resolved classes";
  }
  VLOG(class_linker) << "Collecting class profile took " << PrettyDuration(NanoTime() - start_time);
  return ret;
}

std::unordered_set<std::string> ClassLinker::GetClassDescriptorsForProfileKeys(
    const std::set<DexCacheResolvedClasses>& classes) {
  ScopedTrace trace(__PRETTY_FUNCTION__);
  std::unordered_set<std::string> ret;
  Thread* const self = Thread::Current();
  std::unordered_map<std::string, const DexFile*> location_to_dex_file;
  ScopedObjectAccess soa(self);
  ScopedAssertNoThreadSuspension ants(soa.Self(), __FUNCTION__);
  ReaderMutexLock mu(self, *DexLock());
  for (const ClassLinker::DexCacheData& data : GetDexCachesData()) {
    if (!self->IsJWeakCleared(data.weak_root)) {
      mirror::DexCache* dex_cache =
          down_cast<mirror::DexCache*>(soa.Self()->DecodeJObject(data.weak_root));
      if (dex_cache != nullptr) {
        const DexFile* dex_file = dex_cache->GetDexFile();
        // There could be duplicates if two dex files with the same location are mapped.
        location_to_dex_file.emplace(
            ProfileCompilationInfo::GetProfileDexFileKey(dex_file->GetLocation()), dex_file);
      }
    }
  }
  for (const DexCacheResolvedClasses& info : classes) {
    const std::string& profile_key = info.GetDexLocation();
    auto found = location_to_dex_file.find(profile_key);
    if (found != location_to_dex_file.end()) {
      const DexFile* dex_file = found->second;
      VLOG(profiler) << "Found opened dex file for " << dex_file->GetLocation() << " with "
                     << info.GetClasses().size() << " classes";
      DCHECK_EQ(dex_file->GetLocationChecksum(), info.GetLocationChecksum());
      for (uint16_t class_def_idx : info.GetClasses()) {
        if (class_def_idx >= dex_file->NumClassDefs()) {
          LOG(WARNING) << "Class def index " << class_def_idx << " >= " << dex_file->NumClassDefs();
          continue;
        }
        const DexFile::TypeId& type_id = dex_file->GetTypeId(
            dex_file->GetClassDef(class_def_idx).class_idx_);
        const char* descriptor = dex_file->GetTypeDescriptor(type_id);
        ret.insert(descriptor);
      }
    } else {
      VLOG(class_linker) << "Failed to find opened dex file for profile key " << profile_key;
    }
  }
  return ret;
}

// Instantiate ResolveMethod.
template ArtMethod* ClassLinker::ResolveMethod<ClassLinker::kForceICCECheck>(
    const DexFile& dex_file,
    uint32_t method_idx,
    Handle<mirror::DexCache> dex_cache,
    Handle<mirror::ClassLoader> class_loader,
    ArtMethod* referrer,
    InvokeType type);
template ArtMethod* ClassLinker::ResolveMethod<ClassLinker::kNoICCECheckForCache>(
    const DexFile& dex_file,
    uint32_t method_idx,
    Handle<mirror::DexCache> dex_cache,
    Handle<mirror::ClassLoader> class_loader,
    ArtMethod* referrer,
    InvokeType type);

}  // namespace art
