/**
 ** Copyright (C) 2015 Intel Corporation.
 **
 ** See the file "Kent_license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

#ifndef _ART_GCSPY_H_
#define _ART_GCSPY_H_

#include <thread>
#include <vector>

#include "base/macros.h"
#include "gc/collector_type.h"

extern "C" {
#include "gcspy_main_server.h"
}

namespace art {

namespace gc {

namespace space {
class Space;
} // namespace space

namespace gcspy {

enum DriverTypes {
  kBumpPointerSpace,
  kDlMallocSpace,
  kImageSpace,
  kNonMovingSpace,
  kRegionSpace,
  kRosAllocSpace,
  kZygoteSpace,
  kLargeObjectSpace,
  kLargeObjectMapSpace,
  kFreeListSpace,
#ifdef GCSPY_WITH_TESTING_THREAD
  kDummy, // for testing
#endif
  kDriverTypes
};

// need to support multiple GC algorithms
enum GcSpyEvents {
  kStartGc,
  kFinishGc,
};

class SpaceRecord;
class Driver;

class GcSpyMainLoop;
// GCSpy adaptor
class GcSpy {
 public:

  static bool CreateServer(uint16_t port, CollectorType collector_type);
  static void ShutdownServer();
  static GcSpy* Current() { return instance_; }

  bool AddSpace(space::Space *space);
  bool RemoveSpace(space::Space *space);
  void Update();
  void Send(int event);

 private:

  explicit GcSpy(uint16_t port);
  ~GcSpy();

  Driver *GetDriver(DriverTypes type);

  Driver *AddDriver(DriverTypes type);

  void RemoveDriver(DriverTypes type);

  void SetGeneralInfo(char *info) {
    gcspy_mainServerSetGeneralInfo(&server_, info);
  }

  static Driver* CreateDriver(DriverTypes type, gcspy_gc_driver_t *driver);

  static constexpr size_t MAX_SIZE = 1 * 1024 * 1024;

  static GcSpy* instance_;

  uint16_t port_;
  std::thread server_thread_;

#ifdef GCSPY_WITH_TESTING_THREAD
  // for experimental dummy driver, will be delete later
  std::thread dummy_thread_;
#endif

  gcspy_main_server_t server_;
  std::vector<Driver *> drivers_;

  friend class GcSpyMainLoop;

  DISALLOW_COPY_AND_ASSIGN(GcSpy);
};

} // namespace gcspy
} // namespace gc
} // namespace art

#endif // _ART_GCSPY_H_
