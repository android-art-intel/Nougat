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

#include <limits>
#include <vector>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <fstream>
#include "gc/collector/garbage_collector.h"
#include "gc/space/large_object_space.h"
#include "gc/space/rosalloc_space-inl.h"
#include "gc/space/space-inl.h"
#include "gc/heap-inl.h"
#include "mirror/class-inl.h"
#include "mirror/object.h"
#include "mirror/object-inl.h"
#include "mirror/object_array-inl.h"
#include "os.h"
#include "runtime.h"
#include "thread_list.h"
#include "gcview_glue.h"
#include "gcview.h"
#include "manipulator.h"
#include "gc/allocator/rosalloc.h"

#include "base/mutex-inl.h"
#include <map>
#include <list>
#include <sstream>

namespace art {
namespace gc {
namespace allocator {
  class RosAlloc;
}

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

// GCview event names
static const char* kStickyGCStartEventName = "Sticky Start";
static const char* kStickyGCEndEventName = "Sticky End";
static const char* kPartialGCStartEventName = "Partial Start";
static const char* kPartialGCEndEventName = "Partial End";
static const char* kFullGCStartEventName = "Full GC Start";
static const char* kFullGCEndEventName = "Full GC End";

// GCview Cause names

static const std::map<GcCause, const char*> kCauseMap = {
  { kGcCauseForAlloc, "Allocation" },
  { kGcCauseBackground, "Backgournd GC" },
  { kGcCauseExplicit, "Ecplicit gc" },
  { kGcCauseForNativeAlloc, "Native allocation" },
  { kGcCauseCollectorTransition, "Collector transition" },
  { kGcCauseDisableMovingGc, "Disable moving gc" },
  { kGcCauseTrim, "Trim the heap" },
  { kGcCauseHomogeneousSpaceCompact, "Background transition" }
};

// GCview data names for summary space
static const char* kSummarySpaceName = "Summary";

GCviewGlue GCviewGlue::s_instance_;

GCviewGlue::GCviewGlue()
    : level_(0),
      gcview_profile_running_(false),
      gcview_profile_lock_(nullptr),
      data_dir_("data/local/tmp/gcview"),
      gcview_(nullptr),
      writer_(nullptr),
      array_writer_(nullptr) {
  gcview_profile_lock_ = new Mutex("GCview profile lock");
}

GCviewGlue::~GCviewGlue() { 
  Stop(); 
}

void GCviewGlue::Start(Heap* heap) NO_THREAD_SAFETY_ANALYSIS{
  std::string suffix = "trace";

  LOG(INFO) << "GCview profie Start";
  Thread* self = Thread::Current();
  MutexLock mu(self, *gcview_profile_lock_);

  if (gcview_profile_running_) {
    return;
  }

  level_ = 0;
  gcview_ = new gcview::GCview("ART", GetNowSec());
  gcview_->addEvent(kStickyGCStartEventName);
  gcview_->addEvent(kStickyGCEndEventName);
  gcview_->addEvent(kPartialGCStartEventName);
  gcview_->addEvent(kPartialGCEndEventName);
  gcview_->addEvent(kFullGCStartEventName);
  gcview_->addEvent(kFullGCEndEventName);

  AddSummarySpace(kSummarySpaceName);
  AddRosSpace(heap->GetRosAllocSpace()->GetName());
  AddLargeObjectSpace(heap->GetLargeObjectsSpace()->GetName());

  char file_name[256];
  gcview::Utils::formatStr(file_name, 256, "%s/gcview_profile_log_%d.%s",
                              data_dir_.c_str(), getpid(), suffix.c_str());
  writer_ = new gcview::JSONWriter(file_name);
  array_writer_ = new gcview::JSONArrayWriter(writer_);

  array_writer_->startElem();
  gcview_->writeJSONMetadata(writer_);

  gcview_profile_running_ = true;
}

gcview::GCviewSpace* GCviewGlue::AddSummarySpace(const char* space_name) {
  gcview::GCviewSpace* sp = gcview_->addSpace(space_name);
  Manipulator* manipulator = SummaryManipulator::Create(space_name, sp, level_);
  sp->setManipulator(manipulator);
  manipulator->InitSpaceData();

  return sp;
}

gcview::GCviewSpace* GCviewGlue::AddRosSpace(const char* space_name) {
  gcview::GCviewSpace* sp = gcview_->addSpace(space_name);
  Manipulator* manipulator = RosManipulator::Create(space_name, sp, level_);
  sp->setManipulator(manipulator);
  manipulator->InitSpaceData();

  return sp;
}

gcview::GCviewSpace* GCviewGlue::AddLargeObjectSpace(const char* space_name) {
  gcview::GCviewSpace* sp = gcview_->addSpace(space_name);
  Manipulator* manipulator = LargeObjectManipulator::Create(space_name, sp, level_);
  sp->setManipulator(manipulator);
  manipulator->InitSpaceData();

  return sp;
}

void GCviewGlue::UpdateSummarySpace(const char* space_name, Heap* heap, collector::GcType gc_type, bool is_start) {
  gcview::GCviewSpace* sp = gcview_->findSpace(space_name);
  sp->getManipulator()->UpdateSpaceData(heap, gc_type, is_start);
}

void GCviewGlue::UpdateRosSpace(const char* space_name, Heap* heap, collector::GcType gc_type, bool is_start) {
  gcview::GCviewSpace* sp = gcview_->findSpace(space_name);
  sp->getManipulator()->UpdateSpaceData(heap, gc_type, is_start);
}

void GCviewGlue::UpdateLargeObjectSpace(const char* space_name, Heap* heap, collector::GcType gc_type, bool is_start) {
  gcview::GCviewSpace* sp = gcview_->findSpace(space_name);
  sp->getManipulator()->UpdateSpaceData(heap, gc_type, is_start);
}

void GCviewGlue::DoEvent(Heap* heap, collector::GarbageCollector* collector ATTRIBUTE_UNUSED, collector::GcType gc_type,
                         GcCause gc_cause, bool is_start) NO_THREAD_SAFETY_ANALYSIS{

  const char* event_name = nullptr;

  switch(gc_type) {
    case collector::kGcTypeSticky:
      event_name = (is_start) ? kStickyGCStartEventName : kStickyGCEndEventName;
      break;
    case collector::kGcTypePartial:
      event_name = (is_start) ? kPartialGCStartEventName : kPartialGCEndEventName;
      break;
    case collector::kGcTypeFull:
      event_name = (is_start) ? kFullGCStartEventName : kFullGCEndEventName;
      break;
    default:
      break;
  }

  LOG(INFO) << "avi1 gcview_: " << gcview_ << "gcview_profile_running_: " << gcview_profile_running_;

  if (gcview_ == nullptr) return;
  if (!gcview_profile_running_) return;

  LOG(INFO) << "avi2 gcview_: " << gcview_ << "gcview_profile_running_: " << gcview_profile_running_;

  if (gcview_->eventStart(event_name, GetNowSec())) {
    //  Update Space Info
    {
      //collector::GarbageCollector::ScopedPause pause(collector);
      UpdateSummarySpace(kSummarySpaceName, heap, gc_type, is_start);
      UpdateRosSpace(heap->GetRosAllocSpace()->GetName(), heap, gc_type, is_start);
      UpdateRosSpace(heap->GetLargeObjectsSpace()->GetName(), heap, gc_type, is_start);
    }


    Thread* self = Thread::Current();
    MutexLock mu(self, *gcview_profile_lock_);

    if (gcview_ == nullptr) return;
    if (!gcview_profile_running_) return;

    const char* cause = kCauseMap.find(gc_cause)->second;
    if (cause == nullptr) {
      cause = "Unknown";
    }
    gcview_->eventEnd(cause, GetNowSec());
    array_writer_->startElem();
    gcview_->writeJSONData(writer_);
  }
}

void GCviewGlue::Stop() NO_THREAD_SAFETY_ANALYSIS{
  Thread* self = Thread::Current();
  MutexLock mu(self, *gcview_profile_lock_);

  if (!gcview_profile_running_) {
    return;
  }

  gcview_profile_running_ = false;

  if (gcview_ == nullptr) return;

  delete array_writer_;
  array_writer_ = nullptr;

  delete writer_;
  writer_ = nullptr;

  delete gcview_;
  gcview_ = nullptr;

  LOG(INFO) << "GCview profile finish!";
}

}
}  // namespace art::gc
