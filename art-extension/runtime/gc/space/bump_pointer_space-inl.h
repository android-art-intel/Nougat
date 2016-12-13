/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef ART_RUNTIME_GC_SPACE_BUMP_POINTER_SPACE_INL_H_
#define ART_RUNTIME_GC_SPACE_BUMP_POINTER_SPACE_INL_H_

#include "base/bit_utils.h"
#include "bump_pointer_space.h"
#include "mirror/object-inl.h"
#include "well_known_classes.h"

namespace art {
namespace gc {
namespace space {

inline mirror::Object* BumpPointerSpace::AllocNonvirtualWithoutAccounting(size_t num_bytes) {
  DCHECK_ALIGNED(num_bytes, kAlignment);
  uint8_t* old_end;
  uint8_t* new_end;
  do {
    old_end = end_.LoadRelaxed();
    new_end = old_end + num_bytes;
    // If there is no more room in the region, we are out of memory.
    if (UNLIKELY(new_end > growth_end_)) {
      return nullptr;
    }
  } while (!end_.CompareExchangeWeakSequentiallyConsistent(old_end, new_end));
  return reinterpret_cast<mirror::Object*>(old_end);
}

inline mirror::Object* BumpPointerSpace::AllocNonvirtual(size_t num_bytes,
                                                         size_t* bytes_allocated,
                                                         size_t* usable_size,
                                                         size_t* bytes_tl_bulk_allocated) {
  mirror::Object* ret = AllocNonvirtualWithoutAccounting(num_bytes);
  if (LIKELY(ret != nullptr)) {
    objects_allocated_.FetchAndAddSequentiallyConsistent(1);
    bytes_allocated_.FetchAndAddSequentiallyConsistent(num_bytes);
    *bytes_allocated = num_bytes;
    if (usable_size != nullptr) {
      *usable_size = num_bytes;
    }
    *bytes_tl_bulk_allocated = num_bytes;
  }
  return ret;
}

inline mirror::Object* BumpPointerSpace::Alloc(Thread*, size_t num_bytes,
                                               size_t* bytes_allocated,
                                               size_t* usable_size,
                                               size_t* bytes_tl_bulk_allocated) {
  num_bytes = RoundUp(num_bytes, kAlignment);
  return AllocNonvirtual(num_bytes, bytes_allocated, usable_size, bytes_tl_bulk_allocated);
}

inline mirror::Object* BumpPointerSpace::AllocThreadUnsafe(Thread* self, size_t num_bytes,
                                                           size_t* bytes_allocated,
                                                           size_t* usable_size,
                                                           size_t* bytes_tl_bulk_allocated) {
  Locks::mutator_lock_->AssertExclusiveHeld(self);
  num_bytes = RoundUp(num_bytes, kAlignment);
  uint8_t* end = end_.LoadRelaxed();
  if (end + num_bytes > growth_end_) {
    return nullptr;
  }
  mirror::Object* obj = reinterpret_cast<mirror::Object*>(end);
  end_.StoreRelaxed(end + num_bytes);
  *bytes_allocated = num_bytes;
  // Use the CAS free versions as an optimization.
  objects_allocated_.StoreRelaxed(objects_allocated_.LoadRelaxed() + 1);
  bytes_allocated_.StoreRelaxed(bytes_allocated_.LoadRelaxed() + num_bytes);
  if (UNLIKELY(usable_size != nullptr)) {
    *usable_size = num_bytes;
  }
  *bytes_tl_bulk_allocated = num_bytes;
  return obj;
}

inline size_t BumpPointerSpace::AllocationSizeNonvirtual(mirror::Object* obj, size_t* usable_size)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  size_t num_bytes = obj->SizeOf();
  if (usable_size != nullptr) {
    *usable_size = RoundUp(num_bytes, kAlignment);
  }
  return num_bytes;
}

inline void BumpPointerSpace::AccountAllocation(size_t num_objects) {
  objects_allocated_.FetchAndAddSequentiallyConsistent(num_objects);
}

// Fill the given memory block with a dummy object.
// Use to fill in a copy of object that was lost in race.
inline void BumpPointerSpace::FillWithDummyObject(mirror::Object* dummy_obj, size_t byte_size) {
  DCHECK_ALIGNED(byte_size, kAlignment);
  memset(dummy_obj, 0, byte_size);
  mirror::Class* int_array_class = mirror::IntArray::GetArrayClass();
  DCHECK(int_array_class != nullptr);
  size_t component_size = int_array_class->GetComponentSize();
  DCHECK_EQ(component_size, sizeof(int32_t));
  size_t data_offset = mirror::Array::DataOffset(component_size).SizeValue();
  if (data_offset > byte_size) {
    // An int array is too big. Use java.lang.Object.
    mirror::Class* java_lang_Object = WellKnownClasses::ToClass(WellKnownClasses::java_lang_Object);
    DCHECK_EQ(byte_size, java_lang_Object->GetObjectSize());
    dummy_obj->SetClass(java_lang_Object);
    DCHECK_EQ(byte_size, dummy_obj->SizeOf());
  } else {
    // Use an int array.
    dummy_obj->SetClass(int_array_class);
    DCHECK(dummy_obj->IsArrayInstance());
    int32_t length = (byte_size - data_offset) / component_size;
    dummy_obj->AsArray()->SetLength(length);
    DCHECK_EQ(dummy_obj->AsArray()->GetLength(), length)
        << " byte_size=" << byte_size << " length=" << length
        << " component_size=" << component_size << " data_offset=" << data_offset;
    DCHECK_EQ(byte_size, dummy_obj->SizeOf())
        << " byte_size=" << byte_size << " length=" << length
        << " component_size=" << component_size << " data_offset=" << data_offset;
  }
}

}  // namespace space
}  // namespace gc
}  // namespace art

#endif  // ART_RUNTIME_GC_SPACE_BUMP_POINTER_SPACE_INL_H_
