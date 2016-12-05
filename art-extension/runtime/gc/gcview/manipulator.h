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

#ifndef ART_RUNTIME_GC_GCVIEW_MANIPULATOR_H_
#define ART_RUNTIME_GC_GCVIEW_MANIPULATOR_H_

#include "gc/collector/gc_type.h"
#include "globals.h" 
#include "base/macros.h" 
#include "gcview.h"
#include "gcview_space.h"
#include "gc/space/space-inl.h"
#include <string>
namespace art {
namespace gc {

class Heap;

typedef gcview::BoolValue BoolV;
typedef gcview::ByteValue ByteV;
typedef gcview::IntValue IntV;
typedef gcview::DoubleValue DoubleV;
typedef gcview::StringValue StringV;
typedef gcview::EnumValue EnumV;

typedef gcview::BoolArray BoolA;
typedef gcview::ByteArray ByteA;
typedef gcview::IntArray IntA;
typedef gcview::DoubleArray DoubleA;
typedef gcview::StringArray StringA;
typedef gcview::EnumArray EnumA;

// A manipulator contains operation to manipulate gcview space.
class Manipulator {
 public:
  const char* GetName() const {
    return name_.c_str();
  }

  bool IsLevelMedium() const { return level_ >= 1; }
  bool IsLevelHigh() const { return level_ >= 2; }

  // Init the gcview space data it connect to.
  virtual void InitSpaceData() = 0;

  // Update the date in connected gcview space.
  virtual void UpdateSpaceData(Heap* heap, collector::GcType gc_type, bool is_start) = 0;

  virtual ~Manipulator() {}

 protected:
  Manipulator(const std::string& name, gcview::GCviewSpace* space, int level);

  std::string name_;
  gcview::GCviewSpace* connected_space_;
  int level_;

 private:
  DISALLOW_COPY_AND_ASSIGN(Manipulator);
};

// A manipulator for gcview summary space.
class SummaryManipulator : public Manipulator {
 public:
   static SummaryManipulator* Create(const std::string& name, gcview::GCviewSpace* space, int level);

   virtual ~SummaryManipulator() {}

   virtual void InitSpaceData();
   virtual void UpdateSpaceData(Heap* heap, collector::GcType gc_type, bool is_start);

 protected:
   SummaryManipulator(const std::string& name, gcview::GCviewSpace* space, int level) :
                    Manipulator(name, space, level) {
   }

 private:
   StringV* level_value_;
   StringA* gc_type_array_;
   IntA* gc_count_array_;
   IntV* heap_capacity_;
   IntV* heap_footprint_;
   IntV* heap_allocated_size_;
   IntV* heap_allocated_object_count_;

   DISALLOW_COPY_AND_ASSIGN(SummaryManipulator);
};

// A manipulator for gcview ros space.
class RosManipulator : public Manipulator {
  public:
    static RosManipulator* Create(const std::string& name, gcview::GCviewSpace* space, int level);

    virtual ~RosManipulator() {}

    virtual void InitSpaceData();
    virtual void UpdateSpaceData(Heap* heap, collector::GcType gc_type, bool is_start);

  protected:
    RosManipulator(const std::string& name, gcview::GCviewSpace* space, int level) :
                  Manipulator(name, space, level) {
    }

  private:
    struct PagesInfo {
      std::string pages_kind;
      int pages_kind_map;
      union {
        struct {
          std::string address_range;
          size_t size;
        } cpk_empty_or_release_info;

        struct {
          std::string address_range;
          size_t size;
        } cpk_large_object_info;

        struct {
          std::string address_range;
          size_t size;
          uint64_t allocated_size;
          uint64_t allocated_object_count;
          uint32_t bracket_size;
          uint32_t total_slots_num;
          bool is_local_run;
          bool is_bulk_freed;
          bool is_full;
        } cpk_run_info;

      } ext_info;
    };

    PagesInfo* PagesInfoAlloc();
    void PagesInfoFree(PagesInfo* pi);

    void CollectRosSpaceInfo(space::RosAllocSpace* space, std::vector<PagesInfo*>* pages_info_vec, size_t& empty_num,
                           size_t& release_num, size_t& large_object_num, size_t& run_num);

  private:
    // Info for Space
    IntV* space_capacity_;
    IntV* space_footprint_;
    IntV* space_allocated_size_;
    IntV* space_allocated_object_count_;

    // cpk stand for continuos pages kind
    StringA* cpk_array_;
    IntA* cpk_map_array_;
    IntV* cpk_empty_count_;
    IntV* cpk_release_count_;
    IntV* cpk_empty_or_release_count_;
    IntV* cpk_large_object_count_;
    IntV* cpk_run_count_;

    // Info for Empty/Release Kind
    IntA* cpk_empty_or_rlease_size_array_;
    StringA* cpk_empty_or_release_range_array_;

    // Info for Large Object Kind
    IntA* cpk_large_object_size_array_;
    StringA* cpk_large_object_range_array_;

    // Info for Run Kind
    IntA* cpk_run_size_array_;
    StringA* cpk_range_array_;
    IntA* cpk_allocated_size_array_;
    IntA* cpk_run_allocated_object_count_array_;
    IntA* cpk_run_bracket_size_array_;
    IntA* cpk_run_total_slots_num_array_;
    BoolA* cpk_run_is_local_array_;
    BoolA* cpk_run_is_bulk_freed_array_;
    BoolA* cpk_run_is_full_array_;

    StringA* run_dist_size_array_;
    IntA* run_dist_population_array_;
    IntA* run_dist_total_allocated_bytes_array_;
    
    DISALLOW_COPY_AND_ASSIGN(RosManipulator);
};

class LargeObjectManipulator : public Manipulator {
  public:
    static LargeObjectManipulator* Create(const std::string& name, gcview::GCviewSpace* space, int level);

    virtual ~LargeObjectManipulator() {}

    virtual void InitSpaceData();
    virtual void UpdateSpaceData(Heap* heap, collector::GcType gc_type, bool is_start);

  protected:
    LargeObjectManipulator(const std::string& name, gcview::GCviewSpace* space, int level) :
                          Manipulator(name, space, level) {
    }

  private:
    typedef std::vector<std::pair<uint8_t*, uint8_t*> > address_info;
    static void CollectLOSObjectAddressInfo(void *start , void *end, size_t num_bytes, void* arg);

  private:
    // Info for Space
    IntV* space_allocated_size_;
    IntV* space_allocated_object_count_;
    IntV* space_total_allocated_size_;
    IntV* space_total_allocated_object_count_;
    StringA* los_object_range_array_;
    IntA* los_object_length_array_;
    StringA* los_memmap_range_array_;
    IntA* los_memmap_size_array_;
    BoolA* los_memmap_is_memmap_array_;

    DISALLOW_COPY_AND_ASSIGN(LargeObjectManipulator);
};

}  // namespace gc
}  // namespace art

#endif  // ART_RUNTIME_GC_GCVIEW_MANIPULATOR_H_
