/**
 ** Copyright (C) 2015 Intel Corporation.
 **
 ** See the file "Kent_license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

#include "gcspy.h"

#include <algorithm>

#include "art_field-inl.h"
#include "base/logging.h"
#include "class_linker.h"
#include "dex_file-inl.h"
#include "gc_root.h"
#include "gc/accounting/heap_bitmap.h"
#include "gc/allocation_record.h"
#include "gc/allocator/rosalloc.h"
#include "gc/collector_type.h"
#include "gc/heap.h"
#include "gc/space/bump_pointer_space.h"
#include "gc/space/dlmalloc_space-inl.h"
#include "gc/space/image_space.h"
#include "gc/space/large_object_space.h"
#include "gc/space/region_space.h"
#include "gc/space/rosalloc_space-inl.h"
#include "gc/space/space-inl.h"
#include "gc/space/zygote_space.h"
#include "mirror/class.h"
#include "mirror/class-inl.h"
#include "mirror/object-inl.h"
#include "scoped_thread_state_change.h"
#include "thread.h"
#include "thread-inl.h"
#include "thread_list.h"

extern "C" {
#include "gcspy_comm.h"
#include "gcspy_timer.h"
  //#include "gcspy_d_utils.h"
#include "gcspy_gc_driver.h"
#include "gcspy_main_server.h"
}

#include "space_record.h"

#ifdef GCSPY_WITH_TESTING_THREAD
#include "dummy_driver.h"
#endif

namespace art {

namespace gc {

namespace gcspy {

GcSpy *GcSpy::instance_ = nullptr;

struct ContinuousSpaceTile {
  int usedSpace;
  int roots;
};

struct ImageSpaceTile {
  int usedSpace;
  int roots;
};

struct NonMovingSpaceTile {
  int usedSpace;
  int roots;
};

struct BumpPointerSpaceTile {
  int usedSpace;
  int roots;
};

struct RosAllocSpaceTile {
  int usedSpace;
  int roots;
};

struct RegionSpaceTile {
  int usedSpace;
  int roots;
};

class GcSpyMainLoop {
 public:
  void operator() () {
    gcspy_mainServerMainLoop(&(GcSpy::instance_->server_));
  }
};


class Driver : public SingleRootVisitor {
 public:

  virtual ~Driver() {}

  void
  Init(const char *name, const char *driverName, const char *title,
       const size_t blockSize, const size_t tileNum, const bool mainSpace) {

    char blockInfo[128];
    if (blockSize < 1024)
      sprintf(blockInfo, "Block Size: %zu bytes\n", blockSize);
    else
      sprintf(blockInfo, "Block Size: %zuK\n", blockSize>>10);

    gcspy_driverInit(driver_, -1, name, driverName, title, blockInfo, tileNum,
                     NULL, (mainSpace ? 1 : 0));
  }


  virtual void VisitRoot(mirror::Object* obj, const RootInfo& root_info) = 0;

  virtual void Init(const char *name, const char *driverName, const char *title) = 0;

  virtual void Update() = 0;
  virtual void Send() = 0;

  virtual void AddSpace(space::Space *space) = 0;
  virtual void RemoveSpace(space::Space *space) = 0;

  DriverTypes GetType() { return type_; }
  int GetId() { return driver_->id; }

  void Resize(int tileNum) {
    gcspy_driverResize(driver_, tileNum);
  }

  int GetTileNum() { return driver_->tileNum; }

 protected:
  Driver(gcspy_gc_driver_t *driver, DriverTypes type) : driver_(driver), type_(type) {}

 protected:
  gcspy_gc_driver_t *driver_;
  DriverTypes type_;

 private:
  DISALLOW_COPY_AND_ASSIGN(Driver);
};

class ContinuousSpaceDriver : public Driver {
 public:
  static constexpr size_t kDefaultBlockSize = 32 * 1024;

  //TODO: add features with more streams
  static constexpr int USED_SPACE_STREAM = 0;
  static constexpr int ROOTS_STREAM      = 1;

  ContinuousSpaceDriver(gcspy_gc_driver_t *driver, DriverTypes type)
      : Driver(driver, type),
        totalUsedSpace_(0), totalSpace_(0), totalRoots_(0) {}

  virtual ~ContinuousSpaceDriver() {
    for (SpaceRecord *record : records_) {
      delete record;
    }
  }

  // Init an non-main space
  virtual void Init(const char *name, const char *driverName, const char *title) {
    Init(name, driverName, title, false);
  }

  virtual void Init(const char *name, const char *driverName, const char *title, bool isMainSpace);
  virtual void Update() SHARED_REQUIRES(Locks::mutator_lock_);
  virtual void Send();
  virtual void AddSpace(space::Space *space);
  virtual void RemoveSpace(space::Space *space);
  virtual void VisitRoot(mirror::Object* obj, const RootInfo& root_info ATTRIBUTE_UNUSED)
      OVERRIDE SHARED_REQUIRES(Locks::mutator_lock_);
  void SetupTileNames();
  virtual void Reset();
 protected:
  int totalUsedSpace_;
  int totalSpace_;
  int totalRoots_;
  std::vector<SpaceRecord *> records_;
};

void ContinuousSpaceDriver::Init(const char *name, const char *driverName, const char *title, bool isMainSpace) {
  size_t tileNum = 0u;
  size_t blockSize = ContinuousSpaceDriver::kDefaultBlockSize;
  Driver::Init(name, driverName, title, blockSize, tileNum, isMainSpace);

  gcspy_gc_stream_t *stream;

  stream = gcspy_driverAddStream(driver_, ContinuousSpaceDriver::USED_SPACE_STREAM);
  gcspy_streamInit(stream, ContinuousSpaceDriver::USED_SPACE_STREAM, GCSPY_GC_STREAM_INT_TYPE,
                   "Used Space", 0, blockSize, 0, 0, "Used Space: ", " bytes",
                   GCSPY_GC_STREAM_PRESENTATION_PERCENT,
                   GCSPY_GC_STREAM_PAINT_STYLE_ZERO, 0,
                   gcspy_colorDBGetColorWithName("Red"));

  stream = gcspy_driverAddStream(driver_, ContinuousSpaceDriver::ROOTS_STREAM);
  gcspy_streamInit(stream, ContinuousSpaceDriver::ROOTS_STREAM, GCSPY_GC_STREAM_INT_TYPE,
                   "Roots", 0, gcspy_d_utils_roots_per_block(blockSize),
                   0, 0, "Roots: ", "",
                   GCSPY_GC_STREAM_PRESENTATION_PLUS,
                   GCSPY_GC_STREAM_PAINT_STYLE_ZERO, 0,
                   gcspy_colorDBGetColorWithName("Navy Blue"));
}

void ContinuousSpaceDriver::Update() {
  Reset();

  //TODO: get used space
  int tileNum = 0;
  for (SpaceRecord *r : records_) {
    tileNum += r->GetTileNum();
    totalSpace_ += r->GetSize();
  }
  Resize(tileNum);
  SetupTileNames();

  // Get roots
  Runtime *runtime = Runtime::Current();
  if (Locks::mutator_lock_->IsExclusiveHeld(Thread::Current())) {
    runtime->VisitRoots(this);
  } else {
    runtime->VisitNonThreadRoots(this);
    runtime->VisitConcurrentRoots(this);
  }
}

void ContinuousSpaceDriver::AddSpace(space::Space *space) {
  CHECK(space->IsContinuousSpace());
  space::ContinuousSpace *ispace = space->AsContinuousSpace();
  SpaceRecord *record = new SpaceRecord(space, kDefaultBlockSize, sizeof(ContinuousSpaceTile));
  //record->SetPerc((uintptr_t)ispace->Size(), OFFSETOF_MEMBER(ContinuousSpaceTile, usedSpace));
  records_.push_back(record);

  LOG(INFO) << "ContinuousSpaceDriver::AddSpace, Begin: " << (size_t)ispace->Begin()
            << ", End: " << (size_t)ispace->End();
  // Sort spaces and assume they are in continue spaces
  std::sort(records_.begin(), records_.end(),
            [](SpaceRecord *r1, SpaceRecord *r2) {
              return r1->Begin() < r2->Begin();
            });
}

void ContinuousSpaceDriver::RemoveSpace(space::Space *space) {
  records_.erase(std::remove_if(records_.begin(), records_.end(),
                                [space] (SpaceRecord *record) {
                                  if (record->GetSpace() == space) {
                                    delete record;
                                    return true;
                                  }
                                  return false; }));
  // remove this driver when no active space left */
  if (records_.empty()) {
    delete this;
  }
}

void ContinuousSpaceDriver::SetupTileNames() {
  //TODO: verify this
  uint32_t i = 0;
  for (SpaceRecord *r : records_) {
    for (size_t j = 0; j < r->GetTileNum(); j++) {
      std::string s = r->GetRangeString(j);
      gcspy_driverSetTileName(driver_, i, s.c_str());
      i++;
    }
  }
}

void ContinuousSpaceDriver::VisitRoot(mirror::Object* obj,
                                      const RootInfo& root_info ATTRIBUTE_UNUSED) {
  for (SpaceRecord *r : records_) {
    if (r->AddrInRange((char *)obj)) {
      r->AddSingle((uintptr_t)obj, OFFSETOF_MEMBER(ContinuousSpaceTile, roots));
      ++totalRoots_;
      break;
    }
  }
}

void ContinuousSpaceDriver::Reset() {
  totalUsedSpace_ = 0;
  totalSpace_ = 0;
  totalRoots_ = 0;
  for (SpaceRecord *r : records_) {
    r->Reset();
  }
}

void ContinuousSpaceDriver::Send() {
  ContinuousSpaceTile *tile;
  int tileNum = driver_->tileNum;

  gcspy_driverStartComm(driver_);

  gcspy_driverStream(driver_, ContinuousSpaceDriver::USED_SPACE_STREAM, tileNum);
  for (SpaceRecord *record : records_) {
    for (size_t i = 0; i < record->GetTileNum(); ++i) {
      tile = (ContinuousSpaceTile *)record->GetTile(i);
      gcspy_driverStreamIntValue(driver_, tile->usedSpace);
    }
  }
  gcspy_driverStreamEnd(driver_);

  gcspy_driverSummary(driver_, ContinuousSpaceDriver::USED_SPACE_STREAM, 2);
  gcspy_driverSummaryValue(driver_, totalUsedSpace_);
  gcspy_driverSummaryValue(driver_, totalSpace_);
  gcspy_driverSummaryEnd(driver_);

  gcspy_driverStream(driver_, ContinuousSpaceDriver::ROOTS_STREAM, tileNum);
  for (SpaceRecord *record : records_) {
    for (size_t i = 0; i < record->GetTileNum(); ++i) {
      tile = (ContinuousSpaceTile *)record->GetTile(i);
      gcspy_driverStreamIntValue(driver_, tile->roots);
    }
  }
  gcspy_driverStreamEnd(driver_);

  gcspy_driverSummary(driver_, ContinuousSpaceDriver::ROOTS_STREAM, 1);
  gcspy_driverSummaryValue(driver_, totalRoots_);
  gcspy_driverSummaryEnd(driver_);

  int size = 0;
  for (SpaceRecord *record : records_) {
    size += record->GetSize();
  }
  char spaceInfo[128];
  sprintf(spaceInfo, "Current Size: %s\n", gcspy_formatSize(size));
  gcspy_driverSpaceInfo(driver_, spaceInfo);

  gcspy_driverEndComm(driver_);
}

class ImageSpaceDriver : public ContinuousSpaceDriver {
 public:
  explicit ImageSpaceDriver(gcspy_gc_driver_t *driver)
      : ContinuousSpaceDriver(driver, kImageSpace) {}

  void VisitRoot(mirror::Object* obj, const RootInfo& root_info ATTRIBUTE_UNUSED)
      OVERRIDE SHARED_REQUIRES(Locks::mutator_lock_) {
    for (SpaceRecord *r : records_) {
      if (r->AddrInRange((char *)obj)) {
        r->AddSingle((uintptr_t)obj, OFFSETOF_MEMBER(ContinuousSpaceTile, roots));
        ++totalRoots_;
        break;
      }
    }
  }

  void Update() SHARED_REQUIRES(Locks::mutator_lock_);
};

void ImageSpaceDriver::Update() {
  Reset();

  int tileNum = 0;
  for (SpaceRecord *r : records_) {
    tileNum += r->GetTileNum();
    totalSpace_ += r->GetSize();
    totalUsedSpace_ += r->GetSize();
    r->SetPerc(OFFSETOF_MEMBER(ContinuousSpaceTile, usedSpace));
  }
  Resize(tileNum);
  SetupTileNames();

  Runtime::Current()->VisitImageRoots(this);
}

class ZygoteSpaceDriver : public ContinuousSpaceDriver {
 public:
  explicit ZygoteSpaceDriver(gcspy_gc_driver_t *driver)
      : ContinuousSpaceDriver(driver, kZygoteSpace) {}
};

class NonMovingSpaceDriver : public ContinuousSpaceDriver {
 public:
  explicit NonMovingSpaceDriver(gcspy_gc_driver_t *driver)
      : ContinuousSpaceDriver(driver, kNonMovingSpace) {}
};

static void DlUsedSpaceCallback(void *start, void *end, size_t used_bytes, void *arg) {
  // start and end are null on last call
  if (start == 0 || end == 0)
    return;
  CHECK(arg != NULL);
  SpaceRecord *record = reinterpret_cast<SpaceRecord*>(arg);
  record->SetPercVal(reinterpret_cast<uintptr_t>(start),
                     reinterpret_cast<uintptr_t>(end),
                     OFFSETOF_MEMBER(ContinuousSpaceTile, usedSpace),
                     (int)(used_bytes+sizeof(size_t)));
  // when the chunk is full, used_bytes is 8 bytes larger than chunk_size,
  // something related to dlmalloc implementation?
  //size_t chunk_size = reinterpret_cast<uint8_t*>(end) - reinterpret_cast<uint8_t*>(start);
  //LOG(INFO) << StringPrintf("MallocSpace Chunk [%p, %p], %u of %u used.", start, end, (unsigned)used_bytes, (unsigned)chunk_size);
}

class DlMallocSpaceDriver : public ContinuousSpaceDriver {
 public:
  explicit DlMallocSpaceDriver(gcspy_gc_driver_t *driver)
      : ContinuousSpaceDriver(driver, kDlMallocSpace) {}

  void Update() SHARED_REQUIRES(Locks::mutator_lock_) {
    ContinuousSpaceDriver::Update();
    for (SpaceRecord *r : records_) {
      space::DlMallocSpace *dlspace = r->GetSpace()->AsDlMallocSpace();
      dlspace->Walk(DlUsedSpaceCallback, r);
      totalSpace_ += r->GetSize();
      totalUsedSpace_ += dlspace->GetBytesAllocated();
    }
  }
};

static void RosUsedSpaceCallback(void *start, void *end, size_t used_bytes, void *arg) {
  // start and end are null on last call
  if (start == 0 || end == 0)
    return;
  CHECK(arg != NULL);
  SpaceRecord *record = reinterpret_cast<SpaceRecord*>(arg);
  record->SetPercVal(reinterpret_cast<uintptr_t>(start),
                     reinterpret_cast<uintptr_t>(end),
                     OFFSETOF_MEMBER(ContinuousSpaceTile, usedSpace),
                     (int)used_bytes);
}

class RosAllocSpaceDriver : public ContinuousSpaceDriver {
 public:
  explicit RosAllocSpaceDriver(gcspy_gc_driver_t *driver)
      : ContinuousSpaceDriver(driver, kRosAllocSpace) {}
  void Update() SHARED_REQUIRES(Locks::mutator_lock_);
};

void RosAllocSpaceDriver::Update() {
  ContinuousSpaceDriver::Update();
  for (SpaceRecord *r : records_) {
    space::RosAllocSpace *space = r->GetSpace()->AsRosAllocSpace();
    space->Walk(RosUsedSpaceCallback, r);
    totalSpace_ += r->GetSize();
    totalUsedSpace_ += space->GetBytesAllocated();
  }
}

class BumpPointerSpaceDriver : public ContinuousSpaceDriver {
 public:
  explicit BumpPointerSpaceDriver(gcspy_gc_driver_t *driver)
      : ContinuousSpaceDriver(driver, kBumpPointerSpace) {}
  void Update() SHARED_REQUIRES(Locks::mutator_lock_);
};

//TODO: for TLAB, BumpPointerSpace has multiple blocks, need to handle it properly
void BumpPointerSpaceDriver::Update() {
  Reset();

  int tileNum = 0;
  for (SpaceRecord *r : records_) {
    CHECK(r->GetSpace()->IsBumpPointerSpace());
    space::BumpPointerSpace *space = r->GetSpace()->AsBumpPointerSpace();
    size_t capacity = space->Capacity();
    size_t allocated = space->GetBytesAllocated();
    totalSpace_ += capacity;
    totalUsedSpace_ += allocated;
    tileNum += r->GetTileNum();
    r->SetPerc(reinterpret_cast<intptr_t>(space->End()),
               OFFSETOF_MEMBER(ContinuousSpaceTile, usedSpace));
  }
  Resize(tileNum);
  SetupTileNames();

  //visit roots
  Runtime *runtime = Runtime::Current();
  if (Locks::mutator_lock_->IsExclusiveHeld(Thread::Current())) {
    runtime->VisitRoots(this);
  } else {
    runtime->VisitNonThreadRoots(this);
    runtime->VisitConcurrentRoots(this);
  }
}


// STUBS for DiscontinuousSpace
// Doing nothing but keep the spaces pointers.
class DiscontinuousSpaceDriver : public Driver {
 public:
  DiscontinuousSpaceDriver(gcspy_gc_driver_t *driver, DriverTypes type)
      : Driver(driver, type) {}

  ~DiscontinuousSpaceDriver() {}

  void Init(const char *name, const char *driverName, const char *title) {
    Driver::Init(name, driverName, title, 0, 0, false);
  }

  void Update() SHARED_REQUIRES(Locks::mutator_lock_) {}
  void Send() {}
  void AddSpace(space::Space *space) {
    if (space != nullptr) {
      spaces_.push_back(space);
    }
  }
  void RemoveSpace(space::Space *space) {
    spaces_.erase(std::remove_if(spaces_.begin(), spaces_.end(),
                                 [space] (space::Space *s) {
                                   if (s == space) return true;
                                   return false; }));
    if (spaces_.empty()) {
      delete this;
    }
  }

  virtual void VisitRoot(mirror::Object* obj ATTRIBUTE_UNUSED,
                         const RootInfo& root_info ATTRIBUTE_UNUSED)
      OVERRIDE SHARED_REQUIRES(Locks::mutator_lock_) {}
 private:
  std::vector<space::Space *> spaces_;
};

bool GcSpy::CreateServer(uint16_t port, CollectorType collector_type) {
  if (GcSpy::instance_ != nullptr) {
    return false;
  }

  GcSpy::instance_ = new GcSpy(port);

  char generalInfo[256];

  gcspy_date_time_t time = gcspy_getDateTime();
  strcpy(generalInfo, "GCspy For ART\n\n");
  strcat(generalInfo, "Start Time:\n  ");
  strcat(generalInfo, gcspy_formatDateTime(time));
  //TODO: get real application name
  //strcat(generalInfo, "\nApplication:\n  Dummy Driver Test\n\n");

  strcat(generalInfo, "Current GC algorithm:\n  ");
  switch (collector_type) {
    // only support CMS by now
    case kCollectorTypeCMS:
      strcat(generalInfo, "Concurrent-Mark-Sweep\n");
      break;
    case kCollectorTypeSS:
      strcat(generalInfo, "Semi-Sweep\n");
      break;
    case kCollectorTypeGSS:
      strcat(generalInfo, "Gen-Semi-Sweep\n");
      break;
    case kCollectorTypeCC:
      strcat(generalInfo, "Concurrent-Copy\n");
      break;
    default:
      strcat(generalInfo, "Unknown\n");
      return false;
  }
  instance_->SetGeneralInfo(generalInfo);

#ifdef GCSPY_WITH_TESTING_THREAD
  instance_->AddDriver(kDummy);
#endif

  // start gcspy main loop
  //TODO: new driver(s) added after client connected may cause problem.
  GcSpyMainLoop mainLoop;
  std::thread server(mainLoop);
  instance_->server_thread_ = std::move(server);

  // start dummy driver loop for test
#ifdef GCSPY_WITH_TESTING_THREAD
  GcSpyDummyLoop dummyLoop;
  std::thread dummy(dummyLoop);
  instance_->dummy_thread_ = std::move(dummy);
#endif

  return true;
}

void GcSpy::ShutdownServer() {
  CHECK(instance_ != nullptr);
  LOG(INFO) << "== Shutting down server\n";
  gcspy_main_server_t *server = &(instance_->server_);
  if (server->connected) {
    gcspy_mainServerShutdown(server);
  }
  instance_ = nullptr;
}


GcSpy::GcSpy(uint16_t port) : port_(port) {
  LOG(INFO) << "To start gcspy server\n";
  memset(&server_, 0, sizeof(server_));
  gcspy_mainServerInit(&server_, port_, MAX_SIZE, "GCSPY@ART", 1);

  //TODO: add more events

  LOG(INFO) << "--   Setting start event\n";
  gcspy_mainServerAddEvent(&server_, kStartGc, "GC Start");
  LOG(INFO) << "--   Setting finish event\n";
  gcspy_mainServerAddEvent(&server_, kFinishGc, "GC Finish");
}

GcSpy::~GcSpy() {
  for (Driver *d : drivers_) {
    delete d;
  }
}

Driver* GcSpy::CreateDriver(DriverTypes type, gcspy_gc_driver_t *driver) {
  Driver *res = nullptr;
  switch (type) {
    case kImageSpace:
      res = (Driver *)(new ImageSpaceDriver(driver));
      res->Init("ImageSpace", "Image Space Driver", "Block ");
      break;
    case kNonMovingSpace:
      res = (Driver *)(new NonMovingSpaceDriver(driver));
      res->Init("NonMovingSpace", "NonMoving Space Driver", "Block ");
      break;
    case kDlMallocSpace:
      res = (Driver *)(new DlMallocSpaceDriver(driver));
      res->Init("DlMallocSpace", "DlMalloc Space Driver", "Block ");
      break;
    case kRosAllocSpace:
      res = (Driver *)(new RosAllocSpaceDriver(driver));
      res->Init("RosAllocSpace", "RosAlloc Space Driver", "Block ");
      break;
    case kBumpPointerSpace:
      res = (Driver *)(new BumpPointerSpaceDriver(driver));
      res->Init("BumpPointerSpace", "Bump Pointer Space Driver", "Block ");
      break;
#ifdef GCSPY_WITH_TESTING_THREAD
    case kDummy:
      res = (Driver *)(new DummyDriver(driver));
      res->Init("Dummy Space", "Dummy Driver", "Block ");
      break;
#endif
    case kLargeObjectSpace:
      res = (Driver *)(new DiscontinuousSpaceDriver(driver, kLargeObjectSpace));
      res->Init("LargeObjectSpace", "Large Object Space Driver", "Block ");
      break;
    case kLargeObjectMapSpace:
      res = (Driver *)(new DiscontinuousSpaceDriver(driver, kLargeObjectMapSpace));
      res->Init("LargeObjectMapSpace", "Discontinuous Space Dirver", "Block ");
      break;
    case kFreeListSpace:
      res = (Driver *)(new DiscontinuousSpaceDriver(driver, kFreeListSpace));
      res->Init("FreeListSpace", "Free List Driver", "Block ");
      break;
    default:
      LOG(ERROR) << "Unimplemented Driver Type!";
      break;
  }
  return res;
}

bool GcSpy::AddSpace(space::Space *space) {
  CHECK(space != nullptr);
  CHECK(instance_ != nullptr);

  if (space->IsImageSpace()) {
    ImageSpaceDriver *driver = (ImageSpaceDriver *)GetDriver(kImageSpace);
    if (driver != nullptr) {
      driver->AddSpace(space);
      return true;
    }
    LOG(ERROR) << "ImageSpaceDriver isn't installed";
  } else if (space->IsZygoteSpace()) {
    ZygoteSpaceDriver *driver = (ZygoteSpaceDriver *)GetDriver(kZygoteSpace);
    if (driver != nullptr) {
      driver->AddSpace(space);
      return true;
    }
  } else if (space->IsRosAllocSpace()) {
    RosAllocSpaceDriver *driver = (RosAllocSpaceDriver *)GetDriver(kRosAllocSpace);
    if (driver != nullptr) {
      driver->AddSpace(space);
      return true;
    }
    LOG(ERROR) << "RosAllocSpaceDriver isn't installed";
  } else if (space->IsDlMallocSpace()) {
    DlMallocSpaceDriver *driver = (DlMallocSpaceDriver *)GetDriver(kDlMallocSpace);
    if (driver != nullptr) {
      driver->AddSpace(space);
      return true;
    }
  } else if (space->IsBumpPointerSpace()) {
    BumpPointerSpaceDriver *driver = (BumpPointerSpaceDriver *)GetDriver(kBumpPointerSpace);
    if (driver != nullptr) {
      driver->AddSpace(space);
      return true;
    }
    LOG(ERROR) << "BumpPointerSpaceDriver isn't installed";
  } else if (space->IsDiscontinuousSpace()) {
    //notify unsupported space but continue running
    LOG(INFO) << "DiscontinousSpaceDriver isn't installed";
  } else {
    LOG(ERROR) << "Unimplemented space for GcSpy! ";
  }
  return false;
}

bool GcSpy::RemoveSpace(space::Space *space) {
  if (space->IsImageSpace()) {
    ImageSpaceDriver *driver = (ImageSpaceDriver *)GetDriver(kImageSpace);
    if (driver != nullptr) {
      driver->RemoveSpace(space);
      return true;
    }
    LOG(ERROR) << "ImageSpaceDriver isn't installed";
  } else if (space->IsZygoteSpace()) {
    ZygoteSpaceDriver *driver = (ZygoteSpaceDriver *)GetDriver(kZygoteSpace);
    if (driver != nullptr) {
      driver->RemoveSpace(space);
      return true;
    }
    LOG(ERROR) << "ZygoteSpaceDriver isn't installed";
  } else if (space->IsRosAllocSpace()) {
    RosAllocSpaceDriver *driver = (RosAllocSpaceDriver *)GetDriver(kRosAllocSpace);
    if (driver != nullptr) {
      driver->RemoveSpace(space);
      return true;
    }
    LOG(ERROR) << "RosAllocSpaceDriver isn't installed";
  } else if (space->IsDlMallocSpace()) {
    DlMallocSpaceDriver *driver = (DlMallocSpaceDriver *)GetDriver(kDlMallocSpace);
    if (driver != nullptr) {
      driver->RemoveSpace(space);
      return true;
    }
    LOG(ERROR) << "DlMallocSpaceDriver isn't installed";
  } else if (space->IsBumpPointerSpace()) {
    BumpPointerSpaceDriver *driver = (BumpPointerSpaceDriver *)GetDriver(kBumpPointerSpace);
    if (driver != nullptr) {
      driver->RemoveSpace(space);
      return true;
    }
    LOG(ERROR) << "BumpPointerSpaceDriver isn't installed";
  } else if (space->IsDiscontinuousSpace()) {
    //notify unsupported space but continue running
    LOG(INFO) << "DiscontinousSpaceDriver isn't installed";
  } else {
    LOG(ERROR) << "Unimplemented space for GcSpy!";
  }
  return false;
}

void GcSpy::Update() {
  ScopedSuspendAll ssa(__FUNCTION__);
  for (Driver *d : drivers_) {
    //TODO: for some drivers/spaces we don't need mutator_lock?
    d->Update();
  }
}

void GcSpy::Send(int event) {
  if (gcspy_mainServerIsConnected(&server_, event)) {
    gcspy_mainServerStartCompensationTimer(&server_);
    Update();
    gcspy_mainServerStopCompensationTimer(&server_);
    LOG(INFO) << "GcSpy::Send";
    for (Driver *d : drivers_) {
      d->Send();
    }

    //TODO: move safe point to a place in runtime make it constantly
    //      been called so client can disconnect
    gcspy_mainServerSafepoint(&server_, event);
  }
}

Driver *GcSpy::GetDriver(DriverTypes type) {
  Driver *driver = nullptr;
  for (Driver *d : drivers_) {
    if (d->GetType() == type) {
      driver = d;
      break;
    }
  }

  // Add if not exsit
  if (driver == nullptr) {
    driver = AddDriver(type);
  }
  return driver;
}

Driver *GcSpy::AddDriver(DriverTypes type) {
  /*
   * TODO: gcspy main server is using a indexed array to manage
   * up to 10 drivers, CHANGE it to make drivers removable.
   */
  LOG(INFO) << "GcSpy::AddDriver of type " << type;
  gcspy_gc_driver_t *native_driver = gcspy_mainServerAddDriver(&server_);

  Driver *driver = GcSpy::CreateDriver(type, native_driver);
  if (driver != nullptr) {
    drivers_.push_back(driver);
  }

  return driver;
}

void GcSpy::RemoveDriver(DriverTypes type) {
  drivers_.erase(std::remove_if(drivers_.begin(), drivers_.end(),
                                [type, this] (Driver *d) {
                                  if (d->GetType() == type) {
                                    gcspy_mainServerRemoveDriver(&this->server_, d->GetId());
                                    delete d;
                                    return true;
                                  }
                                  return false; }), drivers_.end());
}


} // namespace gcspy
} // namespace gc
} // namespace art
