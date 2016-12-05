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
 */

#include "manipulator.h"
#include "gcview_space.h"
#include "gc/heap-inl.h"
#include "thread_list.h"
#include <string>
namespace art {
namespace gc {

// GCview data names for summary space
static const char* kLevelValueName = "Level";

static const char* kHeapCapacity = "Heap Capacity";
static const char* kHeapFootprint = "Heap Footprint";
static const char* kHeapAllocatedSize = "Heap Allocated Size";
static const char* kHeapAllocatedObjectCount = "Heap Allocated Object Count";
static const char* kHeapSummaryGroup = "Heap Summary";

static const char* kGCCountArray = "GC Count";
static const char* kGCTypeArray = "GC Type";
static const char* kGCSummaryGroup = "GC Summary";

// GCview data names for common space
static const char* kSpaceCapacity = "Space Capacity";
static const char* kSpaceFootprint = "Space Footprint";
static const char* kSpaceAllocatedSize = "Space Allocated Size";
static const char* kSpaceAllocatedObjectCount = "Space Allocated Object Count";

// GCview data names for ros space
static const char* kContinuousPagesKindArray = "Continous Pages Kind";
static const char* kContinuousPageKindEmptyName = "Empty Kind";
static const char* kContinuousPageKindRelaseName = "Release kind";
static const char* kContinuousPageKindLargeObjectName = "Large Object Kind";
static const char* kContinuousPageKindRunName = "Run Kind";

static const char* kContinuousPagesKindMapArray = "Continous Pages Kind Map";
static const int kContinuousPagesKindEmptyOrReleaseMap = 0;
static const int kContinuousPagesKindLargeObjectMap = 1;
static const int kContinuousPagesKindRunMap = 2;

static const char* kContinuousPagesKindEmptyCount = "Count of Empty Kind";
static const char* kContinuousPagesKindReleaseCount = "Count of Release Kind";
static const char* kContinuousPagesKindEmptyOrReleaseCount = "Count of Empty/Release Kind";
static const char* kContinuousPagesKindLargeObjectCount = "Count of Large Object kind";
static const char* kContinuousPagesKindRunCount = "Count of Run Kind";
static const char* kRunDistName = "Run Distribution";
static const char* kRunDistSize = "Bracket Size";
static const char* kRunDistPopulation = "Population Of The Same Bracket Size";
static const char* kRunDistTotalAllocatedBytes = "Total Allocated Bytes Of The Same Bracket Size";

static const char* kContinuousPagesKindEmptyOrReleaseInfoGroup = "Empty/Release Kind";
static const char* kContinuousPagesKindEmptyOrReleaseSizeArray = "Empty/Release Pages Size";
static const char* kContinuousPagesKindEmptyOrReleaseRangeArray = "Empty/Release Pages Address Range";

static const char* kContinuousPagesKindLargeObjectInfoGroup = "Large Object Kind";
static const char* kContinuousPagesKindLargeObjectSizeArray = "Large Object Size";
static const char* kContinuousPagesKindLargeObjectRangeArray = "Large Object Address Range";

static const char* kContinousPagesKindRunInfoGroup = "Run Kind";
static const char* kContinousPagesKindRunSizeArray = "Run Size";
static const char* kContinousPagesKindRunRangeArray = "Run Address Range";
static const char* kContinousPagesKindRunAllocatedSizeArray = "Run Allocated Size";
static const char* kContinuosPagesKindRunAllocatedObjectCountArray = "Run Allocated Object Count";
static const char* kContinuosPagesKindRunBracketSizeArray = "Run Bracket Size";
static const char* kContinuosPagesKindRunTotalSlotsNumArray = "Run total slots Num";
static const char* kContinuosPagesKindRunIsLocalArray = "Run is Local";
static const char* kContinuosPagesKindRunIsBulkFreedArray = "Run is to be Bulk Freed";
static const char* kContinuosPagesKindRunIsFullArray = "Run is Full";

// GCview data names for large object space
static const char* kLOSBytesAllocated = "Bytes Allocated";
static const char* kLOSObjectsAllocated = "Objects Allocated";
static const char* kLOSTotalBytesAllocated = "Total Bytes Allocated";
static const char* kLOSTotalObjectsAllocated = "Total Object Allocated";
static const char* kLOSObjectAddressRangeArray = "Object Address Range";
static const char* kLOSObjectLengthArray = "Object Length"; 
static const char* kLOSObjectInfoGroup = "Large Object Info";
static const char* kLOSMemMapRangeArray = "MemMap Range";
static const char* kLOSMemMapSizeArray = "MemMap Size";
static const char* kLOSMemMapIsMemMapArray = "Is MemMap";
static const char* kLOSMemMapInfoGroup = "MemMap Info";

Manipulator::Manipulator(const std::string& name, gcview::GCviewSpace* space, int level)
    : name_(name), connected_space_(space), level_(level) { }

SummaryManipulator* SummaryManipulator::Create(const std::string& name, gcview::GCviewSpace* space, int level) {
  return new SummaryManipulator(name, space, level);
}

void SummaryManipulator::InitSpaceData() {
  level_value_ = connected_space_->addData<StringV>(kLevelValueName);
  if (IsLevelHigh()) {
    level_value_->value() = "High";
  } else if (IsLevelMedium()) {
    level_value_->value() = "Medium";
  } else {
    level_value_->value() = "Low";
  } 

  gc_type_array_ = connected_space_->addData<StringA>(kGCTypeArray, kGCSummaryGroup);
  gc_count_array_ = connected_space_->addData<IntA>(kGCCountArray, kGCSummaryGroup);

  gc_type_array_->resize(collector::kGcTypeMax - 1);
  gc_type_array_->value(collector::kGcTypeSticky - 1) = "Sticky GC";
  gc_type_array_->value(collector::kGcTypePartial -1) = "Partial GC";
  gc_type_array_->value(collector::kGcTypeFull -1) = "Full GC";
  gc_count_array_->resize(collector::kGcTypeMax - 1);
  gc_count_array_->value(collector::kGcTypeSticky - 1) = 0;
  gc_count_array_->value(collector::kGcTypePartial -1) = 0;
  gc_count_array_->value(collector::kGcTypeFull -1) = 0;

  heap_capacity_ = connected_space_->addData<IntV>(kHeapCapacity, kHeapSummaryGroup);
  heap_footprint_ = connected_space_->addData<IntV>(kHeapFootprint, kHeapSummaryGroup);
  heap_allocated_size_ = connected_space_->addData<IntV>(kHeapAllocatedSize, kHeapSummaryGroup);
  heap_allocated_object_count_ = connected_space_->addData<IntV>(kHeapAllocatedObjectCount, kHeapSummaryGroup);
}

void SummaryManipulator::UpdateSpaceData(Heap* heap, collector::GcType gc_type, bool is_start) {
  if (!is_start) {
    gc_count_array_->value(gc_type - 1) += 1;
  }

  heap_capacity_->value() = heap->capacity_;
  heap_footprint_->value() = heap->max_allowed_footprint_;
  heap_allocated_size_->value() = heap->GetBytesAllocated();
  heap_allocated_object_count_->value() = heap->GetObjectsAllocated();
}

RosManipulator* RosManipulator::Create(const std::string& name, gcview::GCviewSpace* space, int level) {
  return new RosManipulator(name, space, level);
}

void RosManipulator::InitSpaceData() {
  space_capacity_ = connected_space_->addData<IntV>(kSpaceCapacity);
  space_footprint_ = connected_space_->addData<IntV>(kSpaceFootprint);
  space_allocated_size_ = connected_space_->addData<IntV>(kSpaceAllocatedSize);
  space_allocated_object_count_ = connected_space_->addData<IntV>(kSpaceAllocatedObjectCount);

  cpk_array_ = connected_space_->addData<StringA>(kContinuousPagesKindArray);
  cpk_map_array_ = connected_space_->addData<IntA>(kContinuousPagesKindMapArray);
  cpk_empty_count_ = connected_space_->addData<IntV>(kContinuousPagesKindEmptyCount);
  cpk_release_count_ = connected_space_->addData<IntV>(kContinuousPagesKindReleaseCount);
  cpk_empty_or_release_count_ = connected_space_->addData<IntV>(kContinuousPagesKindEmptyOrReleaseCount);
  cpk_large_object_count_ = connected_space_->addData<IntV>(kContinuousPagesKindLargeObjectCount);
  cpk_run_count_ = connected_space_->addData<IntV>(kContinuousPagesKindRunCount);

  cpk_empty_or_rlease_size_array_ = connected_space_->addData<IntA>(kContinuousPagesKindEmptyOrReleaseSizeArray,
                                                                    kContinuousPagesKindEmptyOrReleaseInfoGroup);
  cpk_empty_or_release_range_array_ = connected_space_->addData<StringA>(kContinuousPagesKindEmptyOrReleaseRangeArray,
                                                                          kContinuousPagesKindEmptyOrReleaseInfoGroup);

  cpk_large_object_size_array_ = connected_space_->addData<IntA>(kContinuousPagesKindLargeObjectSizeArray,
                                                                 kContinuousPagesKindLargeObjectInfoGroup);
  cpk_large_object_range_array_ = connected_space_->addData<StringA>(kContinuousPagesKindLargeObjectRangeArray,
                                                                     kContinuousPagesKindLargeObjectInfoGroup);

  cpk_run_size_array_ = connected_space_->addData<IntA>(kContinousPagesKindRunSizeArray,
                                                        kContinousPagesKindRunInfoGroup);
  cpk_range_array_ = connected_space_->addData<StringA>(kContinousPagesKindRunRangeArray,
                                                        kContinousPagesKindRunInfoGroup);
  cpk_allocated_size_array_ = connected_space_->addData<IntA>(kContinousPagesKindRunAllocatedSizeArray,
                                                             kContinousPagesKindRunInfoGroup);
  cpk_run_allocated_object_count_array_ = connected_space_->addData<IntA>(kContinuosPagesKindRunAllocatedObjectCountArray,
                                                                          kContinousPagesKindRunInfoGroup);
  cpk_run_bracket_size_array_ = connected_space_->addData<IntA>(kContinuosPagesKindRunBracketSizeArray,
                                                                kContinousPagesKindRunInfoGroup);
  cpk_run_total_slots_num_array_ = connected_space_->addData<IntA>(kContinuosPagesKindRunTotalSlotsNumArray,
                                                                   kContinousPagesKindRunInfoGroup);
  cpk_run_is_local_array_ = connected_space_->addData<BoolA>(kContinuosPagesKindRunIsLocalArray,
                                                             kContinousPagesKindRunInfoGroup);
  cpk_run_is_bulk_freed_array_ = connected_space_->addData<BoolA>(kContinuosPagesKindRunIsBulkFreedArray,
                                                                  kContinousPagesKindRunInfoGroup);
  cpk_run_is_full_array_ = connected_space_->addData<BoolA>(kContinuosPagesKindRunIsFullArray,
                                                            kContinousPagesKindRunInfoGroup);

  const size_t num_of_brackets = allocator::RosAlloc::kNumOfSizeBrackets;
  run_dist_size_array_ = connected_space_->addData<StringA>(kRunDistSize, kRunDistName);
  run_dist_size_array_->resize(num_of_brackets);
  run_dist_population_array_ = connected_space_->addData<IntA>(kRunDistPopulation, kRunDistName);
  run_dist_population_array_->resize(num_of_brackets);
  run_dist_total_allocated_bytes_array_ = connected_space_->addData<IntA>(kRunDistTotalAllocatedBytes, kRunDistName);
  run_dist_total_allocated_bytes_array_->resize(num_of_brackets);

  for (size_t i = 0; i < num_of_brackets; i++) {
    char buffer[64] = "";
    gcview::Utils::formatStr(buffer, 64, "%u",
                             allocator::RosAlloc::bracketSizes[i]);
    run_dist_size_array_->value(i) = buffer;
  }
}

RosManipulator::PagesInfo* RosManipulator::PagesInfoAlloc() {
  PagesInfo* newPagesInfo;
  newPagesInfo = reinterpret_cast<PagesInfo*>(malloc(sizeof(struct PagesInfo)));
  memset(newPagesInfo, 0, sizeof(struct PagesInfo));
  return newPagesInfo;
}

void RosManipulator::PagesInfoFree(PagesInfo* pi) {
  if (pi == nullptr) {
    return;
  }
  free(pi);
}

void RosManipulator::CollectRosSpaceInfo(space::RosAllocSpace* space, std::vector<PagesInfo*>* pages_info_vec, size_t& empty_num,
                        size_t& release_num, size_t& large_object_num, size_t& run_num) {
  allocator::RosAlloc* rosalloc = space->GetRosAlloc();

  MutexLock mu(Thread::Current(), rosalloc->lock_);
  size_t pm_end = rosalloc->page_map_size_;
  size_t i = 0;
  while (i < pm_end) {
    uint8_t pm = rosalloc->page_map_[i];
    switch (pm) {
      case allocator::RosAlloc::kPageMapReleased:
        // Fall-through.
      case allocator::RosAlloc::kPageMapEmpty: {
        // The start of a free page run.
        allocator::RosAlloc::FreePageRun* fpr = reinterpret_cast<allocator::RosAlloc::FreePageRun*>(rosalloc->base_ + i * kPageSize);
        size_t fpr_size = fpr->ByteSize(rosalloc);
        void * start = fpr;
        if (kIsDebugBuild) {
          // In the debug build, the first page of a free page run
          // contains a magic number for debugging. Exclude it.
          start = reinterpret_cast<uint8_t*>(fpr) + kPageSize;
        }
        void* end = reinterpret_cast<uint8_t*>(fpr) + fpr_size;

        //  Collect empty/release page set info
        char buffer[64] = "";
        if (pm == allocator::RosAlloc::kPageMapReleased) {
          PagesInfo* empty_info = PagesInfoAlloc();
          empty_info->pages_kind.assign(kContinuousPageKindEmptyName);
          empty_info->pages_kind_map = kContinuousPagesKindEmptyOrReleaseMap;
          empty_info->ext_info.cpk_empty_or_release_info.size = fpr_size;
          gcview::Utils::formatStr(buffer, 64, "[%p, %p)", start, end);
          empty_info->ext_info.cpk_empty_or_release_info.address_range.assign(buffer);
          pages_info_vec->push_back(empty_info);
          empty_num++;
        }
        else {
          PagesInfo* release_info = PagesInfoAlloc();
          release_info->pages_kind.assign(kContinuousPageKindRelaseName);
          release_info->pages_kind_map = kContinuousPagesKindEmptyOrReleaseMap;
          release_info->ext_info.cpk_empty_or_release_info.size = fpr_size;
          gcview::Utils::formatStr(buffer, 64, "[%p, %p)", start, end);
          release_info->ext_info.cpk_empty_or_release_info.address_range.assign(buffer);
          pages_info_vec->push_back(release_info);
          release_num++;
        }

        i += fpr_size / kPageSize;
        break;
      }
      
      case allocator::RosAlloc::kPageMapLargeObject: {
        // The start of a large object.
        size_t num_pages = 1;
        size_t idx = i + 1;
        while (idx < pm_end && rosalloc->page_map_[idx] == allocator::RosAlloc::kPageMapLargeObjectPart) {
          num_pages++;
          idx++;
        }
        void* start = rosalloc->base_ + i * kPageSize;
        void* end = rosalloc->base_ + (i + num_pages) * kPageSize;
        size_t used_bytes = num_pages * kPageSize;
        //  Collect large object info
        PagesInfo* large_object_info = PagesInfoAlloc();
        char buffer[64] = "";
        large_object_info->pages_kind.assign(kContinuousPageKindLargeObjectName);
        large_object_info->pages_kind_map = kContinuousPagesKindLargeObjectMap;
        large_object_info->ext_info.cpk_large_object_info.size = used_bytes;
        gcview::Utils::formatStr(buffer, 64, "[%p, %p)", start, end);
        large_object_info->ext_info.cpk_large_object_info.address_range.assign(buffer);
        pages_info_vec->push_back(large_object_info);
        large_object_num++;

        i += num_pages;
        break;
      }
      case allocator::RosAlloc::kPageMapLargeObjectPart:
        LOG(FATAL) << "Unreachable - page set type: " << static_cast<int>(pm);
        break;
      case allocator::RosAlloc::kPageMapRun: {
        // The start of a run.
        allocator::RosAlloc::Run* run = reinterpret_cast<allocator::RosAlloc::Run*>(rosalloc->base_ + i * kPageSize);
        // The dedicated full run doesn't contain any real allocations, don't visit the slots in
        // there.
        size_t idx = run->size_bracket_idx_;
        size_t num_slots = allocator::RosAlloc::numOfSlots[idx];
        size_t bracket_size = allocator::RosAlloc::IndexToBracketSize(idx);
        size_t num_pages = allocator::RosAlloc::numOfPages[idx];
        void* start = rosalloc->base_ + i * kPageSize;
        void* end = rosalloc->base_ + (i + num_pages) * kPageSize;
        
        PagesInfo* run_info = PagesInfoAlloc();
        char buffer[64] = "";
        run_info->pages_kind.assign(kContinuousPageKindRunName);
        run_info->pages_kind_map = kContinuousPagesKindRunMap;
        run_info->ext_info.cpk_run_info.size = num_pages * kPageSize;
        gcview::Utils::formatStr(buffer, 64, "[%p, %p)", start, end);
        run_info->ext_info.cpk_run_info.address_range.assign(buffer);
        run_info->ext_info.cpk_run_info.bracket_size = bracket_size;
        run_info->ext_info.cpk_run_info.total_slots_num = num_slots;
        run_info->ext_info.cpk_run_info.is_local_run = run->is_thread_local_;
        run_info->ext_info.cpk_run_info.is_bulk_freed = run->to_be_bulk_freed_;
        run_info->ext_info.cpk_run_info.is_full = run->IsFull();
        run_info->ext_info.cpk_run_info.allocated_size = 0;
        run_info->ext_info.cpk_run_info.allocated_object_count = 0;

        size_t bytes_allocated = 0;
        run->InspectAllSlots(art::gc::allocator::RosAlloc::BytesAllocatedCallback, &bytes_allocated);
        run_info->ext_info.cpk_run_info.allocated_size = bytes_allocated;
        run_info->ext_info.cpk_run_info.allocated_object_count = bytes_allocated / bracket_size;

        pages_info_vec->push_back(run_info);
        run_num++;
        i += num_pages;
        break;
      }
      case allocator::RosAlloc::kPageMapRunPart:
        LOG(FATAL) << "Unreachable - page set type: " << static_cast<int>(pm);
        break;
      default:
        LOG(FATAL) << "Unreachable - page set type: " << static_cast<int>(pm);
        break;
    }
  }
}

void RosManipulator::UpdateSpaceData(Heap* heap, collector::GcType gc_type ATTRIBUTE_UNUSED, 
                                     bool is_start ATTRIBUTE_UNUSED) {
  std::vector<PagesInfo*>* pages_info_vector = new std::vector<PagesInfo*>();
  size_t empty_num = 0;
  size_t release_num = 0;
  size_t large_object_num = 0;
  size_t run_num = 0;

  CollectRosSpaceInfo(heap->GetRosAllocSpace(), pages_info_vector, empty_num, release_num, large_object_num, run_num);  

  space_capacity_->value() = heap->GetRosAllocSpace()->Capacity();
  space_footprint_->value() = heap->GetRosAllocSpace()->Size();
  space_allocated_size_->value() = heap->GetRosAllocSpace()->GetBytesAllocated();
  space_allocated_object_count_->value() = heap->GetRosAllocSpace()->GetObjectsAllocated();

  cpk_array_->resize(pages_info_vector->size());
  cpk_map_array_->resize(pages_info_vector->size());
  cpk_empty_count_->value() = empty_num;
  cpk_release_count_->value() = release_num;
  cpk_empty_or_release_count_->value() = empty_num + release_num;
  cpk_large_object_count_->value() = large_object_num;
  cpk_run_count_->value() = run_num;

  for (size_t i = 0; i < allocator::RosAlloc::kNumOfSizeBrackets; i++) {
    run_dist_population_array_->value(i) = 0;
    run_dist_total_allocated_bytes_array_->value(i) = 0;
  }

  cpk_empty_or_rlease_size_array_->resize(empty_num + release_num);
  cpk_empty_or_release_range_array_->resize(empty_num + release_num);

  cpk_large_object_size_array_->resize(large_object_num);
  cpk_large_object_range_array_->resize(large_object_num); 

  cpk_run_size_array_->resize(run_num);
  cpk_range_array_->resize(run_num);
  cpk_allocated_size_array_->resize(run_num);
  cpk_run_allocated_object_count_array_->resize(run_num);
  cpk_run_bracket_size_array_->resize(run_num);
  cpk_run_total_slots_num_array_->resize(run_num);
  cpk_run_is_local_array_->resize(run_num);
  cpk_run_is_bulk_freed_array_->resize(run_num);
  cpk_run_is_full_array_->resize(run_num);

  size_t empty_or_release_idx = 0;
  size_t large_object_idx = 0;
  size_t run_idx = 0;

  for (size_t i = 0; i < pages_info_vector->size(); i++) {
    cpk_array_->value(i) = (*pages_info_vector)[i]->pages_kind.c_str();
    cpk_map_array_->value(i) =  (*pages_info_vector)[i]->pages_kind_map;
    switch ((*pages_info_vector)[i]->pages_kind_map) {
      case kContinuousPagesKindEmptyOrReleaseMap: {
        cpk_empty_or_rlease_size_array_->value(empty_or_release_idx) = 
                                         (*pages_info_vector)[i]->ext_info.cpk_empty_or_release_info.size;
        cpk_empty_or_release_range_array_->value(empty_or_release_idx) =
                                         (*pages_info_vector)[i]->ext_info.cpk_empty_or_release_info.address_range.c_str();
        empty_or_release_idx++;
        break;
      }
      case kContinuousPagesKindLargeObjectMap: {
        cpk_large_object_size_array_->value(large_object_idx) =
                                      (*pages_info_vector)[i]->ext_info.cpk_large_object_info.size;
        cpk_large_object_range_array_->value(large_object_idx) =
                                      (*pages_info_vector)[i]->ext_info.cpk_large_object_info.address_range.c_str();
        large_object_idx++;
        break;
      }
      case kContinuousPagesKindRunMap: {
        size_t bracket_index = allocator::RosAlloc::BracketSizeToIndex((*pages_info_vector)[i]->ext_info.cpk_run_info.bracket_size);
        run_dist_population_array_->value(bracket_index) += 1;
        run_dist_total_allocated_bytes_array_->value(bracket_index) += 
                                               (*pages_info_vector)[i]->ext_info.cpk_run_info.allocated_object_count;

        cpk_run_size_array_->value(run_idx) = (*pages_info_vector)[i]->ext_info.cpk_run_info.size;
        cpk_range_array_->value(run_idx) = (*pages_info_vector)[i]->ext_info.cpk_run_info.address_range.c_str();
        cpk_allocated_size_array_->value(run_idx) = (*pages_info_vector)[i]->ext_info.cpk_run_info.allocated_size;
        cpk_run_allocated_object_count_array_->value(run_idx) = (*pages_info_vector)[i]->ext_info.cpk_run_info.allocated_object_count;
        cpk_run_bracket_size_array_->value(run_idx) = (*pages_info_vector)[i]->ext_info.cpk_run_info.bracket_size;
        cpk_run_total_slots_num_array_->value(run_idx) = (*pages_info_vector)[i]->ext_info.cpk_run_info.total_slots_num;
        cpk_run_is_local_array_->value(run_idx) = (*pages_info_vector)[i]->ext_info.cpk_run_info.is_local_run;
        cpk_run_is_bulk_freed_array_->value(run_idx) = (*pages_info_vector)[i]->ext_info.cpk_run_info.is_bulk_freed;
        cpk_run_is_full_array_->value(run_idx) = (*pages_info_vector)[i]->ext_info.cpk_run_info.is_full;
        run_idx++;
        break;
      }
      default: {
        for(auto it = pages_info_vector->begin(); it != pages_info_vector->end(); it++)
          PagesInfoFree(*it);
        delete pages_info_vector;
        LOG(FATAL) << "Unreachable - page set info type: " << (*pages_info_vector)[i]->pages_kind.c_str();
      }
    }
  }

  for(auto it = pages_info_vector->begin(); it != pages_info_vector->end(); it++)
    PagesInfoFree(*it);
  delete pages_info_vector;
}

LargeObjectManipulator* LargeObjectManipulator::Create(const std::string& name, gcview::GCviewSpace* space, int level) {
  return new LargeObjectManipulator(name, space, level);
}

void LargeObjectManipulator::InitSpaceData() {

  space_allocated_size_ = connected_space_->addData<IntV>(kLOSBytesAllocated);
  space_allocated_object_count_ = connected_space_->addData<IntV>(kLOSObjectsAllocated);
  space_total_allocated_size_ = connected_space_->addData<IntV>(kLOSTotalBytesAllocated);
  space_total_allocated_object_count_ = connected_space_->addData<IntV>(kLOSTotalObjectsAllocated);
  los_object_range_array_ = connected_space_->addData<StringA>(kLOSObjectAddressRangeArray, kLOSObjectInfoGroup);
  los_object_length_array_ = connected_space_->addData<IntA>(kLOSObjectLengthArray, kLOSObjectInfoGroup);

  // Add mem_map info for LargeObjectMapace.
  if (strcmp(connected_space_->getName(), "mem map large object space") == 0) {
    los_memmap_range_array_ = connected_space_->addData<StringA>(kLOSMemMapRangeArray,
                                                                 kLOSMemMapInfoGroup);
    los_memmap_size_array_ = connected_space_->addData<IntA>(kLOSMemMapSizeArray,
                                                             kLOSMemMapInfoGroup);
    los_memmap_is_memmap_array_ = connected_space_->addData<BoolA>(kLOSMemMapIsMemMapArray,
                                                                   kLOSMemMapInfoGroup); 
  } else {
    //To  for Freelist
  }
}

void LargeObjectManipulator::CollectLOSObjectAddressInfo(void *start , void *end, size_t num_bytes, void* arg) {
  if (num_bytes == 0) {
    return;
  }
  address_info* object_address_info = reinterpret_cast<address_info*>(arg); 
  object_address_info->push_back(std::make_pair(reinterpret_cast<uint8_t*>(start), 
                                                reinterpret_cast<uint8_t*>(end)));
}

void LargeObjectManipulator::UpdateSpaceData(Heap* heap, collector::GcType gc_type ATTRIBUTE_UNUSED,
                                             bool is_start ATTRIBUTE_UNUSED) {
  space_allocated_size_->value() = heap->GetLargeObjectsSpace()->GetBytesAllocated();
  space_allocated_object_count_->value() = heap->GetLargeObjectsSpace()->GetObjectsAllocated();
  space_total_allocated_size_->value() = heap->GetLargeObjectsSpace()->GetTotalBytesAllocated();
  space_total_allocated_object_count_->value() = heap->GetLargeObjectsSpace()->GetTotalObjectsAllocated();

  // collect object address range info
  address_info los_object_info;
  heap->GetLargeObjectsSpace()->Walk(CollectLOSObjectAddressInfo, &los_object_info);

  los_object_range_array_->resize(los_object_info.size());
  los_object_length_array_->resize(los_object_info.size());

  // hole_count to record fragmentation between mem_map
  size_t hole_count = 0;
  for (size_t i = 0; i < los_object_info.size(); i++) {
    char buffer[64] = "";
    gcview::Utils::formatStr(buffer, 64, "[%p, %p)", los_object_info[i].first, 
                             los_object_info[i].second);
    los_object_range_array_->value(i) = buffer;
    los_object_length_array_->value(i) = los_object_info[i].second - los_object_info[i].first;

    size_t page_aligned_byte_count = RoundUp(los_object_info[i].second - los_object_info[i].first, kPageSize);
    if (i < los_object_info.size() - 1 &&
        los_object_info[i + 1].first != los_object_info[i].first + page_aligned_byte_count)
      hole_count++;
  }

  if (strcmp(connected_space_->getName(), "mem map large object space") == 0) {
    los_memmap_range_array_->resize(los_object_info.size() + hole_count);
    los_memmap_size_array_->resize(los_object_info.size() + hole_count);
    los_memmap_is_memmap_array_->resize(los_object_info.size() + hole_count);

    for(size_t i = 0, idx = 0; i < los_object_info.size(); i++, idx++) {
      char buffer[64] = "";
      size_t page_aligned_byte_count = RoundUp(los_object_info[i].second - los_object_info[i].first, kPageSize);
      los_memmap_size_array_->value(idx) = page_aligned_byte_count;
      los_memmap_is_memmap_array_->value(idx) = true;
      gcview::Utils::formatStr(buffer, 64, "[%p, %p)", los_object_info[i].first,
          los_object_info[i].first + page_aligned_byte_count);
      los_memmap_range_array_->value(idx) = buffer;

      if (i < los_object_info.size() - 1 &&
          los_object_info[i + 1].first != los_object_info[i].first + page_aligned_byte_count) {
        idx++;
        los_memmap_size_array_->value(idx) = 0;
        los_memmap_is_memmap_array_->value(idx) = false;
        gcview::Utils::formatStr(buffer, 64, "[%p, %p)", los_object_info[i].first + page_aligned_byte_count,
            los_object_info[i + 1].first);
        los_memmap_range_array_->value(idx) = buffer;
      }
    }
  }
}

}  // namespace gc
}  // namespace art
