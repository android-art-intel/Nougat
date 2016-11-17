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

#include "mark_sweep.h"

#include <atomic>
#include <functional>
#include <numeric>
#include <climits>
#include <vector>

#include "base/bit_vector-inl.h"
#include "base/bounded_fifo.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/mutex-inl.h"
#include "base/systrace.h"
#include "base/time_utils.h"
#include "base/timing_logger.h"
#include "gc/accounting/aging_table.h"
#include "gc/accounting/card_table-inl.h"
#include "gc/accounting/heap_bitmap-inl.h"
#include "gc/accounting/mod_union_table.h"
#include "gc/accounting/remembered_set.h"
#include "gc/accounting/space_bitmap-inl.h"
#include "gc/heap.h"
#include "gc/reference_processor.h"
#include "gc/space/large_object_space.h"
#include "gc/space/space-inl.h"
#include "mark_sweep-inl.h"
#include "mirror/object-inl.h"
#include "runtime.h"
#include "scoped_thread_state_change.h"
#include "thread-inl.h"
#include "thread_list.h"

namespace art {
namespace gc {
namespace collector {

// Performance options.
static constexpr bool kUseRecursiveMark = false;
static constexpr bool kUseMarkStackPrefetch = true;
static constexpr size_t kSweepArrayChunkFreeSize = 1024;
static constexpr bool kPreCleanCards = true;

// Parallelism options.
static constexpr bool kParallelCardScan = true;
static constexpr bool kParallelRecursiveMark = true;
// Don't attempt to parallelize mark stack processing unless the mark stack is at least n
// elements. This is temporary until we reduce the overhead caused by allocating tasks, etc.. Not
// having this can add overhead in ProcessReferences since we may end up doing many calls of
// ProcessMarkStack with very small mark stacks.
static constexpr size_t kMinimumParallelMarkStackSize = 128;
static constexpr bool kParallelProcessMarkStack = true;

// Profiling and information flags.
static constexpr bool kProfileLargeObjects = false;
static constexpr bool kMeasureOverhead = false;
static constexpr bool kCountTasks = false;
static constexpr bool kCountMarkedObjects = false;

// Turn off kCheckLocks when profiling the GC since it slows the GC down by up to 40%.
static constexpr bool kCheckLocks = kDebugLocking;
static constexpr bool kVerifyRootsMarked = kIsDebugBuild;

// If true, revoke the rosalloc thread-local buffers at the
// checkpoint, as opposed to during the pause.
static constexpr bool kRevokeRosAllocThreadLocalBuffersAtCheckpoint = true;
static constexpr uint32_t kMinRemainBytes = 16 * KB;

void MarkSweep::BindBitmaps() {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  WriterMutexLock mu(Thread::Current(), *Locks::heap_bitmap_lock_);
  // Mark all of the spaces we never collect as immune.
  for (const auto& space : GetHeap()->GetContinuousSpaces()) {
    if (space->GetGcRetentionPolicy() == space::kGcRetentionPolicyNeverCollect) {
      immune_spaces_.AddSpace(space);
    }
  }
}

MarkSweep::MarkSweep(Heap* heap, bool is_concurrent, bool is_copying, const std::string& name_prefix)
    : GarbageCollector(heap,
                       name_prefix +
                       (is_concurrent ? "concurrent mark sweep": "mark sweep")),
      current_space_bitmap_(nullptr),
      mark_bitmap_(nullptr),
      mark_stack_(nullptr),
      gc_barrier_(new Barrier(0)),
      mark_stack_lock_("mark sweep mark stack lock", kMarkSweepMarkStackLock),
      is_concurrent_(is_concurrent),
      is_copying_(is_copying),
      live_stack_freeze_size_(0),
      updating_reference_(false),
      enable_parallel_(true) {
  std::string error_msg;
  MemMap* mem_map = MemMap::MapAnonymous(
      "mark sweep sweep array free buffer", nullptr,
      RoundUp(kSweepArrayChunkFreeSize * sizeof(mirror::Object*), kPageSize),
      PROT_READ | PROT_WRITE, false, false, &error_msg);
  CHECK(mem_map != nullptr) << "Couldn't allocate sweep array free buffer: " << error_msg;
  sweep_array_free_buffer_mem_map_.reset(mem_map);
}

void MarkSweep::InitializePhase() {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  mark_stack_ = heap_->GetMarkStack();
  DCHECK(mark_stack_ != nullptr);
  immune_spaces_.Reset();
  no_reference_class_count_.StoreRelaxed(0);
  normal_count_.StoreRelaxed(0);
  class_count_.StoreRelaxed(0);
  object_array_count_.StoreRelaxed(0);
  other_count_.StoreRelaxed(0);
  reference_count_.StoreRelaxed(0);
  large_object_test_.StoreRelaxed(0);
  large_object_mark_.StoreRelaxed(0);
  overhead_time_ .StoreRelaxed(0);
  work_chunks_created_.StoreRelaxed(0);
  work_chunks_deleted_.StoreRelaxed(0);
  mark_null_count_.StoreRelaxed(0);
  mark_immune_count_.StoreRelaxed(0);
  mark_fastpath_count_.StoreRelaxed(0);
  mark_slowpath_count_.StoreRelaxed(0);
  {
    // TODO: I don't think we should need heap bitmap lock to Get the mark bitmap.
    ReaderMutexLock mu(Thread::Current(), *Locks::heap_bitmap_lock_);
    mark_bitmap_ = heap_->GetMarkBitmap();
  }
  /*
   For copying living objects from from bps to to bps, there are below two solutions:
   * a) requires somewhere to backup the lock words of the living objects in from bps.
   *    1) calculate forwarding address of living objects in from bps, store the forwarding
   *       address into the lock word with the lock word backup somewhere.
   *    2) walk the living objects in from bps to fixup the references.
   *    3) copy the living objects from from bps to to bps.
   *
   * b) requires live bitmap of to bps to record the living objects in to bps after promotion.
   *    1) calculate the forwarding address and copy the living objects from from bps to to bps
   *       at the same time. The forwarding address are stored in the lock word of the old objects
   *       in from bps.
   *    2) walk the living objects in to bps to fixup the references.
   *
   * solution a) needs 3X to walk the living objects in bps.
   * solution b) needs 2X to walk the living objects in bps.
   * the lock word backup footprint is higher than living bitmap of to bps.
   *
   * Generational Copying is using solution b).
  */
  if (IsCopying()) {
    from_bps_ = heap_->GetFromBumpPointerSpace();
    to_bps_ = heap_->GetToBumpPointerSpace();
    promo_dest_space_ = GetHeap()->GetPrimaryFreeListSpace();
    force_copy_all_ = false;
    threshold_age_ = heap_->GetThresholdAge();
    // Bitmap which describes which objects we have to move in bump pointer space.
    // No need to create mark and live bitmaps for bump pointer space from the scratch.
    // One bump pointer space only requires one bitmap to track the living objects.
    objects_before_forwarding_.reset(accounting::ContinuousSpaceBitmap::Create(
        "objects before forwarding", from_bps_->Begin(), from_bps_->Capacity()));
    objects_after_forwarding_.reset(accounting::ContinuousSpaceBitmap::Create(
        "objects after forwarding", to_bps_->Begin(), to_bps_->Capacity()));
    // Used for parallel copy.
    // TODO: 4MB maybe too large, use Resize() to reduce the memory overhead.
    // TODO: Resize() requires atomic operations for parallel GC.
    copy_candidate_stack_.reset(accounting::ObjectStack::Create("candidate copy stack",
                                8 * MB, 8 * MB));
    from_age_table_ = from_bps_->GetAgingTable();
    to_age_table_ = to_bps_->GetAgingTable();
    if (GetCurrentIteration()->GetClearSoftReferences()) {
      // Force copy all since it is close to throw OOM.
      force_copy_all_ = true;
      VLOG(heap) << "Copying all objects in young space";
    }
    DCHECK(from_age_table_ != nullptr);
    DCHECK(to_age_table_ != nullptr);
    saved_bytes_ = 0;
    bytes_moved_ = 0;
    bytes_adjusted_ = 0;
    objects_moved_ = 0;
    bytes_promoted_ = 0;
    self_ = Thread::Current();
  }
  if (!GetCurrentIteration()->GetClearSoftReferences()) {
    // Always clear soft references if a non-sticky collection.
    GetCurrentIteration()->SetClearSoftReferences(GetGcType() != collector::kGcTypeSticky);
  }
}

void MarkSweep::NeedToWakeMutators() {
  if (GetCollectorType() == kCollectorTypeGenCopying) {
    GetHeap()->WakeMutators();
  }
}

void MarkSweep::RunPhases() {
  Thread* self = Thread::Current();
  InitializePhase();
  Locks::mutator_lock_->AssertNotHeld(self);
  if (IsConcurrent()) {
    GetHeap()->PreGcVerification(this);
    {
      ReaderMutexLock mu(self, *Locks::mutator_lock_);
      if (Runtime::Current()->EnabledGcProfile()) {
        uint64_t mark_start = NanoTime();
        MarkingPhase();
        RegisterMark(NanoTime() - mark_start);
      } else {
        MarkingPhase();
      }
    }
    ScopedPause pause(this);
    GetHeap()->PrePauseRosAllocVerification(this);
    PausePhase();
    RevokeAllThreadLocalBuffers();
  } else {
    ScopedPause pause(this);
    GetHeap()->PreGcVerificationPaused(this);
    if (Runtime::Current()->EnabledGcProfile()) {
      uint64_t mark_start = NanoTime();
      MarkingPhase();
      RegisterMark(NanoTime() - mark_start);
    } else {
      MarkingPhase();
    }
    GetHeap()->PrePauseRosAllocVerification(this);
    PausePhase();
    RevokeAllThreadLocalBuffers();
  }
  {
    NeedToWakeMutators();
    // Sweeping always done concurrently, even for non concurrent mark sweep.
    ReaderMutexLock mu(self, *Locks::mutator_lock_);
    if (Runtime::Current()->EnabledGcProfile()) {
      uint64_t sweep_start = NanoTime();
      ReclaimPhase();
      RegisterSweep(NanoTime() - sweep_start);
    } else {
      ReclaimPhase();
    }
  }
  GetHeap()->PostGcVerification(this);
  FinishPhase();
}

void MarkSweep::ProcessReferences(Thread* self) {
  WriterMutexLock mu(self, *Locks::heap_bitmap_lock_);
  GetHeap()->GetReferenceProcessor()->ProcessReferences(
      true,
      GetTimings(),
      GetCurrentIteration()->GetClearSoftReferences(),
      this);
}

static inline size_t CopyAvoidingDirtyingPages(void* dest, const void* src, size_t size) {
  if (LIKELY(size <= static_cast<size_t>(kPageSize))) {
    // We will dirty the current page and somewhere in the middle of the next page. This means
    // that the next object copied will also dirty that page.
    // TODO: Worth considering the last object copied? We may end up dirtying one page which is
    // not necessary per GC.
    memcpy(dest, src, size);
    return 0;
  }
  size_t saved_bytes = 0;
  uint8_t* byte_dest = reinterpret_cast<uint8_t*>(dest);
  if (kIsDebugBuild) {
    for (size_t i = 0; i < size; ++i) {
      CHECK_EQ(byte_dest[i], 0U);
    }
  }
  // Process the start of the page. The page must already be dirty, don't bother with checking.
  const uint8_t* byte_src = reinterpret_cast<const uint8_t*>(src);
  const uint8_t* limit = byte_src + size;
  size_t page_remain = AlignUp(byte_dest, kPageSize) - byte_dest;
  // Copy the bytes until the start of the next page.
  memcpy(dest, src, page_remain);
  byte_src += page_remain;
  byte_dest += page_remain;
  DCHECK_ALIGNED(reinterpret_cast<uintptr_t>(byte_dest), kPageSize);
  DCHECK_ALIGNED(reinterpret_cast<uintptr_t>(byte_dest), sizeof(uintptr_t));
  DCHECK_ALIGNED(reinterpret_cast<uintptr_t>(byte_src), sizeof(uintptr_t));
  while (byte_src + kPageSize < limit) {
    bool all_zero = true;
    uintptr_t* word_dest = reinterpret_cast<uintptr_t*>(byte_dest);
    const uintptr_t* word_src = reinterpret_cast<const uintptr_t*>(byte_src);
    for (size_t i = 0; i < kPageSize / sizeof(*word_src); ++i) {
      // Assumes the destination of the copy is all zeros.
      if (word_src[i] != 0) {
        all_zero = false;
        word_dest[i] = word_src[i];
      }
    }
    if (all_zero) {
      // Avoided copying into the page since it was all zeros.
      saved_bytes += kPageSize;
    }
    byte_src += kPageSize;
    byte_dest += kPageSize;
  }
  // Handle the part of the page at the end.
  memcpy(byte_dest, byte_src, limit - byte_src);
  return saved_bytes;
}

void MarkSweep::ForwardObject(mirror::Object* obj) {
  const size_t object_size = obj->SizeOf();
  size_t bytes_allocated = 0;
  size_t dummy = 0;
  mirror::Object* forward_address = nullptr;
  uint8_t age = from_age_table_->GetObjectAge(obj);
  if (force_copy_all_ || age >= threshold_age_) {
    forward_address = promo_dest_space_->AllocThreadUnsafe(self_, object_size, &bytes_allocated,
                                               nullptr, &dummy);
    if (UNLIKELY(forward_address == nullptr)) {
      // If out of space, fall back to the to-space.
      forward_address = to_bps_->AllocThreadUnsafe(self_, object_size, &bytes_allocated,
                                                                  nullptr, &dummy);
      objects_after_forwarding_->Set(forward_address);
      to_age_table_->IncreaseObjectAge(forward_address, age);
    } else {
      bytes_promoted_ += bytes_allocated;
      bytes_adjusted_ += bytes_allocated - object_size;
      GetHeap()->WriteBarrierEveryFieldOf(forward_address);
      // Handle the bitmaps marking.
      accounting::ContinuousSpaceBitmap* live_bitmap = promo_dest_space_->GetLiveBitmap();
      DCHECK(live_bitmap != nullptr);
      accounting::ContinuousSpaceBitmap* mark_bitmap = promo_dest_space_->GetMarkBitmap();
      DCHECK(mark_bitmap != nullptr);
      DCHECK(!live_bitmap->Test(forward_address));
      // Mark forward_address on the live bit map.
      live_bitmap->Set(forward_address);
      // Mark forward_address on the mark bit map.
      DCHECK(!mark_bitmap->Test(forward_address));
      mark_bitmap->Set(forward_address);
    }
  } else {
    forward_address = to_bps_->AllocThreadUnsafe(self_, object_size, &bytes_allocated,
                                                                nullptr, &dummy);
    objects_after_forwarding_->Set(forward_address);
    to_age_table_->IncreaseObjectAge(forward_address, age);
  }
  CHECK((forward_address != nullptr));
  CHECK(to_bps_->HasAddress(forward_address) ||
        promo_dest_space_->HasAddress(forward_address));
  ++objects_moved_;
  // The objects promoted to ros will be rounded as backet size.
  bytes_moved_ += bytes_allocated;
  // Copy over the object and add it to the mark stack since we still need to update its
  // references.
  saved_bytes_ +=
      CopyAvoidingDirtyingPages(reinterpret_cast<void*>(forward_address), obj, object_size);
  // Make sure to only update the forwarding address AFTER you copy the object so that the
  // monitor word doesn't Get stomped over.
  obj->SetLockWord(
      LockWord::FromForwardingAddress(reinterpret_cast<size_t>(forward_address)), false);
}

class ForwardObjectsVisitor {
 public:
  explicit ForwardObjectsVisitor(MarkSweep* collector)
      : collector_(collector) {}
  void operator()(mirror::Object* obj) const REQUIRES(Locks::mutator_lock_,
                                                                      Locks::heap_bitmap_lock_) {
    DCHECK_ALIGNED(obj, space::BumpPointerSpace::kAlignment);
    DCHECK(collector_->IsMarked(obj));
    collector_->ForwardObject(obj);
  }

 private:
  MarkSweep* const collector_;
};

inline uint8_t* MarkSweep::ForwardObjectParallelToBuffer(uint8_t* buffer, mirror::Object* obj) {
  const size_t object_size = obj->SizeOf();
  size_t bytes_allocated = RoundUp(object_size, space::BumpPointerSpace::kAlignment);

  mirror::Object* forward_address = reinterpret_cast<mirror::Object*>(buffer);
  buffer += bytes_allocated;

  uint8_t age = from_age_table_->GetObjectAge(obj);
  objects_after_forwarding_->AtomicTestAndSet(forward_address);
  to_age_table_->IncreaseObjectAge(forward_address, age);

  ++objects_moved_;
  bytes_moved_ += bytes_allocated;
  // Copy over the object and add it to the mark stack since we still need to update its
  // references.
  saved_bytes_ +=
      CopyAvoidingDirtyingPages(reinterpret_cast<void*>(forward_address), obj, object_size);
  obj->SetLockWord(
      LockWord::FromForwardingAddress(reinterpret_cast<size_t>(forward_address)), false);

  CHECK(to_bps_->HasAddress(forward_address));
  return buffer;
}

inline bool MarkSweep::ForwardObjectParallelPromo(Thread* self, mirror::Object* obj, size_t& req_space_size) {
  const size_t object_size = obj->SizeOf();
  uint8_t age = from_age_table_->GetObjectAge(obj);
  if (UNLIKELY(force_copy_all_ || age >= threshold_age_)) {
    size_t bytes_allocated = 0;
    DCHECK(GetMarkedForwardAddress(obj) == nullptr);
    mirror::Object* forward_address = nullptr;

    // Allocate.
    size_t bytes_tl_bulk_allocated = 0;
    if (promo_dest_space_->IsRosAllocSpace() && kUsePlab) {
      space::RosAllocSpace* ros_space = promo_dest_space_->AsRosAllocSpace();
      size_t byte_count = RoundUp(object_size, space::BumpPointerSpace::kAlignment);
      forward_address = ros_space->AllocThreadLocal(self, byte_count, &bytes_allocated);
      if (forward_address == nullptr) {
        forward_address = ros_space->Alloc(self, object_size, &bytes_allocated,
                                           nullptr, &bytes_tl_bulk_allocated);
      }
    } else {
      forward_address = promo_dest_space_->Alloc(self, object_size, &bytes_allocated,
                                                 nullptr, &bytes_tl_bulk_allocated);
    }

    if (LIKELY(forward_address != nullptr)) {
      if (bytes_tl_bulk_allocated > 0u) {
        GetHeap()->AddBytesAllocated(bytes_tl_bulk_allocated);
      }
      bytes_promoted_ += bytes_allocated;
      bytes_adjusted_ += bytes_allocated - object_size;
      GetHeap()->WriteBarrierEveryFieldOf(forward_address);
      // Handle the bitmaps marking.
      accounting::ContinuousSpaceBitmap* live_bitmap = promo_dest_space_->GetLiveBitmap();
      DCHECK(live_bitmap != nullptr);
      accounting::ContinuousSpaceBitmap* mark_bitmap = promo_dest_space_->GetMarkBitmap();
      DCHECK(mark_bitmap != nullptr);
      // Mark forward_address on the live bit map.
      live_bitmap->AtomicTestAndSet(forward_address);
      // Mark forward_address on the mark bit map.
      mark_bitmap->AtomicTestAndSet(forward_address);

      ++objects_moved_;
      // The objects promoted to ros will be rounded as backet size.
      bytes_moved_ += bytes_allocated;
      // Copy over the object and add it to the mark stack since we still need to update its
      // references.
      saved_bytes_ +=
          CopyAvoidingDirtyingPages(reinterpret_cast<void*>(forward_address), obj, object_size);
      obj->SetLockWord(
          LockWord::FromForwardingAddress(reinterpret_cast<size_t>(forward_address)), false);

      CHECK(promo_dest_space_->HasAddress(forward_address));
      return true;
    }
  }
  // should be copied to "to" space.
  req_space_size += RoundUp(object_size, space::BumpPointerSpace::kAlignment);
  return false;
}

inline uint8_t* MarkSweep::ForwardObjectParallelAllocBuffer(Thread* self,
                                                            size_t buffer_size,
                                                            size_t count) {
  DCHECK_ALIGNED(buffer_size, space::BumpPointerSpace::kAlignment);
  size_t dummy = 0;
  size_t bytes_allocated = 0;
  uint8_t* buffer =
      reinterpret_cast<uint8_t*>(to_bps_->Alloc(self, buffer_size, &bytes_allocated, nullptr, &dummy));
  DCHECK_EQ(bytes_allocated, buffer_size);
  CHECK(buffer != nullptr) << "Failed to allocate buffer in 'to' space of size " << buffer_size;
  // The first object was counted when we allocated the buffer.
  to_bps_->AccountAllocation(count - 1);
  return buffer;
}

class ParallelForwardTask : public Task {
 public:
  ParallelForwardTask(MarkSweep* mark_sweep,
                      StackReference<mirror::Object>* stack,
                      size_t begin,
                      size_t end)
      : mark_sweep_(mark_sweep),
        stack_(stack),
        begin_(begin),
        end_(end) {
    }

 protected:
  MarkSweep* const mark_sweep_;
  StackReference<mirror::Object>* stack_;
  const size_t begin_;
  const size_t end_;

  virtual void Finalize() {
    delete this;
  }

  // Scans all of the objects
  virtual void Run(Thread* self) NO_THREAD_SAFETY_ANALYSIS {
    DCHECK_EQ(self, Thread::Current());
    // Two passes:
    // 1) Copy to "promote" space and compute the required "to" space.
    // 2) Copy to "to" space.
    DCHECK_LE(end_ - begin_, kMaxGcParalelTaskSize);
    static_assert(kMaxGcParalelTaskSize % 32 == 0u, "kMaxGcParalelTaskSize must be 32 bit aligned");
    uint32_t storage[kMaxGcParalelTaskSize / 32];
    BitVector handled(false, nullptr, kMaxGcParalelTaskSize / 32, storage);
    handled.ClearAllBits();
    size_t req_space_size = 0;
    size_t count = 0;
    for (size_t i = begin_; i < end_; i++) {
      mirror::Object* obj = stack_[i].AsMirrorPtr();
      if (mark_sweep_->ForwardObjectParallelPromo(self, obj, req_space_size)) {
        handled.SetBit(i - begin_);
      } else {
        count++;
      }
    }
    if (LIKELY(count > 0)) {
      // Now allocate the buffer in "to" space.
      uint8_t* buffer = mark_sweep_->ForwardObjectParallelAllocBuffer(self, req_space_size, count);
      const uint8_t* start_buffer = buffer;
      // Second pass, copy to "to".
      for (size_t i = begin_; i < end_; i++) {
        mirror::Object* obj = stack_[i].AsMirrorPtr();
        if (!handled.IsBitSet(i - begin_)) {
          buffer = mark_sweep_->ForwardObjectParallelToBuffer(buffer, obj);
        }
      }
      // Ensure there is no gap.
      CHECK_EQ(start_buffer + req_space_size, buffer);
    }
  }
};

void MarkSweep::ForwardObjects() {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  size_t thread_count = (heap_->GetThreadPool() == nullptr) ? 1 :
                        (heap_->GetParallelGCThreadCount() + 1);
  ThreadPool* thread_pool = GetHeap()->GetThreadPool();
  Thread* self = Thread::Current();
  size_t count = copy_candidate_stack_->Size();
  if (thread_count > 1 && count > thread_count && enable_parallel_) {
     // Revoke Thread local buffers because parallel copying needs to reuse TLAB as PLAB.
     GetHeap()->RevokeAllThreadLocalBuffers();
     const size_t task_size = kMaxGcParalelTaskSize;
     for (size_t i = 0; i < count; i += task_size) {
       size_t end = std::min(i + task_size, count);
       auto* task = new ParallelForwardTask(this,
                                            copy_candidate_stack_->Begin(),
                                            i,
                                            end);
       thread_pool->AddTask(self, task);
     }
     thread_pool->SetMaxActiveWorkers(thread_count - 1);
     thread_pool->StartWorkers(self);
     thread_pool->Wait(self, true, true);
     thread_pool->StopWorkers(self);
   } else {
     // Visit all the marked objects in the from bump pointer space.
     ForwardObjectsVisitor visitor(this);
     objects_before_forwarding_->VisitMarkedRange(reinterpret_cast<uintptr_t>(from_bps_->Begin()),
                                                  reinterpret_cast<uintptr_t>(from_bps_->End()),
                                                  visitor);
   }
}

inline mirror::Object* MarkSweep::GetMarkedForwardAddress(mirror::Object* obj) const {
  if (immune_spaces_.ContainsObject(obj) || (to_bps_ != nullptr && to_bps_->HasAddress(obj))) {
    return obj;
  } else if (current_space_bitmap_->HasAddress(obj)) {
    LockWord lock_word = obj->GetLockWord(false);
    if (lock_word.GetState() != LockWord::kForwardingAddress) {
      return nullptr;
    }
    mirror::Object* ret =
        reinterpret_cast<mirror::Object*>(lock_word.ForwardingAddress());
    DCHECK(ret != nullptr);
    return ret;
  }
  DCHECK(!(to_bps_ != nullptr && to_bps_->HasAddress(obj)) &&
         !current_space_bitmap_->HasAddress(obj));
  return mark_bitmap_->Test(obj) ? obj : nullptr;
}

class MarkSweepUpdateRootVisitor : public RootVisitor {
 public:
  explicit MarkSweepUpdateRootVisitor(MarkSweep* collector) : collector_(collector) {
  }

  void VisitRoots(mirror::Object*** roots, size_t count, const RootInfo& info ATTRIBUTE_UNUSED)
      OVERRIDE REQUIRES(Locks::mutator_lock_)
      REQUIRES(Locks::heap_bitmap_lock_) {
    for (size_t i = 0; i < count; ++i) {
      mirror::Object* obj = *roots[i];
      mirror::Object* new_obj = collector_->GetMarkedForwardAddress(obj);
      if (obj != new_obj && new_obj != nullptr) {
        *roots[i] = new_obj;
        DCHECK(new_obj != nullptr);
      }
    }
  }

  void VisitRoots(mirror::CompressedReference<mirror::Object>** roots, size_t count,
                  const RootInfo& info ATTRIBUTE_UNUSED)
      OVERRIDE REQUIRES(Locks::mutator_lock_)
      REQUIRES(Locks::heap_bitmap_lock_) {
    for (size_t i = 0; i < count; ++i) {
      mirror::Object* obj = roots[i]->AsMirrorPtr();
      mirror::Object* new_obj = collector_->GetMarkedForwardAddress(obj);
      if (obj != new_obj && new_obj != nullptr) {
        roots[i]->Assign(new_obj);
        DCHECK(new_obj != nullptr);
      }
    }
  }

 private:
  MarkSweep* const collector_;
};

inline void MarkSweep::UpdateHeapReference(mirror::HeapReference<mirror::Object>* reference) {
  mirror::Object* obj = reference->AsMirrorPtr();
  if (obj != nullptr) {
    mirror::Object* new_obj = GetMarkedForwardAddress(obj);
    if (obj != new_obj && new_obj != nullptr) {
      reference->Assign(new_obj);
    }
  }
}

mirror::Object* MarkSweep::MarkedForwardingAddressCallback(mirror::Object* obj, void* arg) {
  return reinterpret_cast<MarkSweep*>(arg)->GetMarkedForwardAddress(obj);
}

class MarkSweepUpdateReferenceVisitor {
 public:
  explicit MarkSweepUpdateReferenceVisitor(MarkSweep* collector) : collector_(collector) {
  }

  void operator()(mirror::Object* obj, MemberOffset offset, bool /*is_static*/) const
      ALWAYS_INLINE REQUIRES(Locks::mutator_lock_, Locks::heap_bitmap_lock_) {
    collector_->UpdateHeapReference(obj->GetFieldObjectReferenceAddr<kVerifyNone>(offset));
  }

  void operator()(mirror::Class* /*klass*/, mirror::Reference* ref) const
      REQUIRES(Locks::mutator_lock_, Locks::heap_bitmap_lock_) {
    collector_->UpdateHeapReference(
        ref->GetFieldObjectReferenceAddr<kVerifyNone>(mirror::Reference::ReferentOffset()));
  }

  void VisitRootIfNonNull(mirror::CompressedReference<mirror::Object>* root) const
      NO_THREAD_SAFETY_ANALYSIS {
    if (!root->IsNull()) {
      VisitRoot(root);
    }
  }

  void VisitRoot(mirror::CompressedReference<mirror::Object>* root) const
      NO_THREAD_SAFETY_ANALYSIS {
    root->Assign(collector_->GetMarkedForwardAddress(root->AsMirrorPtr()));
  }

 private:
  MarkSweep* const collector_;
};

void MarkSweep::UpdateObjectReferences(mirror::Object* obj) {
  MarkSweepUpdateReferenceVisitor visitor(this);
  obj->VisitReferences(visitor, visitor);
}

class MarkSweepUpdateObjectReferencesVisitor {
 public:
  explicit MarkSweepUpdateObjectReferencesVisitor(MarkSweep* collector) : collector_(collector) {
  }
  void operator()(mirror::Object* obj) const SHARED_REQUIRES(Locks::heap_bitmap_lock_)
          REQUIRES(Locks::mutator_lock_) ALWAYS_INLINE {
    collector_->UpdateObjectReferences(obj);
  }

 private:
  MarkSweep* const collector_;
};

class RecursiveUpdateReferenceTask : public Task {
 public:
  RecursiveUpdateReferenceTask(MarkSweep* mark_sweep,
                               accounting::ContinuousSpaceBitmap* bitmap,
                               uintptr_t begin,
                               uintptr_t end)
      : mark_sweep_(mark_sweep),
        bitmap_(bitmap),
        begin_(begin),
        end_(end) {
    }

 protected:
  MarkSweep* const mark_sweep_;
  accounting::ContinuousSpaceBitmap* const bitmap_;
  const uintptr_t begin_;
  const uintptr_t end_;

  virtual void Finalize() {
    delete this;
  }

  virtual void Run(Thread* self) NO_THREAD_SAFETY_ANALYSIS {
    DCHECK_EQ(self, Thread::Current());
    MarkSweepUpdateObjectReferencesVisitor visitor(mark_sweep_);
    bitmap_->VisitMarkedRange(begin_, end_, visitor);
  }
};

// Process the "referent" field in a java.lang.ref.Reference.  If the referent has not yet been
// marked, put it on the appropriate list in the heap for later processing.
void MarkSweep::DelayReference(mirror::Class* klass, mirror::Reference* reference) {
  heap_->GetReferenceProcessor()->DelayReferenceReferent(klass, reference, this);
}

void MarkSweep::DelayReferenceCallback(mirror::Class* klass, mirror::Reference* ref,
                                               void* arg) {
  reinterpret_cast<MarkSweep*>(arg)->DelayReference(klass, ref);
}

// Used to verify that there's no references to the from-space.
class MarkSweepVerifyNoFromSpaceReferencesVisitor {
 public:
  explicit MarkSweepVerifyNoFromSpaceReferencesVisitor(space::ContinuousMemMapAllocSpace* from_space)
      : from_space_(from_space) {}

  void operator()(mirror::Object* obj, MemberOffset offset, bool /* is_static */) const
      SHARED_REQUIRES(Locks::mutator_lock_) ALWAYS_INLINE {
    mirror::Object* ref = obj->GetFieldObject<mirror::Object>(offset);
    if (from_space_->HasAddress(ref)) {
      Runtime::Current()->GetHeap()->DumpObject(LOG(INFO), obj);
      LOG(FATAL) << ref << " found in from space";
    }
  }

  void VisitRootIfNonNull(mirror::CompressedReference<mirror::Object>* root) const
      SHARED_REQUIRES(Locks::mutator_lock_) {
    if (!root->IsNull()) {
      VisitRoot(root);
    }
  }

  void VisitRoot(mirror::CompressedReference<mirror::Object>* root) const
      SHARED_REQUIRES(Locks::mutator_lock_) {
    mirror::Object* obj = root->AsMirrorPtr();
    if (from_space_->HasAddress(obj)) {
      Runtime::Current()->GetHeap()->DumpObject(LOG(INFO), obj);
      LOG(FATAL) << obj << " found in from space";
    }
  }

 private:
  space::ContinuousMemMapAllocSpace* from_space_;
};

void MarkSweep::VerifyNoFromSpaceReferences(mirror::Object* obj) {
  CHECK(!from_bps_->HasAddress(obj)) << "Scanning object "
      << obj << " in from bump pointer space";
  MarkSweepVerifyNoFromSpaceReferencesVisitor visitor(from_bps_);
  obj->VisitReferences(visitor, VoidFunctor());
}

class MarkSweepVerifyNoFromSpaceReferencesObjectVisitor {
 public:
  explicit MarkSweepVerifyNoFromSpaceReferencesObjectVisitor(MarkSweep* mark_sweep)
      : mark_sweep_(mark_sweep) {}
  void operator()(mirror::Object* obj) const
      SHARED_REQUIRES(Locks::heap_bitmap_lock_, Locks::mutator_lock_) {
    DCHECK(obj != nullptr);
    mark_sweep_->VerifyNoFromSpaceReferences(obj);
  }
 private:
  MarkSweep* const mark_sweep_;
};

void MarkSweep::UpdateReferences() {
  TimingLogger::ScopedTiming t1(__FUNCTION__, GetTimings());
  updating_reference_ = true;
  Runtime* runtime = Runtime::Current();
  // Update roots.
  MarkSweepUpdateRootVisitor update_root_visitor(this);
  runtime->VisitRoots(&update_root_visitor);
  space::RosAllocSpace* ros = nullptr;
  // Update object references in mod union tables and spaces.
  for (const auto& space : heap_->GetContinuousSpaces()) {
    // If the space is immune then we need to mark the references to other spaces.
    accounting::ModUnionTable* table = heap_->FindModUnionTableFromSpace(space);
    accounting::RememberedSet* rem_set = heap_->FindRememberedSetFromSpace(space);
    if (table != nullptr) {
      // TODO: Improve naming.
      TimingLogger::ScopedTiming t2(
          space->IsZygoteSpace() ? "UpdateZygoteModUnionTableReferences" :
                                   "UpdateImageModUnionTableReferences",
                                   GetTimings());
      table->UpdateAndMarkReferences(this);
      DCHECK(rem_set == nullptr);
    } else if (rem_set != nullptr &&
               (space->IsRosAllocSpace() || space == heap_->GetNonMovingSpace())) {
      TimingLogger::ScopedTiming t2("UpdateAndMarkRememberedSet", GetTimings());
      rem_set->UpdateAndMarkReferences(from_bps_, this, false);
      if (space->IsRosAllocSpace()) {
        ros = space->AsRosAllocSpace();
      }
    } else if (space == to_bps_) {
      TimingLogger::ScopedTiming t2("VisitToBumpPointerSpaceObjects", GetTimings());
      size_t thread_count = (heap_->GetThreadPool() == nullptr) ? 1 :
                            (heap_->GetParallelGCThreadCount() + 1);
      if (thread_count > 1) {
        Thread* self = Thread::Current();
        ThreadPool* thread_pool = GetHeap()->GetThreadPool();
        uintptr_t begin = reinterpret_cast<uintptr_t>(to_bps_->Begin());
        uintptr_t end = reinterpret_cast<uintptr_t>(to_bps_->End());
        // Create thread_count worker tasks as we copy with bump pointer.
        // So the bitmap should be dense.
        const size_t n = thread_count;
        uintptr_t start = begin;
        uintptr_t delta = (end - begin) / n;
        while (begin != end) {
          start = begin;
          delta = RoundUp(delta, KB);
          if (delta < kMinRemainBytes) delta = end - begin;
          if (begin + delta > end) delta = end - begin;
          begin += delta;
          auto* task = new RecursiveUpdateReferenceTask(this,
                                                        objects_after_forwarding_.get(),
                                                        start,
                                                        begin);
          thread_pool->AddTask(self, task);
        }
        thread_pool->SetMaxActiveWorkers(thread_count - 1);
        thread_pool->StartWorkers(self);
        thread_pool->Wait(self, true, true);
        thread_pool->StopWorkers(self);
      } else {
        MarkSweepUpdateObjectReferencesVisitor from_visitor(this);
        objects_after_forwarding_->VisitMarkedRange(reinterpret_cast<uintptr_t>(to_bps_->Begin()),
                                                    reinterpret_cast<uintptr_t>(to_bps_->End()),
                                                    from_visitor);
      }
    } else {
      // Nothing to do for From bump pointer space.
    }
  }
  if (kIsDebugBuild && ros != nullptr) {
    // Verify that there are no from-space references that
    // remain in the space, that is, the remembered set (and the
    // card table) didn't miss any from-space references in the
    // space.
    TimingLogger::ScopedTiming t3("VerifyRosObjects", GetTimings());
    accounting::ContinuousSpaceBitmap* mark_bitmap = ros->GetMarkBitmap();
    MarkSweepVerifyNoFromSpaceReferencesObjectVisitor visitor(this);
    mark_bitmap->VisitMarkedRange(reinterpret_cast<uintptr_t>(ros->Begin()),
                                  reinterpret_cast<uintptr_t>(ros->End()),
                                  visitor);
  }
  space::LargeObjectSpace* los = GetHeap()->GetLargeObjectsSpace();
  if (los != nullptr) {
    TimingLogger::ScopedTiming t4("VisitLargeObjects", GetTimings());
    accounting::LargeObjectBitmap* large_mark_bitmap = los->GetMarkBitmap();
    MarkSweepUpdateObjectReferencesVisitor los_visitor(this);
    large_mark_bitmap->VisitMarkedRange(reinterpret_cast<uintptr_t>(los->Begin()),
                                        reinterpret_cast<uintptr_t>(los->End()),
                                        los_visitor);
  }
  // Update the reference processor cleared list.
  heap_->GetReferenceProcessor()->UpdateRoots(this);
  // Update and sweep the system weaks.
  runtime->SweepSystemWeaks(this);
  if (kIsDebugBuild && los != nullptr) {
    TimingLogger::ScopedTiming t5("VerifyLargeObjects", GetTimings());
    accounting::LargeObjectBitmap* large_mark_bitmap = los->GetMarkBitmap();
    MarkSweepVerifyNoFromSpaceReferencesObjectVisitor los_visitor(this);
    large_mark_bitmap->VisitMarkedRange(reinterpret_cast<uintptr_t>(los->Begin()),
                                        reinterpret_cast<uintptr_t>(los->End()),
                                        los_visitor);
  }
  updating_reference_ = false;
  // The above references update in modunion table or rem set may put objects
  // in mark stack. Reset mark stack after the references are updated already.
  mark_stack_->Reset();
}

class VerifyRootMarkedVisitorAfterCopying : public SingleRootVisitor {
 public:
  explicit VerifyRootMarkedVisitorAfterCopying(MarkSweep* collector) : collector_(collector) { }

  void VisitRoot(mirror::Object* root, const RootInfo& info) OVERRIDE
      SHARED_REQUIRES(Locks::mutator_lock_, Locks::heap_bitmap_lock_) {
    CHECK(collector_->IsMarkedAfterCopying(root)) << info.ToString();
  }

 private:
  MarkSweep* const collector_;
};

class VerifyRootMarkedVisitor : public SingleRootVisitor {
 public:
  explicit VerifyRootMarkedVisitor(MarkSweep* collector) : collector_(collector) { }

  void VisitRoot(mirror::Object* root, const RootInfo& info) OVERRIDE
      SHARED_REQUIRES(Locks::mutator_lock_, Locks::heap_bitmap_lock_) {
    CHECK(collector_->IsMarked(root)) << info.ToString();
  }

 private:
  MarkSweep* const collector_;
};

void MarkSweep::PausePhase() {
  TimingLogger::ScopedTiming t1("(Paused)PausePhase", GetTimings());
  Thread* self = Thread::Current();
  Locks::mutator_lock_->AssertExclusiveHeld(self);
  if (IsConcurrent()) {
    // Handle the dirty objects if we are a concurrent GC.
    WriterMutexLock mu(self, *Locks::heap_bitmap_lock_);
    // Re-mark root set.
    ReMarkRoots();
    // Update mod union tables and remember sets in order to update references later.
    if (IsCopying()) {
      // TODO: In pause phase if we can record dirty cards for only rememberedset
      // and modunion tables, this will save paused time, because no CAS and no change from
      // dirty to dirty - 1.
      heap_->ProcessCards(GetTimings(), true, false, false);
    }
    // Scan dirty objects, this is only required if we are doing concurrent GC.
    RecursiveMarkDirtyObjects(true, accounting::CardTable::kCardDirty);
  }
  {
    TimingLogger::ScopedTiming t2("SwapStacks", GetTimings());
    WriterMutexLock mu(self, *Locks::heap_bitmap_lock_);
    heap_->SwapStacks();
    live_stack_freeze_size_ = heap_->GetLiveStack()->Size();
    // Need to revoke all the thread local allocation stacks since we just swapped the allocation
    // stacks and don't want anybody to allocate into the live stack.
    RevokeAllThreadLocalAllocationStacks(self);
  }
  if (kVerifyRootsMarked) {
    TimingLogger::ScopedTiming t_verify("(Paused)VerifyRoots before copying", GetTimings());
    VerifyRootMarkedVisitor visitor_verify(this);
    Runtime::Current()->VisitRoots(&visitor_verify);
  }
  // Copying between two bump pointer spaces and fixup the references.
  if (IsCopying()) {
    // At the beginning, no need to revoke thread local buffers, because we will
    // forward live objects to another empty bump pointer space.
    TimingLogger::ScopedTiming t3("copy bump pointer space", GetTimings());
    WriterMutexLock mu(self, *Locks::heap_bitmap_lock_);
    GetHeap()->GetReferenceProcessor()->ProcessReferences(
        false,
        GetTimings(),
        GetCurrentIteration()->GetClearSoftReferences(),
        this);


    ForwardObjects();
    // Update remember set of ros.
    {
      space::RosAllocSpace* ros = heap_->GetRosAllocSpace();
      accounting::RememberedSet* rem_set = heap_->FindRememberedSetFromSpace(ros);
      // TODO: No need to change to dirty - 1 to avoid CAS.
      if (rem_set != nullptr) {
        rem_set->ClearCards();
      }
    }
    UpdateReferences();
    // Revoke buffers before measuring how many objects were moved since the TLABs needs
    // to be revoked before they are properly counted.
    // Also the old enough objects may be promoted into ros,
    // and this helps prevent fragmentation.
    GetHeap()->RevokeAllThreadLocalBuffers();
    // Record freed memory.
    const size_t from_objects = from_bps_->GetObjectsAllocated();
    const size_t to_objects = objects_moved_;
    CHECK_LE(to_objects, from_objects);
    // Note: Freed bytes can be negative because sometimes the promoted objects will be large.
    int64_t freed_bytes_bps = static_cast<int64_t>(from_bps_->Size()) -
                              static_cast<int64_t>(to_bps_->Size()) -
                              static_cast<int64_t>((bytes_promoted_));
    RecordFree(ObjectBytePair(from_objects - to_objects, freed_bytes_bps));
    // Clear and protect the from space.
    from_bps_->Clear();
    // Protect with PROT_NONE.
    VLOG(heap) << "Protecting from_space_ : " << *from_bps_ << " to_space: " << *to_bps_;
    from_bps_->GetMemMap()->Protect(PROT_NONE);
  }
  if (kVerifyRootsMarked) {
    TimingLogger::ScopedTiming t_verifyAfterCopying("(Paused)VerifyRoots after copying", GetTimings());
    VerifyRootMarkedVisitorAfterCopying visitorAfterCopying(this);
    Runtime::Current()->VisitRoots(&visitorAfterCopying);
  }
  heap_->PreSweepingGcVerification(this);
  if (!IsCopying()) {
    // Disallow new system weaks to prevent a race which occurs when someone adds a new system
    // weak before we sweep them. Since this new system weak may not be marked, the GC may
    // incorrectly sweep it. This also fixes a race where interning may attempt to return a strong
    // reference to a string that is about to be swept.
    Runtime::Current()->DisallowNewSystemWeaks();
    // Enable the reference processing slow path, needs to be done with mutators paused since there
    // is no lock in the GetReferent fast path.
    GetHeap()->GetReferenceProcessor()->EnableSlowPath();
  } else {
    from_bps_->GetAgingTable()->Reset();
    heap_->SwapSemiSpaces();
  }
  const GcType gc_type = GetGcType();
  const space::RosAllocSpace* ros_space = GetHeap()->GetRosAllocSpace();
  if (ros_space != nullptr && (gc_type == kGcTypePartial || gc_type == kGcTypeFull)) {
    allocator::RosAlloc* rosalloc = ros_space->GetRosAlloc();
    DCHECK(rosalloc != nullptr);
    rosalloc->SetPageMapSizeSnapshot();
  }
}

void MarkSweep::PreCleanCards() {
  // Don't do this for non concurrent GCs since they don't have any dirty cards.
  if (kPreCleanCards && IsConcurrent()) {
    TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
    Thread* self = Thread::Current();
    CHECK(!Locks::mutator_lock_->IsExclusiveHeld(self));
    // Process dirty cards and add dirty cards to mod union tables, also ages cards.
    if (IsCopying()) {
      heap_->ProcessCards(GetTimings(), true, true, false);
    } else {
      heap_->ProcessCards(GetTimings(), false, true, false);
    }
    // The checkpoint root marking is required to avoid a race condition which occurs if the
    // following happens during a reference write:
    // 1. mutator dirties the card (write barrier)
    // 2. GC ages the card (the above ProcessCards call)
    // 3. GC scans the object (the RecursiveMarkDirtyObjects call below)
    // 4. mutator writes the value (corresponding to the write barrier in 1.)
    // This causes the GC to age the card but not necessarily mark the reference which the mutator
    // wrote into the object stored in the card.
    // Having the checkpoint fixes this issue since it ensures that the card mark and the
    // reference write are visible to the GC before the card is scanned (this is due to locks being
    // acquired / released in the checkpoint code).
    // Run checkpoint except marking roots again just to avoid the above race condition.
    MarkRootsCheckpoint(self, false /*not revoke thread local buffer*/,
                        false /*not mark thread roots*/);
    // The other roots are also marked to help reduce the pause.
    // Except new roots, no need to mark other roots again, otherwise, it will increase GC duration
    // and also probably dirty cards which will increase pause time accordingly.
    MarkConcurrentRoots(
        static_cast<VisitRootFlags>(kVisitRootFlagClearRootLog | kVisitRootFlagNewRoots));
    // Process the newly aged cards.
    RecursiveMarkDirtyObjects(false, accounting::CardTable::kCardDirty - 1);
    // TODO: Empty allocation stack to reduce the number of objects we need to test / mark as live
    // in the next GC.
  }
}

void MarkSweep::RevokeAllThreadLocalAllocationStacks(Thread* self) {
  if (kUseThreadLocalAllocationStack) {
    TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
    Locks::mutator_lock_->AssertExclusiveHeld(self);
    heap_->RevokeAllThreadLocalAllocationStacks(self);
  }
}

void MarkSweep::MarkingPhase() {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  Thread* self = Thread::Current();
  BindBitmaps();
  FindDefaultSpaceBitmap();
  // Process dirty cards and add dirty cards to mod union tables.
  // If the GC type is non sticky, then we just clear the cards instead of aging them.
  // Generational Copying GC doesn't have sticky GC. And it doesn't need
  // dirty cards of bump pointer spaces if it's not concurrent.
  if (IsCopying()) {
    heap_->ProcessCards(GetTimings(), true, true, true);
  } else {
    heap_->ProcessCards(GetTimings(), false, true, GetGcType() != kGcTypeSticky);
  }
  WriterMutexLock mu(self, *Locks::heap_bitmap_lock_);
  MarkRoots(self);
  MarkReachableObjects();
  // Pre-clean dirtied cards to reduce pauses.
  PreCleanCards();
}

class ScanObjectVisitor {
 public:
  explicit ScanObjectVisitor(MarkSweep* const mark_sweep) ALWAYS_INLINE
      : mark_sweep_(mark_sweep) {}

  void operator()(mirror::Object* obj) const
      ALWAYS_INLINE
      REQUIRES(Locks::heap_bitmap_lock_)
      SHARED_REQUIRES(Locks::mutator_lock_) {
    if (kCheckLocks) {
      Locks::mutator_lock_->AssertSharedHeld(Thread::Current());
      Locks::heap_bitmap_lock_->AssertExclusiveHeld(Thread::Current());
    }
    mark_sweep_->ScanObject(obj);
  }

 private:
  MarkSweep* const mark_sweep_;
};

void MarkSweep::UpdateAndMarkModUnion() {
  for (const auto& space : immune_spaces_.GetSpaces()) {
    const char* name = space->IsZygoteSpace()
        ? "UpdateAndMarkZygoteModUnionTable"
        : "UpdateAndMarkImageModUnionTable";
    DCHECK(space->IsZygoteSpace() || space->IsImageSpace()) << *space;
    TimingLogger::ScopedTiming t(name, GetTimings());
    accounting::ModUnionTable* mod_union_table = heap_->FindModUnionTableFromSpace(space);
    if (mod_union_table != nullptr) {
      mod_union_table->UpdateAndMarkReferences(this);
    } else {
      // No mod-union table, scan all the live bits. This can only occur for app images.
      space->GetLiveBitmap()->VisitMarkedRange(reinterpret_cast<uintptr_t>(space->Begin()),
                                               reinterpret_cast<uintptr_t>(space->End()),
                                               ScanObjectVisitor(this));
    }
  }
}

void MarkSweep::MarkReachableObjects() {
  UpdateAndMarkModUnion();
  // Recursively mark all the non-image bits set in the mark bitmap.
  RecursiveMark();
}

void MarkSweep::ReclaimPhase() {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  Thread* const self = Thread::Current();
  Runtime* const runtime = Runtime::Current();
  if (!IsCopying()) {
    // Process the references concurrently.
    ProcessReferences(self);
    SweepSystemWeaks(self);
    runtime->AllowNewSystemWeaks();
  }
  // Clean up class loaders after system weaks are swept since that is how we know if class
  // unloading occurred.
  runtime->GetClassLinker()->CleanupClassLoaders();
  {
    WriterMutexLock mu(self, *Locks::heap_bitmap_lock_);
    GetHeap()->RecordFreeRevoke();
    // Reclaim unmarked objects.
    Sweep(false);
    // Swap the live and mark bitmaps for each space which we modified space. This is an
    // optimization that enables us to not clear live bits inside of the sweep. Only swaps unbound
    // bitmaps.
    SwapBitmaps();
    // Unbind the live and mark bitmaps.
    GetHeap()->UnBindBitmaps();
  }
}

void MarkSweep::FindDefaultSpaceBitmap() {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  for (const auto& space : GetHeap()->GetContinuousSpaces()) {
    accounting::ContinuousSpaceBitmap* bitmap = space->GetMarkBitmap();
    // We want to have the main space instead of non moving if possible.
    if (bitmap != nullptr &&
        space->GetGcRetentionPolicy() == space::kGcRetentionPolicyAlwaysCollect) {
      if (space->IsRosAllocSpace()) {
        current_space_bitmap_ = bitmap;
      }
    }
  }
  if (GetCollectorType() == kCollectorTypeGenCopying) {
    current_space_bitmap_ = objects_before_forwarding_.get();
  }
  CHECK(current_space_bitmap_ != nullptr) << "Could not find a default mark bitmap\n"
      << heap_->DumpSpaces();
}

void MarkSweep::ExpandMarkStack() {
  ResizeMarkStack(mark_stack_->Capacity() * 2);
}

void MarkSweep::ResizeMarkStack(size_t new_size) {
  // Rare case, no need to have Thread::Current be a parameter.
  if (UNLIKELY(mark_stack_->Size() < mark_stack_->Capacity())) {
    // Someone else acquired the lock and expanded the mark stack before us.
    return;
  }
  std::vector<StackReference<mirror::Object>> temp(mark_stack_->Begin(), mark_stack_->End());
  CHECK_LE(mark_stack_->Size(), new_size);
  mark_stack_->Resize(new_size);
  for (auto& obj : temp) {
    mark_stack_->PushBack(obj.AsMirrorPtr());
  }
}

mirror::Object* MarkSweep::MarkObject(mirror::Object* obj) {
  MarkObject(obj, nullptr, MemberOffset(0));
  mirror::Object* forward_address = GetMarkedForwardAddress(obj);
  if (forward_address != nullptr) {
    return forward_address;
  }
  return obj;
}

inline void MarkSweep::MarkObjectNonNullParallel(mirror::Object* obj) {
  DCHECK(obj != nullptr);
  if (MarkObjectParallel(obj)) {
    MutexLock mu(Thread::Current(), mark_stack_lock_);
    if (UNLIKELY(mark_stack_->Size() >= mark_stack_->Capacity())) {
      ExpandMarkStack();
    }
    // The object must be pushed on to the mark stack.
    mark_stack_->PushBack(obj);
  }
}

bool MarkSweep::IsMarkedHeapReference(mirror::HeapReference<mirror::Object>* ref) {
  mirror::Object* obj = ref->AsMirrorPtr();
  mirror::Object* new_obj = IsMarked(obj);
  if (new_obj == nullptr) {
    return false;
  }
  if (new_obj != obj) {
    // Write barrier is not necessary since it still points to the same object, just at a different
    // address.
    ref->Assign(new_obj);
  }
  return true;
}

class MarkSweepMarkObjectSlowPath {
 public:
  explicit MarkSweepMarkObjectSlowPath(MarkSweep* mark_sweep,
                                       mirror::Object* holder = nullptr,
                                       MemberOffset offset = MemberOffset(0))
      : mark_sweep_(mark_sweep), holder_(holder), offset_(offset) {}

  void operator()(const mirror::Object* obj) const NO_THREAD_SAFETY_ANALYSIS {
    if (kProfileLargeObjects) {
      // TODO: Differentiate between marking and testing somehow.
      ++mark_sweep_->large_object_test_;
      ++mark_sweep_->large_object_mark_;
    }
    space::LargeObjectSpace* large_object_space = mark_sweep_->GetHeap()->GetLargeObjectsSpace();
    if (UNLIKELY(obj == nullptr || !IsAligned<kPageSize>(obj) ||
                 (kIsDebugBuild && large_object_space != nullptr &&
                     !large_object_space->Contains(obj)))) {
      LOG(INTERNAL_FATAL) << "Tried to mark " << obj << " not contained by any spaces";
      if (holder_ != nullptr) {
        size_t holder_size = holder_->SizeOf();
        ArtField* field = holder_->FindFieldByOffset(offset_);
        LOG(INTERNAL_FATAL) << "Field info: "
                            << " holder=" << holder_
                            << " holder is "
                            << (mark_sweep_->GetHeap()->IsLiveObjectLocked(holder_)
                                ? "alive" : "dead")
                            << " holder_size=" << holder_size
                            << " holder_type=" << PrettyTypeOf(holder_)
                            << " offset=" << offset_.Uint32Value()
                            << " field=" << (field != nullptr ? field->GetName() : "nullptr")
                            << " field_type="
                            << (field != nullptr ? field->GetTypeDescriptor() : "")
                            << " first_ref_field_offset="
                            << (holder_->IsClass()
                                ? holder_->AsClass()->GetFirstReferenceStaticFieldOffset(
                                    sizeof(void*))
                                : holder_->GetClass()->GetFirstReferenceInstanceFieldOffset())
                            << " num_of_ref_fields="
                            << (holder_->IsClass()
                                ? holder_->AsClass()->NumReferenceStaticFields()
                                : holder_->GetClass()->NumReferenceInstanceFields())
                            << "\n";
        // Print the memory content of the holder.
        for (size_t i = 0; i < holder_size / sizeof(uint32_t); ++i) {
          uint32_t* p = reinterpret_cast<uint32_t*>(holder_);
          LOG(INTERNAL_FATAL) << &p[i] << ": " << "holder+" << (i * sizeof(uint32_t)) << " = "
                              << std::hex << p[i];
        }
      }
      PrintFileToLog("/proc/self/maps", LogSeverity::INTERNAL_FATAL);
      MemMap::DumpMaps(LOG(INTERNAL_FATAL), true);
      {
        LOG(INTERNAL_FATAL) << "Attempting see if it's a bad root";
        Thread* self = Thread::Current();
        if (Locks::mutator_lock_->IsExclusiveHeld(self)) {
          mark_sweep_->VerifyRoots();
        } else {
          const bool heap_bitmap_exclusive_locked =
              Locks::heap_bitmap_lock_->IsExclusiveHeld(self);
          if (heap_bitmap_exclusive_locked) {
            Locks::heap_bitmap_lock_->ExclusiveUnlock(self);
          }
          {
            ScopedThreadSuspension(self, kSuspended);
            ScopedSuspendAll ssa(__FUNCTION__);
            mark_sweep_->VerifyRoots();
          }
          if (heap_bitmap_exclusive_locked) {
            Locks::heap_bitmap_lock_->ExclusiveLock(self);
          }
        }
      }
      LOG(FATAL) << "Can't mark invalid object";
    }
  }

 private:
  MarkSweep* const mark_sweep_;
  mirror::Object* const holder_;
  MemberOffset offset_;
};

inline void MarkSweep::MarkObjectNonNull(mirror::Object* obj,
                                         mirror::Object* holder,
                                         MemberOffset offset) {
  DCHECK(obj != nullptr);
  if (kUseBakerOrBrooksReadBarrier) {
    // Verify all the objects have the correct pointer installed.
    obj->AssertReadBarrierPointer();
  }
  if (immune_spaces_.IsInImmuneRegion(obj)) {
    if (kCountMarkedObjects) {
      ++mark_immune_count_;
    }
    DCHECK(mark_bitmap_->Test(obj));
  } else if (LIKELY(current_space_bitmap_->HasAddress(obj))) {
    if (kCountMarkedObjects) {
      ++mark_fastpath_count_;
    }
    if (UNLIKELY(!current_space_bitmap_->Set(obj))) {
      PushOnMarkStack(obj);  // This object was not previously marked.
      // Record the object in from space as copy candidate for parallel copy.
      if (IsCopying() && enable_parallel_ && current_space_bitmap_ ==  objects_before_forwarding_.get()) {
        copy_candidate_stack_->PushBack(obj);
        if (copy_candidate_stack_->IsFull()) {
          LOG(WARNING) << "diable parallel because of copy stack limitation!";
          enable_parallel_ = false;
        }
      }
    }
  } else {
    if (kCountMarkedObjects) {
      ++mark_slowpath_count_;
    }
    MarkSweepMarkObjectSlowPath visitor(this, holder, offset);
    // TODO: We already know that the object is not in the current_space_bitmap_ but MarkBitmap::Set
    // will check again.
    if (!mark_bitmap_->Set(obj, visitor)) {
      PushOnMarkStack(obj);  // Was not already marked, push.
    }
  }
}

inline void MarkSweep::PushOnMarkStack(mirror::Object* obj) {
  if (UNLIKELY(mark_stack_->Size() >= mark_stack_->Capacity())) {
    // Lock is not needed but is here anyways to please annotalysis.
    MutexLock mu(Thread::Current(), mark_stack_lock_);
    ExpandMarkStack();
  }
  // The object must be pushed on to the mark stack.
  mark_stack_->PushBack(obj);
}

inline bool MarkSweep::MarkObjectParallel(mirror::Object* obj) {
  DCHECK(obj != nullptr);
  if (kUseBakerOrBrooksReadBarrier) {
    // Verify all the objects have the correct pointer installed.
    obj->AssertReadBarrierPointer();
  }
  if (immune_spaces_.IsInImmuneRegion(obj)) {
    DCHECK(IsMarked(obj) != nullptr);
    return false;
  }
  // Try to take advantage of locality of references within a space, failing this find the space
  // the hard way.
  accounting::ContinuousSpaceBitmap* object_bitmap = current_space_bitmap_;
  if (LIKELY(object_bitmap->HasAddress(obj))) {
    bool ret = !object_bitmap->AtomicTestAndSet(obj);
    if (IsCopying() && enable_parallel_ &&ret && object_bitmap == objects_before_forwarding_.get()) {
      if (!copy_candidate_stack_->AtomicPushBack(obj)) {
        LOG(WARNING) << "diable parallel because of copy stack limitation!";
        enable_parallel_ = false;
      }
    }
    return ret;
  }
  MarkSweepMarkObjectSlowPath visitor(this);
  return !mark_bitmap_->AtomicTestAndSet(obj, visitor);
}

void MarkSweep::MarkHeapReference(mirror::HeapReference<mirror::Object>* ref) {
  MarkObject(ref->AsMirrorPtr(), nullptr, MemberOffset(0));
  if (updating_reference_) {
    UpdateHeapReference(ref);
  }
}

// Used to mark objects when processing the mark stack. If an object is null, it is not marked.
inline void MarkSweep::MarkObject(mirror::Object* obj,
                                  mirror::Object* holder,
                                  MemberOffset offset) {
  if (obj != nullptr) {
    MarkObjectNonNull(obj, holder, offset);
  } else if (kCountMarkedObjects) {
    ++mark_null_count_;
  }
}

void MarkSweep::VisitRoots(mirror::Object*** roots,
                           size_t count,
                           const RootInfo& info ATTRIBUTE_UNUSED) {
  for (size_t i = 0; i < count; ++i) {
    MarkObjectNonNull(*roots[i]);
  }
}

void MarkSweep::VisitRoots(mirror::CompressedReference<mirror::Object>** roots,
                           size_t count,
                           const RootInfo& info ATTRIBUTE_UNUSED) {
  for (size_t i = 0; i < count; ++i) {
    MarkObjectNonNull(roots[i]->AsMirrorPtr());
  }
}

class VerifyRootVisitor : public SingleRootVisitor {
 public:
  void VisitRoot(mirror::Object* root, const RootInfo& info) OVERRIDE
      SHARED_REQUIRES(Locks::mutator_lock_, Locks::heap_bitmap_lock_) {
    // See if the root is on any space bitmap.
    auto* heap = Runtime::Current()->GetHeap();
    if (heap->GetLiveBitmap()->GetContinuousSpaceBitmap(root) == nullptr) {
      space::LargeObjectSpace* large_object_space = heap->GetLargeObjectsSpace();
      if (large_object_space != nullptr && !large_object_space->Contains(root)) {
        LOG(INTERNAL_FATAL) << "Found invalid root: " << root << " " << info;
      }
    }
  }
};

void MarkSweep::VerifyRoots() {
  VerifyRootVisitor visitor;
  Runtime::Current()->GetThreadList()->VisitRoots(&visitor);
}

inline bool MarkSweep::IsMarkedAfterCopying(const mirror::Object* object) const {
  if (immune_spaces_.ContainsObject(object)) {
    return true;
  }
  if (objects_after_forwarding_ != nullptr &&
             objects_after_forwarding_->HasAddress(object)) {
    return objects_after_forwarding_->Test(object);
  }
  return mark_bitmap_->Test(object);
}

void MarkSweep::MarkRoots(Thread* self) {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  if (Locks::mutator_lock_->IsExclusiveHeld(self)) {
    // If we exclusively hold the mutator lock, all threads must be suspended.
    Runtime::Current()->VisitRoots(this);
    RevokeAllThreadLocalAllocationStacks(self);
  } else {
    MarkRootsCheckpoint(self, kRevokeRosAllocThreadLocalBuffersAtCheckpoint,
                        true /*mark thread roots*/);
    // At this point the live stack should no longer have any mutators which push into it.
    MarkNonThreadRoots();
    MarkConcurrentRoots(
        static_cast<VisitRootFlags>(kVisitRootFlagAllRoots | kVisitRootFlagStartLoggingNewRoots));
  }
}

void MarkSweep::MarkNonThreadRoots() {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  Runtime::Current()->VisitNonThreadRoots(this);
}

void MarkSweep::MarkConcurrentRoots(VisitRootFlags flags) {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  // Visit all runtime roots and clear dirty flags.
  Runtime::Current()->VisitConcurrentRoots(
      this, static_cast<VisitRootFlags>(flags | kVisitRootFlagNonMoving));
}

class DelayReferenceReferentVisitor {
 public:
  explicit DelayReferenceReferentVisitor(MarkSweep* collector) : collector_(collector) {}

  void operator()(mirror::Class* klass, mirror::Reference* ref) const
      REQUIRES(Locks::heap_bitmap_lock_)
      SHARED_REQUIRES(Locks::mutator_lock_) {
    collector_->DelayReferenceReferent(klass, ref);
  }

 private:
  MarkSweep* const collector_;
};

template <bool kUseFinger = false>
class MarkStackTask : public Task {
 public:
  MarkStackTask(ThreadPool* thread_pool,
                MarkSweep* mark_sweep,
                size_t mark_stack_size,
                StackReference<mirror::Object>* mark_stack)
      : mark_sweep_(mark_sweep),
        thread_pool_(thread_pool),
        mark_stack_pos_(mark_stack_size) {
    // We may have to copy part of an existing mark stack when another mark stack overflows.
    if (mark_stack_size != 0) {
      DCHECK(mark_stack != nullptr);
      // TODO: Check performance?
      std::copy(mark_stack, mark_stack + mark_stack_size, mark_stack_);
    }
    if (kCountTasks) {
      ++mark_sweep_->work_chunks_created_;
    }
  }

  static const size_t kMaxSize = 1 * KB;

 protected:
  class MarkObjectParallelVisitor {
   public:
    ALWAYS_INLINE MarkObjectParallelVisitor(MarkStackTask<kUseFinger>* chunk_task,
                                            MarkSweep* mark_sweep)
        : chunk_task_(chunk_task), mark_sweep_(mark_sweep) {}

    ALWAYS_INLINE void operator()(mirror::Object* obj,
                    MemberOffset offset,
                    bool is_static ATTRIBUTE_UNUSED) const
        SHARED_REQUIRES(Locks::mutator_lock_) {
      Mark(obj->GetFieldObject<mirror::Object>(offset));
    }

    void VisitRootIfNonNull(mirror::CompressedReference<mirror::Object>* root) const
        SHARED_REQUIRES(Locks::mutator_lock_) {
      if (!root->IsNull()) {
        VisitRoot(root);
      }
    }

    void VisitRoot(mirror::CompressedReference<mirror::Object>* root) const
        SHARED_REQUIRES(Locks::mutator_lock_) {
      Mark(root->AsMirrorPtr());
    }

   private:
    ALWAYS_INLINE void Mark(mirror::Object* ref) const SHARED_REQUIRES(Locks::mutator_lock_) {
      if (ref != nullptr && mark_sweep_->MarkObjectParallel(ref)) {
        if (kUseFinger) {
          std::atomic_thread_fence(std::memory_order_seq_cst);
          if (reinterpret_cast<uintptr_t>(ref) >=
              static_cast<uintptr_t>(mark_sweep_->atomic_finger_.LoadRelaxed())) {
            return;
          }
        }
        chunk_task_->MarkStackPush(ref);
      }
    }

    MarkStackTask<kUseFinger>* const chunk_task_;
    MarkSweep* const mark_sweep_;
  };

  class ScanObjectParallelVisitor {
   public:
    ALWAYS_INLINE explicit ScanObjectParallelVisitor(MarkStackTask<kUseFinger>* chunk_task)
        : chunk_task_(chunk_task) {}

    // No thread safety analysis since multiple threads will use this visitor.
    void operator()(mirror::Object* obj) const
        REQUIRES(Locks::heap_bitmap_lock_)
        SHARED_REQUIRES(Locks::mutator_lock_) {
      MarkSweep* const mark_sweep = chunk_task_->mark_sweep_;
      MarkObjectParallelVisitor mark_visitor(chunk_task_, mark_sweep);
      DelayReferenceReferentVisitor ref_visitor(mark_sweep);
      mark_sweep->ScanObjectVisit(obj, mark_visitor, ref_visitor);
    }

   private:
    MarkStackTask<kUseFinger>* const chunk_task_;
  };

  virtual ~MarkStackTask() {
    // Make sure that we have cleared our mark stack.
    DCHECK_EQ(mark_stack_pos_, 0U);
    if (kCountTasks) {
      ++mark_sweep_->work_chunks_deleted_;
    }
  }

  MarkSweep* const mark_sweep_;
  ThreadPool* const thread_pool_;
  // Thread local mark stack for this task.
  StackReference<mirror::Object> mark_stack_[kMaxSize];
  // Mark stack position.
  size_t mark_stack_pos_;

  ALWAYS_INLINE void MarkStackPush(mirror::Object* obj)
      SHARED_REQUIRES(Locks::mutator_lock_) {
    if (UNLIKELY(mark_stack_pos_ == kMaxSize)) {
      // Mark stack overflow, give 1/2 the stack to the thread pool as a new work task.
      mark_stack_pos_ /= 2;
      auto* task = new MarkStackTask(thread_pool_,
                                     mark_sweep_,
                                     kMaxSize - mark_stack_pos_,
                                     mark_stack_ + mark_stack_pos_);
      thread_pool_->AddTask(Thread::Current(), task);
    }
    DCHECK(obj != nullptr);
    DCHECK_LT(mark_stack_pos_, kMaxSize);
    mark_stack_[mark_stack_pos_++].Assign(obj);
  }

  virtual void Finalize() {
    delete this;
  }

  // Scans all of the objects
  virtual void Run(Thread* self ATTRIBUTE_UNUSED)
      REQUIRES(Locks::heap_bitmap_lock_)
      SHARED_REQUIRES(Locks::mutator_lock_) {
    ScanObjectParallelVisitor visitor(this);
    // TODO: Tune this.
    static const size_t kFifoSize = 4;
    BoundedFifoPowerOfTwo<mirror::Object*, kFifoSize> prefetch_fifo;
    for (;;) {
      mirror::Object* obj = nullptr;
      if (kUseMarkStackPrefetch) {
        while (mark_stack_pos_ != 0 && prefetch_fifo.size() < kFifoSize) {
          mirror::Object* const mark_stack_obj = mark_stack_[--mark_stack_pos_].AsMirrorPtr();
          DCHECK(mark_stack_obj != nullptr);
          __builtin_prefetch(mark_stack_obj);
          prefetch_fifo.push_back(mark_stack_obj);
        }
        if (UNLIKELY(prefetch_fifo.empty())) {
          break;
        }
        obj = prefetch_fifo.front();
        prefetch_fifo.pop_front();
      } else {
        if (UNLIKELY(mark_stack_pos_ == 0)) {
          break;
        }
        obj = mark_stack_[--mark_stack_pos_].AsMirrorPtr();
      }
      DCHECK(obj != nullptr);
      visitor(obj);
    }
  }
};

class CardScanTask : public MarkStackTask<false> {
 public:
  CardScanTask(ThreadPool* thread_pool,
               MarkSweep* mark_sweep,
               accounting::ContinuousSpaceBitmap* bitmap,
               uint8_t* begin,
               uint8_t* end,
               uint8_t minimum_age,
               size_t mark_stack_size,
               StackReference<mirror::Object>* mark_stack_obj,
               bool clear_card)
      : MarkStackTask<false>(thread_pool, mark_sweep, mark_stack_size, mark_stack_obj),
        bitmap_(bitmap),
        begin_(begin),
        end_(end),
        minimum_age_(minimum_age),
        clear_card_(clear_card) {}

 protected:
  accounting::ContinuousSpaceBitmap* const bitmap_;
  uint8_t* const begin_;
  uint8_t* const end_;
  const uint8_t minimum_age_;
  const bool clear_card_;

  virtual void Finalize() {
    delete this;
  }

  virtual void Run(Thread* self) NO_THREAD_SAFETY_ANALYSIS {
    ScanObjectParallelVisitor visitor(this);
    accounting::CardTable* card_table = mark_sweep_->GetHeap()->GetCardTable();
    size_t cards_scanned = clear_card_
        ? card_table->Scan<true>(bitmap_, begin_, end_, visitor, minimum_age_)
        : card_table->Scan<false>(bitmap_, begin_, end_, visitor, minimum_age_);
    VLOG(heap) << "Parallel scanning cards " << reinterpret_cast<void*>(begin_) << " - "
        << reinterpret_cast<void*>(end_) << " = " << cards_scanned;
    // Finish by emptying our local mark stack.
    MarkStackTask::Run(self);
  }
};

void MarkSweep::ScanGrayObjects(bool paused, uint8_t minimum_age) {
  accounting::CardTable* card_table = GetHeap()->GetCardTable();
  ThreadPool* thread_pool = GetHeap()->GetThreadPool();
  size_t thread_count = GetHeap()->GetThreadCount(paused);
  // The bump pointer space doesn't have mark bitmap. Use objects_before_forwarding_ instead.
  accounting::ContinuousSpaceBitmap* mark_bitmap = nullptr;
  // The parallel version with only one thread is faster for card scanning, TODO: fix.
  if (kParallelCardScan && thread_count > 1) {
    Thread* self = Thread::Current();
    // Can't have a different split for each space since multiple spaces can have their cards being
    // scanned at the same time.
    TimingLogger::ScopedTiming t(paused ? "(Paused)ScanGrayObjects" : __FUNCTION__,
        GetTimings());
    // Try to take some of the mark stack since we can pass this off to the worker tasks.
    StackReference<mirror::Object>* mark_stack_begin = mark_stack_->Begin();
    StackReference<mirror::Object>* mark_stack_end = mark_stack_->End();
    const size_t mark_stack_size = mark_stack_end - mark_stack_begin;
    // Estimated number of work tasks we will create.
    const size_t mark_stack_tasks = GetHeap()->GetContinuousSpaces().size() * thread_count;
    DCHECK_NE(mark_stack_tasks, 0U);
    const size_t mark_stack_delta = std::min(CardScanTask::kMaxSize / 2,
                                             mark_stack_size / mark_stack_tasks + 1);
    for (const auto& space : GetHeap()->GetContinuousSpaces()) {
      // The dirty cards of below spaces are changed to dirty -1 during ProcessCards in Pause phase.
      // TODO: No change the dirty cards to dirty - 1 in Pause phase to avoid CAS.
      if (paused && (space->IsZygoteSpace() || space->IsImageSpace() || space->IsRosAllocSpace())) {
        minimum_age = accounting::CardTable::kCardDirty - 1;
      }
      if (space->GetMarkBitmap() == nullptr && space == from_bps_) {
          mark_bitmap = objects_before_forwarding_.get();
      } else if (space->GetMarkBitmap() != nullptr) {
          mark_bitmap = space->GetMarkBitmap();
      } else {
        continue;
      }
      uint8_t* card_begin = space->Begin();
      uint8_t* card_end = space->End();
      // Align up the end address. For example, the image space's end
      // may not be card-size-aligned.
      card_end = AlignUp(card_end, accounting::CardTable::kCardSize);
      DCHECK_ALIGNED(card_begin, accounting::CardTable::kCardSize);
      DCHECK_ALIGNED(card_end, accounting::CardTable::kCardSize);
      // Calculate how many bytes of heap we will scan,
      const size_t address_range = card_end - card_begin;
      // Calculate how much address range each task gets.
      const size_t card_delta = RoundUp(address_range / thread_count + 1,
                                        accounting::CardTable::kCardSize);
      // If paused and the space is neither zygote nor image space, we could clear the dirty
      // cards to avoid accumulating them to increase card scanning load in the following GC
      // cycles. We need to keep dirty cards of image space and zygote space in order to track
      // references to the other spaces.
      bool clear_card = paused && !space->IsZygoteSpace() && !space->IsImageSpace();
      // Create the worker tasks for this space.
      while (card_begin != card_end) {
        // Add a range of cards.
        size_t addr_remaining = card_end - card_begin;
        size_t card_increment = std::min(card_delta, addr_remaining);
        // Take from the back of the mark stack.
        size_t mark_stack_remaining = mark_stack_end - mark_stack_begin;
        size_t mark_stack_increment = std::min(mark_stack_delta, mark_stack_remaining);
        mark_stack_end -= mark_stack_increment;
        mark_stack_->PopBackCount(static_cast<int32_t>(mark_stack_increment));
        DCHECK_EQ(mark_stack_end, mark_stack_->End());
        // Add the new task to the thread pool.
        auto* task = new CardScanTask(thread_pool,
                                      this,
                                      mark_bitmap,
                                      card_begin,
                                      card_begin + card_increment,
                                      minimum_age,
                                      mark_stack_increment,
                                      mark_stack_end,
                                      clear_card);
        thread_pool->AddTask(self, task);
        card_begin += card_increment;
      }
    }

    // Note: the card scan below may dirty new cards (and scan them)
    // as a side effect when a Reference object is encountered and
    // queued during the marking. See b/11465268.
    thread_pool->SetMaxActiveWorkers(thread_count - 1);
    thread_pool->StartWorkers(self);
    thread_pool->Wait(self, true, true);
    thread_pool->StopWorkers(self);
  } else {
    for (const auto& space : GetHeap()->GetContinuousSpaces()) {
      // The dirty cards of below spaces are changed to dirty -1 during ProcessCards in Pause phase.
      // TODO: No change the dirty cards to dirty - 1 in Pause phase to avoid CAS.
      if (paused && (space->IsZygoteSpace() || space->IsImageSpace() || space->IsRosAllocSpace())) {
        minimum_age = accounting::CardTable::kCardDirty - 1;
      }
      if (space->GetMarkBitmap() == nullptr && space == from_bps_) {
          mark_bitmap = objects_before_forwarding_.get();
      } else if (space->GetMarkBitmap() != nullptr) {
          mark_bitmap = space->GetMarkBitmap();
      } else {
        continue;
      }
      // Image spaces are handled properly since live == marked for them.
      const char* name = nullptr;
      switch (space->GetGcRetentionPolicy()) {
      case space::kGcRetentionPolicyNeverCollect:
        name = paused ? "(Paused)ScanGrayImageSpaceObjects" : "ScanGrayImageSpaceObjects";
        break;
      case space::kGcRetentionPolicyFullCollect:
        name = paused ? "(Paused)ScanGrayZygoteSpaceObjects" : "ScanGrayZygoteSpaceObjects";
        break;
      case space::kGcRetentionPolicyAlwaysCollect:
        name = paused ? "(Paused)ScanGrayAllocSpaceObjects" : "ScanGrayAllocSpaceObjects";
        break;
      default:
        LOG(FATAL) << "Unreachable";
        UNREACHABLE();
      }
      TimingLogger::ScopedTiming t(name, GetTimings());
      ScanObjectVisitor visitor(this);
      bool clear_card = paused && !space->IsZygoteSpace() && !space->IsImageSpace();
      if (clear_card) {
        card_table->Scan<true>(mark_bitmap,
                               space->Begin(),
                               space->End(),
                               visitor,
                               minimum_age);
      } else {
        card_table->Scan<false>(mark_bitmap,
                                space->Begin(),
                                space->End(),
                                visitor,
                                minimum_age);
      }
    }
  }
}

class RecursiveMarkTask : public MarkStackTask<false> {
 public:
  RecursiveMarkTask(ThreadPool* thread_pool,
                    MarkSweep* mark_sweep,
                    accounting::ContinuousSpaceBitmap* bitmap,
                    uintptr_t begin,
                    uintptr_t end)
      : MarkStackTask<false>(thread_pool, mark_sweep, 0, nullptr),
        bitmap_(bitmap),
        begin_(begin),
        end_(end) {}

 protected:
  accounting::ContinuousSpaceBitmap* const bitmap_;
  const uintptr_t begin_;
  const uintptr_t end_;

  virtual void Finalize() {
    delete this;
  }

  // Scans all of the objects
  virtual void Run(Thread* self) NO_THREAD_SAFETY_ANALYSIS {
    ScanObjectParallelVisitor visitor(this);
    bitmap_->VisitMarkedRange(begin_, end_, visitor);
    // Finish by emptying our local mark stack.
    MarkStackTask::Run(self);
  }
};

// Populates the mark stack based on the set of marked objects and
// recursively marks until the mark stack is emptied.
void MarkSweep::RecursiveMark() {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  // RecursiveMark will build the lists of known instances of the Reference classes. See
  // DelayReferenceReferent for details.
  if (kUseRecursiveMark) {
    const bool partial = GetGcType() == kGcTypePartial;
    ScanObjectVisitor scan_visitor(this);
    auto* self = Thread::Current();
    ThreadPool* thread_pool = heap_->GetThreadPool();
    size_t thread_count = heap_->GetThreadCount(false);
    const bool parallel = kParallelRecursiveMark && thread_count > 1;
    mark_stack_->Reset();
    for (const auto& space : GetHeap()->GetContinuousSpaces()) {
      if ((space->GetGcRetentionPolicy() == space::kGcRetentionPolicyAlwaysCollect) ||
          (!partial && space->GetGcRetentionPolicy() == space::kGcRetentionPolicyFullCollect)) {
        current_space_bitmap_ = space->GetMarkBitmap();
        if (current_space_bitmap_ == nullptr && space->IsBumpPointerSpace()) {
          current_space_bitmap_ = objects_before_forwarding_.get();
        } else {
          continue;
        }
        if (parallel) {
          // We will use the mark stack the future.
          // CHECK(mark_stack_->IsEmpty());
          // This function does not handle heap end increasing, so we must use the space end.
          uintptr_t begin = reinterpret_cast<uintptr_t>(space->Begin());
          uintptr_t end = reinterpret_cast<uintptr_t>(space->End());
          atomic_finger_.StoreRelaxed(AtomicInteger::MaxValue());

          // Create a few worker tasks.
          const size_t n = thread_count * 2;
          while (begin != end) {
            uintptr_t start = begin;
            uintptr_t delta = (end - begin) / n;
            delta = RoundUp(delta, KB);
            if (delta < 16 * KB) delta = end - begin;
            begin += delta;
            auto* task = new RecursiveMarkTask(thread_pool,
                                               this,
                                               current_space_bitmap_,
                                               start,
                                               begin);
            thread_pool->AddTask(self, task);
          }
          thread_pool->SetMaxActiveWorkers(thread_count - 1);
          thread_pool->StartWorkers(self);
          thread_pool->Wait(self, true, true);
          thread_pool->StopWorkers(self);
        } else {
          // This function does not handle heap end increasing, so we must use the space end.
          uintptr_t begin = reinterpret_cast<uintptr_t>(space->Begin());
          uintptr_t end = reinterpret_cast<uintptr_t>(space->End());
          current_space_bitmap_->VisitMarkedRange(begin, end, scan_visitor);
        }
      }
    }
  }
  ProcessMarkStack(false);
}

void MarkSweep::RecursiveMarkDirtyObjects(bool paused, uint8_t minimum_age) {
  ScanGrayObjects(paused, minimum_age);
  ProcessMarkStack(paused);
}

void MarkSweep::ReMarkRoots() {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  Locks::mutator_lock_->AssertExclusiveHeld(Thread::Current());
  Runtime::Current()->VisitRoots(this, static_cast<VisitRootFlags>(
      kVisitRootFlagNewRoots | kVisitRootFlagStopLoggingNewRoots | kVisitRootFlagClearRootLog));
  if (kVerifyRootsMarked) {
    TimingLogger::ScopedTiming t2("(Paused)VerifyRoots", GetTimings());
    VerifyRootMarkedVisitor visitor(this);
    Runtime::Current()->VisitRoots(&visitor);
  }
}

void MarkSweep::SweepSystemWeaks(Thread* self) {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  ReaderMutexLock mu(self, *Locks::heap_bitmap_lock_);
  Runtime::Current()->SweepSystemWeaks(this);
}

class VerifySystemWeakVisitor : public IsMarkedVisitor {
 public:
  explicit VerifySystemWeakVisitor(MarkSweep* mark_sweep) : mark_sweep_(mark_sweep) {}

  virtual mirror::Object* IsMarked(mirror::Object* obj)
      OVERRIDE
      SHARED_REQUIRES(Locks::mutator_lock_, Locks::heap_bitmap_lock_) {
    mark_sweep_->VerifyIsLive(obj);
    return obj;
  }

  MarkSweep* const mark_sweep_;
};

void MarkSweep::VerifyIsLive(const mirror::Object* obj) {
  if (!heap_->GetLiveBitmap()->Test(obj)) {
    // TODO: Consider live stack? Has this code bitrotted?
    CHECK(!heap_->allocation_stack_->Contains(obj))
        << "Found dead object " << obj << "\n" << heap_->DumpSpaces();
  }
}

void MarkSweep::VerifySystemWeaks() {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  // Verify system weaks, uses a special object visitor which returns the input object.
  VerifySystemWeakVisitor visitor(this);
  Runtime::Current()->SweepSystemWeaks(&visitor);
}

class CheckpointMarkThreadRoots : public Closure, public RootVisitor {
 public:
  CheckpointMarkThreadRoots(MarkSweep* mark_sweep,
                            bool mark_roots,
                            bool revoke_ros_alloc_thread_local_buffers_at_checkpoint)
      : mark_sweep_(mark_sweep), mark_roots_(mark_roots),
        revoke_ros_alloc_thread_local_buffers_at_checkpoint_(
            revoke_ros_alloc_thread_local_buffers_at_checkpoint) {
  }

  void VisitRoots(mirror::Object*** roots, size_t count, const RootInfo& info ATTRIBUTE_UNUSED)
      OVERRIDE SHARED_REQUIRES(Locks::mutator_lock_)
      REQUIRES(Locks::heap_bitmap_lock_) {
    for (size_t i = 0; i < count; ++i) {
      mark_sweep_->MarkObjectNonNullParallel(*roots[i]);
    }
  }

  void VisitRoots(mirror::CompressedReference<mirror::Object>** roots,
                  size_t count,
                  const RootInfo& info ATTRIBUTE_UNUSED)
      OVERRIDE SHARED_REQUIRES(Locks::mutator_lock_)
      REQUIRES(Locks::heap_bitmap_lock_) {
    for (size_t i = 0; i < count; ++i) {
      mark_sweep_->MarkObjectNonNullParallel(roots[i]->AsMirrorPtr());
    }
  }

  virtual void Run(Thread* thread) OVERRIDE NO_THREAD_SAFETY_ANALYSIS {
    ScopedTrace trace("Marking thread roots");
    // Note: self is not necessarily equal to thread since thread may be suspended.
    Thread* const self = Thread::Current();
    CHECK(thread == self || thread->IsSuspended() || thread->GetState() == kWaitingPerformingGc)
        << thread->GetState() << " thread " << thread << " self " << self;
    if (mark_roots_) {
      thread->VisitRoots(this);
    }
    if (revoke_ros_alloc_thread_local_buffers_at_checkpoint_) {
      ScopedTrace trace2("RevokeRosAllocThreadLocalBuffers");
      mark_sweep_->GetHeap()->RevokeRosAllocThreadLocalBuffers(thread);
    }
    // If thread is a running mutator, then act on behalf of the garbage collector.
    // See the code in ThreadList::RunCheckpoint.
    mark_sweep_->GetBarrier().Pass(self);
  }

 private:
  MarkSweep* const mark_sweep_;
  const bool mark_roots_;
  const bool revoke_ros_alloc_thread_local_buffers_at_checkpoint_;
};

void MarkSweep::MarkRootsCheckpoint(Thread* self,
                                    bool revoke_ros_alloc_thread_local_buffers_at_checkpoint,
                                    bool mark_roots) {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  CheckpointMarkThreadRoots check_point(this, mark_roots,
                                        revoke_ros_alloc_thread_local_buffers_at_checkpoint);
  ThreadList* thread_list = Runtime::Current()->GetThreadList();
  // Request the check point is run on all threads returning a count of the threads that must
  // run through the barrier including self.
  size_t barrier_count = thread_list->RunCheckpoint(&check_point);
  // Release locks then wait for all mutator threads to pass the barrier.
  // If there are no threads to wait which implys that all the checkpoint functions are finished,
  // then no need to release locks.
  if (barrier_count == 0) {
    return;
  }
  Locks::heap_bitmap_lock_->ExclusiveUnlock(self);
  Locks::mutator_lock_->SharedUnlock(self);
  {
    ScopedThreadStateChange tsc(self, kWaitingForCheckPointsToRun);
    gc_barrier_->Increment(self, barrier_count);
  }
  Locks::mutator_lock_->SharedLock(self);
  Locks::heap_bitmap_lock_->ExclusiveLock(self);
}

void MarkSweep::SweepArray(accounting::ObjectStack* allocations, bool swap_bitmaps) {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  Thread* self = Thread::Current();
  mirror::Object** chunk_free_buffer = reinterpret_cast<mirror::Object**>(
      sweep_array_free_buffer_mem_map_->BaseBegin());
  size_t chunk_free_pos = 0;
  ObjectBytePair freed;
  ObjectBytePair freed_los;
  // How many objects are left in the array, modified after each space is swept.
  StackReference<mirror::Object>* objects = allocations->Begin();
  size_t count = allocations->Size();
  // Change the order to ensure that the non-moving space last swept as an optimization.
  std::vector<space::ContinuousSpace*> sweep_spaces;
  space::ContinuousSpace* non_moving_space = nullptr;
  for (space::ContinuousSpace* space : heap_->GetContinuousSpaces()) {
    if (space->IsAllocSpace() &&
        !immune_spaces_.ContainsSpace(space) &&
        space->GetLiveBitmap() != nullptr) {
      if (space == heap_->GetNonMovingSpace()) {
        non_moving_space = space;
      } else {
        sweep_spaces.push_back(space);
      }
    }
  }
  // Unlikely to sweep a significant amount of non_movable objects, so we do these after the after
  // the other alloc spaces as an optimization.
  if (non_moving_space != nullptr) {
    sweep_spaces.push_back(non_moving_space);
  }
  // Start by sweeping the continuous spaces.
  for (space::ContinuousSpace* space : sweep_spaces) {
    space::AllocSpace* alloc_space = space->AsAllocSpace();
    accounting::ContinuousSpaceBitmap* live_bitmap = space->GetLiveBitmap();
    accounting::ContinuousSpaceBitmap* mark_bitmap = space->GetMarkBitmap();
    if (swap_bitmaps) {
      std::swap(live_bitmap, mark_bitmap);
    }
    StackReference<mirror::Object>* out = objects;
    for (size_t i = 0; i < count; ++i) {
      mirror::Object* const obj = objects[i].AsMirrorPtr();
      if (kUseThreadLocalAllocationStack && obj == nullptr) {
        continue;
      }
      if (space->HasAddress(obj)) {
        // This object is in the space, remove it from the array and add it to the sweep buffer
        // if needed.
        if (!mark_bitmap->Test(obj)) {
          if (chunk_free_pos >= kSweepArrayChunkFreeSize) {
            TimingLogger::ScopedTiming t2("FreeList", GetTimings());
            freed.objects += chunk_free_pos;
            freed.bytes += alloc_space->FreeList(self, chunk_free_pos, chunk_free_buffer);
            chunk_free_pos = 0;
          }
          chunk_free_buffer[chunk_free_pos++] = obj;
        }
      } else {
        (out++)->Assign(obj);
      }
    }
    if (chunk_free_pos > 0) {
      TimingLogger::ScopedTiming t2("FreeList", GetTimings());
      freed.objects += chunk_free_pos;
      freed.bytes += alloc_space->FreeList(self, chunk_free_pos, chunk_free_buffer);
      chunk_free_pos = 0;
    }
    // All of the references which space contained are no longer in the allocation stack, update
    // the count.
    count = out - objects;
  }
  // Handle the large object space.
  space::LargeObjectSpace* large_object_space = GetHeap()->GetLargeObjectsSpace();
  if (large_object_space != nullptr) {
    accounting::LargeObjectBitmap* large_live_objects = large_object_space->GetLiveBitmap();
    accounting::LargeObjectBitmap* large_mark_objects = large_object_space->GetMarkBitmap();
    if (swap_bitmaps) {
      std::swap(large_live_objects, large_mark_objects);
    }
    for (size_t i = 0; i < count; ++i) {
      mirror::Object* const obj = objects[i].AsMirrorPtr();
      // Handle large objects.
      if (kUseThreadLocalAllocationStack && obj == nullptr) {
        continue;
      }
      if (!large_mark_objects->Test(obj)) {
        ++freed_los.objects;
        freed_los.bytes += large_object_space->Free(self, obj);
      }
    }
  }
  {
    TimingLogger::ScopedTiming t2("RecordFree", GetTimings());
    RecordFree(freed);
    RecordFreeLOS(freed_los);
    t2.NewTiming("ResetStack");
    allocations->Reset();
  }
  sweep_array_free_buffer_mem_map_->MadviseDontNeedAndZero();
}

void MarkSweep::Sweep(bool swap_bitmaps) {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  // Ensure that nobody inserted items in the live stack after we swapped the stacks.
  CHECK_GE(live_stack_freeze_size_, GetHeap()->GetLiveStack()->Size());
  {
    TimingLogger::ScopedTiming t2("MarkAllocStackAsLive", GetTimings());
    // Mark everything allocated since the last as GC live so that we can sweep concurrently,
    // knowing that new allocations won't be marked as live.
    accounting::ObjectStack* live_stack = heap_->GetLiveStack();
    heap_->MarkAllocStackAsLive(live_stack);
    live_stack->Reset();
    DCHECK(mark_stack_->IsEmpty());
  }
  for (const auto& space : GetHeap()->GetContinuousSpaces()) {
    // The free of bps is done separately.
    if (space->IsContinuousMemMapAllocSpace() && !space->IsBumpPointerSpace()) {
      space::ContinuousMemMapAllocSpace* alloc_space = space->AsContinuousMemMapAllocSpace();
      TimingLogger::ScopedTiming split(
          alloc_space->IsZygoteSpace() ? "SweepZygoteSpace" : "SweepMallocSpace",
          GetTimings());
      if (space->IsRosAllocSpace()) {
        RecordFree(heap_->GetRosAllocSpace()->GetRosAlloc()->SweepWalkPagemap(swap_bitmaps));
      } else {
        RecordFree(alloc_space->Sweep(swap_bitmaps));
      }
    }
  }
  SweepLargeObjects(swap_bitmaps);
}

void MarkSweep::SweepLargeObjects(bool swap_bitmaps) {
  space::LargeObjectSpace* los = heap_->GetLargeObjectsSpace();
  if (los != nullptr) {
    TimingLogger::ScopedTiming split(__FUNCTION__, GetTimings());
    RecordFreeLOS(los->Sweep(swap_bitmaps));
  }
}

// Process the "referent" field in a java.lang.ref.Reference.  If the referent has not yet been
// marked, put it on the appropriate list in the heap for later processing.
void MarkSweep::DelayReferenceReferent(mirror::Class* klass, mirror::Reference* ref) {
  heap_->GetReferenceProcessor()->DelayReferenceReferent(klass, ref, this);
}

class MarkVisitor {
 public:
  ALWAYS_INLINE explicit MarkVisitor(MarkSweep* const mark_sweep) : mark_sweep_(mark_sweep) {}

  ALWAYS_INLINE void operator()(mirror::Object* obj,
                                MemberOffset offset,
                                bool is_static ATTRIBUTE_UNUSED) const
      REQUIRES(Locks::heap_bitmap_lock_)
      SHARED_REQUIRES(Locks::mutator_lock_) {
    if (kCheckLocks) {
      Locks::mutator_lock_->AssertSharedHeld(Thread::Current());
      Locks::heap_bitmap_lock_->AssertExclusiveHeld(Thread::Current());
    }
    mark_sweep_->MarkObject(obj->GetFieldObject<mirror::Object>(offset), obj, offset);
  }

  void VisitRootIfNonNull(mirror::CompressedReference<mirror::Object>* root) const
      REQUIRES(Locks::heap_bitmap_lock_)
      SHARED_REQUIRES(Locks::mutator_lock_) {
    if (!root->IsNull()) {
      VisitRoot(root);
    }
  }

  void VisitRoot(mirror::CompressedReference<mirror::Object>* root) const
      REQUIRES(Locks::heap_bitmap_lock_)
      SHARED_REQUIRES(Locks::mutator_lock_) {
    if (kCheckLocks) {
      Locks::mutator_lock_->AssertSharedHeld(Thread::Current());
      Locks::heap_bitmap_lock_->AssertExclusiveHeld(Thread::Current());
    }
    mark_sweep_->MarkObject(root->AsMirrorPtr());
  }

 private:
  MarkSweep* const mark_sweep_;
};

// Scans an object reference.  Determines the type of the reference
// and dispatches to a specialized scanning routine.
void MarkSweep::ScanObject(mirror::Object* obj) {
  MarkVisitor mark_visitor(this);
  DelayReferenceReferentVisitor ref_visitor(this);
  ScanObjectVisit(obj, mark_visitor, ref_visitor);
}

void MarkSweep::ProcessMarkStackParallel(size_t thread_count) {
  Thread* self = Thread::Current();
  ThreadPool* thread_pool = GetHeap()->GetThreadPool();
  const size_t chunk_size = std::min(mark_stack_->Size() / thread_count + 1,
                                     static_cast<size_t>(MarkStackTask<false>::kMaxSize));
  CHECK_GT(chunk_size, 0U);
  // Split the current mark stack up into work tasks.
  for (auto* it = mark_stack_->Begin(), *end = mark_stack_->End(); it < end; ) {
    const size_t delta = std::min(static_cast<size_t>(end - it), chunk_size);
    thread_pool->AddTask(self, new MarkStackTask<false>(thread_pool, this, delta, it));
    it += delta;
  }
  thread_pool->SetMaxActiveWorkers(thread_count - 1);
  thread_pool->StartWorkers(self);
  thread_pool->Wait(self, true, true);
  thread_pool->StopWorkers(self);
  mark_stack_->Reset();
  CHECK_EQ(work_chunks_created_.LoadSequentiallyConsistent(),
           work_chunks_deleted_.LoadSequentiallyConsistent())
      << " some of the work chunks were leaked";
}

// Scan anything that's on the mark stack.
void MarkSweep::ProcessMarkStack(bool paused) {
  TimingLogger::ScopedTiming t(paused ? "(Paused)ProcessMarkStack" : __FUNCTION__, GetTimings());
  size_t thread_count = GetHeap()->GetThreadCount(paused);
  if (kParallelProcessMarkStack && thread_count > 1 &&
      mark_stack_->Size() >= kMinimumParallelMarkStackSize) {
    ProcessMarkStackParallel(thread_count);
  } else {
    // TODO: Tune this.
    static const size_t kFifoSize = 4;
    BoundedFifoPowerOfTwo<mirror::Object*, kFifoSize> prefetch_fifo;
    for (;;) {
      mirror::Object* obj = nullptr;
      if (kUseMarkStackPrefetch) {
        while (!mark_stack_->IsEmpty() && prefetch_fifo.size() < kFifoSize) {
          mirror::Object* mark_stack_obj = mark_stack_->PopBack();
          DCHECK(mark_stack_obj != nullptr);
          __builtin_prefetch(mark_stack_obj);
          prefetch_fifo.push_back(mark_stack_obj);
        }
        if (prefetch_fifo.empty()) {
          break;
        }
        obj = prefetch_fifo.front();
        prefetch_fifo.pop_front();
      } else {
        if (mark_stack_->IsEmpty()) {
          break;
        }
        obj = mark_stack_->PopBack();
      }
      DCHECK(obj != nullptr);
      ScanObject(obj);
    }
  }
}

inline mirror::Object* MarkSweep::IsMarked(mirror::Object* object) {
  if (immune_spaces_.IsInImmuneRegion(object)) {
    return object;
  }
  if (current_space_bitmap_->HasAddress(object)) {
    if (!is_copying_) {
      return current_space_bitmap_->Test(object) ? object : nullptr;
    } else {
      mirror::Object* obj = current_space_bitmap_->Test(object) ? object : nullptr;
      if (obj == nullptr) {
        return nullptr;
      }
      mirror::Object* new_obj = GetMarkedForwardAddress(const_cast<mirror::Object*>(obj));
      return new_obj == nullptr ? obj : new_obj;
    }
  }
  return mark_bitmap_->Test(object) ? object : nullptr;
}

void MarkSweep::FinishPhase() {
  TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
  if (kCountScannedTypes) {
    VLOG(gc)
        << "MarkSweep scanned"
        << " no reference objects=" << no_reference_class_count_.LoadRelaxed()
        << " normal objects=" << normal_count_.LoadRelaxed()
        << " classes=" << class_count_.LoadRelaxed()
        << " object arrays=" << object_array_count_.LoadRelaxed()
        << " references=" << reference_count_.LoadRelaxed()
        << " other=" << other_count_.LoadRelaxed();
  }
  if (kCountTasks) {
    VLOG(gc) << "Total number of work chunks allocated: " << work_chunks_created_.LoadRelaxed();
  }
  if (kMeasureOverhead) {
    VLOG(gc) << "Overhead time " << PrettyDuration(overhead_time_.LoadRelaxed());
  }
  if (kProfileLargeObjects) {
    VLOG(gc) << "Large objects tested " << large_object_test_.LoadRelaxed()
        << " marked " << large_object_mark_.LoadRelaxed();
  }
  if (kCountMarkedObjects) {
    VLOG(gc) << "Marked: null=" << mark_null_count_.LoadRelaxed()
        << " immune=" <<  mark_immune_count_.LoadRelaxed()
        << " fastpath=" << mark_fastpath_count_.LoadRelaxed()
        << " slowpath=" << mark_slowpath_count_.LoadRelaxed();
  }
  CHECK(mark_stack_->IsEmpty());  // Ensure that the mark stack is empty.
  mark_stack_->Reset();
  Thread* const self = Thread::Current();
  ReaderMutexLock mu(self, *Locks::mutator_lock_);
  WriterMutexLock mu2(self, *Locks::heap_bitmap_lock_);
  heap_->ClearMarkedObjects();
  if (IsCopying()) {
    objects_before_forwarding_.get()->Clear();
    objects_before_forwarding_.reset(nullptr);
    objects_after_forwarding_.get()->Clear();
    objects_after_forwarding_.reset(nullptr);
    copy_candidate_stack_->Reset();
    copy_candidate_stack_.reset(nullptr);
    to_bps_ = nullptr;
    from_bps_ = nullptr;
  }
}

void MarkSweep::RevokeAllThreadLocalBuffers() {
  if (kRevokeRosAllocThreadLocalBuffersAtCheckpoint && (IsConcurrent() && !IsCopying())) {
    // If concurrent, rosalloc thread-local buffers are revoked at the
    // thread checkpoint. Bump pointer space thread-local buffers must
    // not be in use.
    // For GenCopying, BP space may be used.
    GetHeap()->AssertAllBumpPointerSpaceThreadLocalBuffersAreRevoked();
  } else {
    TimingLogger::ScopedTiming t(__FUNCTION__, GetTimings());
    GetHeap()->RevokeAllThreadLocalBuffers();
  }
}

}  // namespace collector
}  // namespace gc
}  // namespace art
