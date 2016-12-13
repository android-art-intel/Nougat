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

#ifndef ART_RUNTIME_GC_SPACE_BUMP_POINTER_SPACE_H_
#define ART_RUNTIME_GC_SPACE_BUMP_POINTER_SPACE_H_

#include "object_callbacks.h"
#include "space.h"
#include "gc/accounting/aging_table.h"

namespace art {
namespace gc {

namespace collector {
  class MarkSweep;
}  // namespace collector

namespace space {

// A bump pointer space allocates by incrementing a pointer, it doesn't provide a free
// implementation as its intended to be evacuated.
class BumpPointerSpace FINAL : public ContinuousMemMapAllocSpace {
 public:
  typedef void(*WalkCallback)(void *start, void *end, size_t num_bytes, void* callback_arg);

  SpaceType GetType() const OVERRIDE {
    return kSpaceTypeBumpPointerSpace;
  }

  // Create a bump pointer space with the requested sizes. The requested base address is not
  // guaranteed to be granted, if it is required, the caller should call Begin on the returned
  // space to confirm the request was granted.
  static BumpPointerSpace* Create(const std::string& name, size_t capacity, uint8_t* requested_begin);
  static BumpPointerSpace* CreateFromMemMap(const std::string& name, MemMap* mem_map);

  // Allocate num_bytes, returns null if the space is full.
  mirror::Object* Alloc(Thread* self, size_t num_bytes, size_t* bytes_allocated,
                        size_t* usable_size,
                        size_t* bytes_tl_bulk_allocated) OVERRIDE;
  // Thread-unsafe allocation for when mutators are suspended, used by the semispace collector.
  mirror::Object* AllocThreadUnsafe(Thread* self, size_t num_bytes, size_t* bytes_allocated,
                                    size_t* usable_size, size_t* bytes_tl_bulk_allocated)
      OVERRIDE REQUIRES(Locks::mutator_lock_);

  mirror::Object* AllocNonvirtual(size_t num_bytes, size_t* bytes_allocated,
                                  size_t* usable_size, size_t* bytes_tl_bulk_allocated);

  // Return the storage space required by obj.
  size_t AllocationSize(mirror::Object* obj, size_t* usable_size) OVERRIDE
      SHARED_REQUIRES(Locks::mutator_lock_) {
    return AllocationSizeNonvirtual(obj, usable_size);
  }

  // Account allocations, used by Parallel copying collector.
  void AccountAllocation(size_t num_objects);

  // NOPS unless we support free lists.
  size_t Free(Thread*, mirror::Object*) OVERRIDE {
    return 0;
  }

  size_t FreeList(Thread*, size_t, mirror::Object**) OVERRIDE {
    return 0;
  }

  size_t AllocationSizeNonvirtual(mirror::Object* obj, size_t* usable_size)
      SHARED_REQUIRES(Locks::mutator_lock_);

  // Removes the fork time growth limit on capacity, allowing the application to allocate up to the
  // maximum reserved size of the heap.
  void ClearGrowthLimit() {
    growth_end_ = Limit();
  }

  // Override capacity so that we only return the possibly limited capacity
  size_t Capacity() const {
    return growth_end_ - begin_;
  }

  // The total amount of memory reserved for the space.
  size_t NonGrowthLimitCapacity() const {
    return GetMemMap()->Size();
  }

  accounting::ContinuousSpaceBitmap* GetLiveBitmap() const OVERRIDE {
    return nullptr;
  }

  accounting::ContinuousSpaceBitmap* GetMarkBitmap() const OVERRIDE {
    return nullptr;
  }

  // Reset the space to empty.
  void Clear();

  void Dump(std::ostream& os) const;

  size_t RevokeThreadLocalBuffers(Thread* thread) SHARED_REQUIRES(Locks::mutator_lock_);
  size_t RevokeAllThreadLocalBuffers() SHARED_REQUIRES(Locks::mutator_lock_)
      REQUIRES(!Locks::runtime_shutdown_lock_, !Locks::thread_list_lock_);

  void AssertTlabOperationSafety(Thread* thread);
  void AssertThreadLocalBuffersAreRevoked(Thread* thread);
  void AssertAllThreadLocalBuffersAreRevoked()
      REQUIRES(!Locks::runtime_shutdown_lock_, !Locks::thread_list_lock_);

  uint64_t GetBytesAllocated() SHARED_REQUIRES(Locks::mutator_lock_)
      REQUIRES(!*Locks::runtime_shutdown_lock_, !*Locks::thread_list_lock_);
  uint64_t GetObjectsAllocated() SHARED_REQUIRES(Locks::mutator_lock_)
      REQUIRES(!*Locks::runtime_shutdown_lock_, !*Locks::thread_list_lock_);

  bool IsEmpty() const {
    return Begin() == End();
  }

  bool CanMoveObjects() const OVERRIDE {
    return true;
  }

  bool Contains(const mirror::Object* obj) const {
    const uint8_t* byte_obj = reinterpret_cast<const uint8_t*>(obj);
    return byte_obj >= Begin() && byte_obj < End();
  }

  // TODO: Change this? Mainly used for compacting to a particular region of memory.
  BumpPointerSpace(const std::string& name, uint8_t* begin, uint8_t* limit);

  // Return the object which comes after obj, while ensuring alignment.
  static mirror::Object* GetNextObject(mirror::Object* obj)
      SHARED_REQUIRES(Locks::mutator_lock_);

  // Allocate a new TLAB, returns false if the allocation failed.
  bool AllocNewTlab(Thread* self, size_t bytes) SHARED_REQUIRES(Locks::mutator_lock_);

  BumpPointerSpace* AsBumpPointerSpace() OVERRIDE {
    return this;
  }

  // Go through all of the blocks and visit the continuous objects.
  void Walk(ObjectCallback* callback, void* arg) SHARED_REQUIRES(Locks::mutator_lock_);

  accounting::ContinuousSpaceBitmap::SweepCallback* GetSweepCallback() OVERRIDE;

  // Record objects / bytes freed.
  void RecordFree(int32_t objects, int32_t bytes) {
    objects_allocated_.FetchAndSubSequentiallyConsistent(objects);
    bytes_allocated_.FetchAndSubSequentiallyConsistent(bytes);
  }

  void LogFragmentationAllocFailure(std::ostream& os, size_t failed_alloc_bytes) OVERRIDE
      SHARED_REQUIRES(Locks::mutator_lock_);

  // Object alignment within the space.
  static constexpr size_t kAlignment = art::kObjectAlignment;
  accounting::AgingTable* GetAgingTable() {
    return aging_table_.get();
  }

  size_t GetMaxContiguousBytes() {
    return Limit() - End();
  }

 protected:
  BumpPointerSpace(const std::string& name, MemMap* mem_map);

  // Allocate a raw block of bytes.
  mirror::Object* AllocNonvirtualWithoutAccounting(size_t num_bytes);

  // Used to fill a memory block when updating the forwarding pointer fails.
  ALWAYS_INLINE void FillWithDummyObject(mirror::Object* dummy_obj, size_t byte_size)
      SHARED_REQUIRES(Locks::mutator_lock_);

  uint8_t* growth_end_;
  AtomicInteger objects_allocated_;  // Accumulated from revoked thread local regions.
  AtomicInteger bytes_allocated_;    // Accumulated from revoked thread local regions.
  AtomicInteger tlabs_alive_;        // Number of currently allocated TLABs

  // Aging table.
  std::unique_ptr<accounting::AgingTable> aging_table_;

 private:
  struct TlabPtrs {
    TlabPtrs(uint8_t* start, uint8_t* end) :
             start_(start), end_(end) {}
    uint8_t* start_;
    uint8_t* end_;
  };

  static bool TlabPtrsSorter(TlabPtrs a, TlabPtrs b) {
    return a.start_ < b.start_;
  }

  // Thread-unsafe walk for when active TLABs are known.
  void WalkThreadUnsafe(std::vector<TlabPtrs>* active_tlabs, ObjectCallback* callback, void* arg)
      SHARED_REQUIRES(Locks::mutator_lock_);
  void CollectActiveTlabsWithSuspendAll(Thread* self, std::vector<TlabPtrs>* active_tlabs);
  // Thread-unsafe collecting for when mutators are suspended.
  void CollectActiveTlabsUnsafe(std::vector<TlabPtrs>* active_tlabs)
      SHARED_REQUIRES(Locks::mutator_lock_)
      REQUIRES(Locks::runtime_shutdown_lock_, Locks::thread_list_lock_);

  friend class collector::MarkSweep;
  DISALLOW_COPY_AND_ASSIGN(BumpPointerSpace);
};

}  // namespace space
}  // namespace gc
}  // namespace art

#endif  // ART_RUNTIME_GC_SPACE_BUMP_POINTER_SPACE_H_
