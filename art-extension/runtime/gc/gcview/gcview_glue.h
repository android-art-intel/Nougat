// Copyright 2012 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef ART_RUNTIME_GC_GCVIEW_GCVIEW_GLUE_H_
#define ART_RUNTIME_GC_GCVIEW_GCVIEW_GLUE_H_

#include <limits>
#include <vector>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include "gc/space/space-inl.h"
#include "gc/heap-inl.h"
#include "mirror/class-inl.h"
#include "mirror/object.h"
#include "mirror/object-inl.h"
#include "mirror/object_array-inl.h"
#include "os.h"
#include "gcview.h"
#include "gc/collector/gc_type.h"
#include "gcview_space.h"

namespace art {
namespace gc {
namespace gcview {

// We do forward declarations of the GCview classes so that we don't have
// to recompile everything every time one of those classes is changed.
class GCview;
class JSONArrayWriter;
class JSONWriter;

} // namespace gcview
} // namespace gc
} // namespace art

namespace art {
namespace gc {
namespace space {
  class AllocSpace;
  class BumpPointerSpace;
  class ContinuousMemMapAllocSpace;
  class DiscontinuousSpace;
  class DlMallocSpace;
  class ImageSpace;
  class LargeObjectSpace;
  class MallocSpace;
  class RegionSpace;
  class RosAllocSpace;
  class Space;
  class SpaceTest;
  class ZygoteSpace;
}  // namespace space
}  // namespace gc
}  // namespace art

namespace art {
namespace gc {
namespace allocator {

  class RosAlloc;

}  // namespace space
}  // namespace gc
}  // namespace art
namespace art {
namespace gc {

class GCviewGlue {

public:

  void Start(Heap* heap);

  void Stop();

  void DoEvent(Heap* heap, collector::GarbageCollector* collector, collector::GcType gc_type,
               GcCause gc_cause, bool is_start);

  bool IsRunning() { 
    return gcview_profile_running_; 
  }

  void SetDir(std::string dir) {
    data_dir_ = dir;
  }

  static GCviewGlue* GetInstance() {
    return &s_instance_;
  }

  ~GCviewGlue(); 

private:

  GCviewGlue();

  static double GetNowSec() { return NanoTime() / 1000000000.0; }

  bool IsLevelMedium() const { return level_ >= 1; }
  bool IsLevelHigh() const { return level_ >= 2; }

  gcview::GCviewSpace* AddRosSpace(const char* space_name);
  gcview::GCviewSpace* AddLargeObjectSpace(const char* space_name);
  gcview::GCviewSpace* AddSummarySpace(const char* space_name);
  void UpdateSummarySpace(const char* space_name, Heap* heap, collector::GcType gc_type, bool is_start);
  void UpdateRosSpace(const char* space_name, Heap* heap, collector::GcType gc_type, bool is_start);
  void UpdateLargeObjectSpace(const char* space_name, Heap* heap, collector::GcType gc_type, bool is_start);

  // A pointer to the active gcview glue or NULL.
  static GCviewGlue s_instance_;
  // 0 -> low, 1 -> medium, 2 -> high
  int level_;
  // Indicate if gcview is running
  bool gcview_profile_running_;
  // Lock for start/end/do_event operation of gcview
  Mutex* gcview_profile_lock_;

  std::string data_dir_;
  gcview::GCview* gcview_;
  gcview::JSONWriter* writer_;
  gcview::JSONArrayWriter* array_writer_;

  DISALLOW_COPY_AND_ASSIGN(GCviewGlue);
};

}
}  // namespace art::gc

#endif // ART_RUNTIME_GC_GCVIEW_GCVIEW_GLUE_H_
