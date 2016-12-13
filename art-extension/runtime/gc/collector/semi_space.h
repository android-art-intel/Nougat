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

#ifndef ART_RUNTIME_GC_COLLECTOR_SEMI_SPACE_H_
#define ART_RUNTIME_GC_COLLECTOR_SEMI_SPACE_H_

#include <memory>

#include "atomic.h"
#include "base/macros.h"
#include "base/mutex.h"
#include "garbage_collector.h"
#include "gc_root.h"
#include "gc/accounting/aging_table.h"
#include "gc/accounting/heap_bitmap.h"
#include "immune_spaces.h"
#include "mirror/object_reference.h"
#include "object_callbacks.h"
#include "offsets.h"
#include "stack.h"

namespace art {

class Thread;

namespace mirror {
  class Class;
  class Object;
}  // namespace mirror

namespace gc {

class Heap;

namespace accounting {
  template <typename T> class AtomicStack;
  typedef AtomicStack<mirror::Object> ObjectStack;
}  // namespace accounting

namespace space {
  class ContinuousMemMapAllocSpace;
  class ContinuousSpace;
}  // namespace space

namespace collector {

class SemiSpace : public GarbageCollector {
 public:
  // If true, use remembered sets in the generational mode.
  static constexpr bool kUseRememberedSet = true;
  explicit SemiSpace(Heap* heap, bool generational = false,
                     const std::string& name_prefix = "", bool need_aging_table = false, bool support_parallel = true);

  ~SemiSpace() {}

  virtual void RunPhases() OVERRIDE NO_THREAD_SAFETY_ANALYSIS;
  virtual void InitializePhase();
  virtual void MarkingPhase() REQUIRES(Locks::mutator_lock_)
      REQUIRES(!Locks::heap_bitmap_lock_);
  virtual void ReclaimPhase() REQUIRES(Locks::mutator_lock_)
      REQUIRES(!Locks::heap_bitmap_lock_);
  virtual void FinishPhase() REQUIRES(Locks::mutator_lock_);
  void MarkReachableObjects()
      REQUIRES(Locks::mutator_lock_, Locks::heap_bitmap_lock_);
  virtual GcType GetGcType() const OVERRIDE {
    return need_aging_table_ ? kGcTypeYoung : kGcTypePartial;
  }
  virtual CollectorType GetCollectorType() const OVERRIDE {
    return generational_ ? kCollectorTypeGSS : kCollectorTypeSS;
  }

  // Wake up suspended mutators due to allocation failures.
  void NeedToWakeMutators();
  // Sets which space we will be copying objects to.
  void SetToSpace(space::ContinuousMemMapAllocSpace* to_space);

  // Set the space where we copy objects from.
  void SetFromSpace(space::ContinuousMemMapAllocSpace* from_space);

  // Get from space.
  space::ContinuousMemMapAllocSpace* GetFromSpace();
  // Get to space.
  space::ContinuousMemMapAllocSpace* GetToSpace();
  // Get promote space.
  space::ContinuousMemMapAllocSpace* GetPromoSpace();
  // Used by parallel copying task to determine the promoted space.
  bool IsCollectFromSpaceOnly();
  // Set whether or not we swap the semi spaces in the heap. This needs to be done with mutators
  // suspended.
  void SetSwapSemiSpaces(bool swap_semi_spaces) {
    swap_semi_spaces_ = swap_semi_spaces;
  }

  // Initializes internal structures.
  void Init();

  // Find the default mark bitmap.
  void FindDefaultMarkBitmap();

  // Updates obj_ptr if the object has moved.
  template<bool kPoisonReferences>
  void MarkObject(mirror::ObjectReference<kPoisonReferences, mirror::Object>* obj_ptr)
      REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);

  template<bool kPoisonReferences>
  void MarkObjectIfNotInToSpace(mirror::ObjectReference<kPoisonReferences, mirror::Object>* obj_ptr)
      REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);

  virtual mirror::Object* MarkObject(mirror::Object* root) OVERRIDE
      REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);

  virtual void MarkHeapReference(mirror::HeapReference<mirror::Object>* obj_ptr) OVERRIDE
      REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);

  void ScanObject(mirror::Object* obj)
      REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);

  void VerifyNoFromSpaceReferences(mirror::Object* obj)
      SHARED_REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);

  void MarkThreadRoots()
      REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);
  void MarkNonThreadRoots()
      REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);
   void MarkConcurrentRoots()
      REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);
 // Marks the root set at the start of a garbage collection.
  void MarkRoots()
      REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);

  // Bind the live bits to the mark bits of bitmaps for spaces that are never collected, ie
  // the image. Mark that portion of the heap as immune.
  virtual void BindBitmaps() SHARED_REQUIRES(Locks::mutator_lock_)
      REQUIRES(!Locks::heap_bitmap_lock_);

  void UnBindBitmaps()
      REQUIRES(Locks::heap_bitmap_lock_);

  void ProcessReferences(Thread* self) REQUIRES(Locks::mutator_lock_)
      REQUIRES(Locks::mutator_lock_);

  // Sweeps unmarked objects to complete the garbage collection.
  virtual void Sweep(bool swap_bitmaps)
      REQUIRES(Locks::heap_bitmap_lock_)
      SHARED_REQUIRES(Locks::mutator_lock_);

  // Sweeps unmarked objects to complete the garbage collection.
  void SweepLargeObjects(bool swap_bitmaps) REQUIRES(Locks::heap_bitmap_lock_);

  void SweepSystemWeaks()
      SHARED_REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);

  virtual void VisitRoots(mirror::Object*** roots, size_t count, const RootInfo& info) OVERRIDE
      REQUIRES(Locks::mutator_lock_, Locks::heap_bitmap_lock_);

  virtual void VisitRoots(mirror::CompressedReference<mirror::Object>** roots, size_t count,
                          const RootInfo& info) OVERRIDE
      REQUIRES(Locks::mutator_lock_, Locks::heap_bitmap_lock_);

  virtual mirror::Object* MarkNonForwardedObject(mirror::Object* obj)
      REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);

  // Schedules an unmarked object for reference processing.
  void DelayReferenceReferent(mirror::Class* klass, mirror::Reference* reference)
      SHARED_REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);

  // Inner class for parallel copying.
  class ThreadRootMarkStack {
   public:
    ThreadRootMarkStack(size_t capacity);
    inline size_t Capacity() const { return capacity_; }
    // Get the mark stack size.
    inline size_t Size() const { return size_; }
    void Resize();
    void PushBack(mirror::Object* obj)
      SHARED_REQUIRES(Locks::mutator_lock_);
    StackReference<mirror::Object>* GetMarkStack() const;
    ~ThreadRootMarkStack();

   private:
    size_t capacity_;
    // Incremental used for enlarge capacity.
    size_t incremental_;
    size_t size_;
    StackReference<mirror::Object>* mark_stack_;
    inline void CreateMarkStack(size_t capacity);
    inline void RegenerateMarkStack();
    inline void DeleteMarkStack();
  };

  typedef std::map<Thread*, ThreadRootMarkStack*> ThreadRootStacksMap;

 protected:
  // Returns null if the object is not marked, otherwise returns the forwarding address (same as
  // object for non movable things).
  virtual mirror::Object* IsMarked(mirror::Object* object) OVERRIDE
      REQUIRES(Locks::mutator_lock_)
      SHARED_REQUIRES(Locks::heap_bitmap_lock_);

  virtual bool IsMarkedHeapReference(mirror::HeapReference<mirror::Object>* object) OVERRIDE
      REQUIRES(Locks::mutator_lock_)
      SHARED_REQUIRES(Locks::heap_bitmap_lock_);

  // Marks or unmarks a large object based on whether or not set is true. If set is true, then we
  // mark, otherwise we unmark.
  bool MarkLargeObject(const mirror::Object* obj)
      REQUIRES(Locks::heap_bitmap_lock_)
      SHARED_REQUIRES(Locks::mutator_lock_);

  // Expand mark stack to 2x its current size.
  void ResizeMarkStack(size_t new_size) SHARED_REQUIRES(Locks::mutator_lock_);

  // Returns true if we should sweep the space.
  virtual bool ShouldSweepSpace(space::ContinuousSpace* space) const;

  // Push an object onto the mark stack.
  void MarkStackPush(mirror::Object* obj) SHARED_REQUIRES(Locks::mutator_lock_);

  void UpdateAndMarkModUnion()
      REQUIRES(Locks::heap_bitmap_lock_)
      SHARED_REQUIRES(Locks::mutator_lock_);

  // Recursively blackens objects on the mark stack.
  void ProcessMarkStack()
      REQUIRES(Locks::mutator_lock_, Locks::heap_bitmap_lock_);

  inline mirror::Object* GetForwardingAddressInFromSpace(mirror::Object* obj) const
      SHARED_REQUIRES(Locks::mutator_lock_);

  inline bool IsMarkedParallel(mirror::Object* obj)
      SHARED_REQUIRES(Locks::mutator_lock_, Locks::heap_bitmap_lock_);
  // Revoke all the thread-local buffers.
  void RevokeAllThreadLocalBuffers() SHARED_REQUIRES(Locks::mutator_lock_);
  // Get thread count for parallel copy.
  size_t GetThreadCount() const;

  // Used to fill a memory block when updating the forwarding pointer fails.
  ALWAYS_INLINE void FillWithDummyObject(mirror::Object* dummy_obj, size_t byte_size)
      SHARED_REQUIRES(Locks::mutator_lock_);

  virtual mirror::Object* MarkNonForwardedObjectParallel(mirror::Object* obj,
                                                         bool* win,
                                                         void* task)
      SHARED_REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);

  mirror::Object* TryInstallForwardingAddress(mirror::Object* obj,
                                              space::ContinuousMemMapAllocSpace* dest_space,
                                              size_t* bytes_allocated,
                                              bool* win) ALWAYS_INLINE
      SHARED_REQUIRES(Locks::mutator_lock_);

  template<bool kPoisonReference>
  void MarkParallel(mirror::ObjectReference<kPoisonReference, mirror::Object>* obj_ptr,
                    void* task) ALWAYS_INLINE
      SHARED_REQUIRES(Locks::mutator_lock_)
      REQUIRES(Locks::heap_bitmap_lock_);

  template<bool kPoisonReferences>
  void MarkObjectIfNotInToSpaceParallel(mirror::ObjectReference<kPoisonReferences, mirror::Object>* obj_ptr,
                                        void* task)
      REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);

  // Process the mark stack parallel.
  void ProcessMarkStackParallel(size_t thread_count)
      REQUIRES(Locks::mutator_lock_, Locks::heap_bitmap_lock_);

  template<typename MarkVisitor, typename ReferenceVisitor>
  ALWAYS_INLINE void ScanObjectVisit(mirror::Object* obj,
                                     const MarkVisitor& visitor,
                                     const ReferenceVisitor& ref_visitor)
      SHARED_REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_);

  // Current space, we check this space first to avoid searching for the appropriate space for an
  // object.
  accounting::ObjectStack* mark_stack_;

  // Every object inside the immune spaces is assumed to be marked.
  ImmuneSpaces immune_spaces_;

  // If true, the large object space is immune.
  bool is_large_object_space_immune_;

  // Destination and source spaces (can be any type of ContinuousMemMapAllocSpace which either has
  // a live bitmap or doesn't).
  space::ContinuousMemMapAllocSpace* to_space_;
  // Cached live bitmap as an optimization.
  accounting::ContinuousSpaceBitmap* to_space_live_bitmap_;
  space::ContinuousMemMapAllocSpace* from_space_;
  // Cached mark bitmap as an optimization.
  accounting::HeapBitmap* mark_bitmap_;

  Thread* self_;

  // When true, the generational mode (promotion and the bump pointer
  // space only collection) is enabled. TODO: move these to a new file
  // as a new garbage collector?
  const bool generational_;

  // Used for the generational mode. During a collection, keeps track
  // of how many bytes of objects have been copied so far from the
  // bump pointer space to the non-moving space.
  uint64_t bytes_promoted_;
  uint64_t bytes_wasted_;

  // Used for the generational mode. Keeps track of how many bytes of
  // objects have been copied so far from the bump pointer space to
  // the non-moving space, since the last whole heap collection.
  uint64_t bytes_promoted_since_last_whole_heap_collection_;

  // Used for the generational mode. Keeps track of how many bytes of
  // large objects were allocated at the last whole heap collection.
  uint64_t large_object_bytes_allocated_at_last_whole_heap_collection_;

  // Used for generational mode. When true, we only collect the from_space_.
  bool collect_from_space_only_;
  bool force_copy_all_ = false;

  // The space which we are promoting into, only used for GSS.
  space::ContinuousMemMapAllocSpace* promo_dest_space_;

  // The space which we copy to if the to_space_ is full.
  space::ContinuousMemMapAllocSpace* fallback_space_;

  // How many objects and bytes we moved, used so that we don't need to Get the size of the
  // to_space_ when calculating how many objects and bytes we freed.
  size_t bytes_moved_;
  size_t objects_moved_;

  // How many bytes we avoided dirtying.
  size_t saved_bytes_;

  // The name of the collector.
  std::string collector_name_;

  // Used for the generational mode. The default interval of the whole
  // heap collection. If N, the whole heap collection occurs every N
  // collections.
  static constexpr int kDefaultWholeHeapCollectionInterval = 5;

  // Whether or not we swap the semi spaces in the heap during the marking phase.
  bool swap_semi_spaces_;
  // Whether or not record root using seperate mark stack for parallel copying.
  bool marking_roots_;

  // Whether or not we create the aging table to track the ages of living objects.
  bool need_aging_table_;

  AtomicInteger work_chunks_created_;
  AtomicInteger work_chunks_deleted_;
  Atomic<size_t> bytes_promoted_parallel_;
  Atomic<size_t> bytes_wasted_promoted_parallel_;
  Atomic<size_t> objects_promoted_parallel_;
  Atomic<size_t> objects_moved_parallel_;
  Atomic<size_t> bytes_moved_parallel_;
  Atomic<size_t> dummy_bytes_;
  Atomic<size_t> dummy_objects_;
  Atomic<size_t> fallback_bytes_parallel_;
  Atomic<size_t> fallback_objects_parallel_;
  Atomic<size_t> wasted_bytes_;
  accounting::AgingTable* from_age_table_;
  accounting::AgingTable* to_age_table_;
  size_t threshold_age_;

  // Map stores the pair of thread and stack of the thread's roots.
  ThreadRootStacksMap* thread_roots_stacks_;
  ThreadRootMarkStack* thread_mark_stack_;
  // Support parallel copy or not.
  // Used for ZygoteCompact because we don't want gaps in zygote space.
  bool support_parallel_;
  bool support_parallel_default_;
 private:
  friend class BitmapSetSlowPathVisitor;
  friend class SSBitmapSetSlowPathVisitor;
  friend class MarkStackCopyTask;
  DISALLOW_IMPLICIT_CONSTRUCTORS(SemiSpace);
};

}  // namespace collector
}  // namespace gc
}  // namespace art

#endif  // ART_RUNTIME_GC_COLLECTOR_SEMI_SPACE_H_
