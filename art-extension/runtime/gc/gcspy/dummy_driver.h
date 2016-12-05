/**
 ** Copyright (C) 2015 Intel Corporation.
 **
 ** See the file "Kent_license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/


#ifndef _DUMMY_DRIVER_H_
#define _DUMMY_DRIVER_H_

#ifdef GCSPY_WITH_TESTING_THREAD
namespace art {

namespace gc {

namespace gcspy {

struct DummyTile {
  int usedSpace;
  int cards;
  int roots;
  int marking;
};

class DummyDriver : public Driver {
 public:
  static constexpr int DUMMY_USED_SPACE_STREAM = 0;
  static constexpr int DUMMY_CARD_TABLE_STREAM = 1;
  static constexpr int DUMMY_ROOTS_STREAM      = 2;
  static constexpr int DUMMY_MARKING_STREAM    = 3;

  static constexpr int DUMMY_CARD_STATE_CLEAN      = 0;
  static constexpr int DUMMY_CARD_STATE_SUMMARISED = 1;
  static constexpr int DUMMY_CARD_STATE_DIRTY      = 2;

  static constexpr uintptr_t START =  4 * 1024 * 1024;
  static constexpr uintptr_t FIRST = 32 * 1024 * 1024;
  static constexpr uintptr_t ONE   = 31 * 1024 * 1024;
  static constexpr uintptr_t TWO   = 63 * 1024 * 1024;
  static constexpr uintptr_t END   = 64 * 1024 * 1024;

  static constexpr size_t kDefaultBlockSize = 32 * 1024;
  static constexpr size_t kCardSize = 1024;

  explicit DummyDriver(gcspy_gc_driver_t *driver) : Driver(driver, kDummy) {}
  virtual ~DummyDriver() {
    delete record_;
  }

  virtual void
  Init(const char *name, const char *driverName, const char *title) {
    size_t statsStructSize = sizeof(DummyTile);
    size_t blockSize = DummyDriver::kDefaultBlockSize;
    // CHECK(end > start)
    record_ = new SpaceRecord(DummyDriver::START, DummyDriver::END, blockSize, statsStructSize);
    int tileNum = record_->GetTileNum();

    Driver::Init(name, driverName, title, blockSize, tileNum, false);

    SetupTileNames(0, tileNum);

    gcspy_gc_stream_t *stream;

    stream = gcspy_driverAddStream(driver_, DUMMY_USED_SPACE_STREAM);
    gcspy_streamInit(stream, DUMMY_USED_SPACE_STREAM,
                     GCSPY_GC_STREAM_INT_TYPE,
                     "Used Space",
                     0, blockSize,
                     0, 0,
                     "Used Space: ", " bytes",
                     GCSPY_GC_STREAM_PRESENTATION_PERCENT,
                     GCSPY_GC_STREAM_PAINT_STYLE_ZERO, 0,
                     gcspy_colorDBGetColorWithName("Blue"));

    stream = gcspy_driverAddStream(driver_, DUMMY_CARD_TABLE_STREAM);
    gcspy_streamInit(stream, DUMMY_CARD_TABLE_STREAM,
                     GCSPY_GC_STREAM_BYTE_TYPE,
                     "Card Table",
                     0, 2,
                     2, 2,
                     "Card State: ", "",
                     GCSPY_GC_STREAM_PRESENTATION_ENUM,
                     GCSPY_GC_STREAM_PAINT_STYLE_PLAIN, 0,
                     gcspy_colorDBGetColorWithName("Dark Gray"));
    gcspy_streamAddEnumName(stream, DUMMY_CARD_STATE_CLEAN, "CLEAN");
    gcspy_streamAddEnumName(stream, DUMMY_CARD_STATE_SUMMARISED, "SUMMARISED");
    gcspy_streamAddEnumName(stream, DUMMY_CARD_STATE_DIRTY, "DIRTY");

    stream = gcspy_driverAddStream(driver_, DUMMY_ROOTS_STREAM);
    gcspy_streamInit(stream, DUMMY_ROOTS_STREAM,
                     GCSPY_GC_STREAM_SHORT_TYPE,
                     "Roots",
                     0, gcspy_d_utils_roots_per_block(blockSize),
                     0, 0,
                     "Roots: ", "",
                     GCSPY_GC_STREAM_PRESENTATION_PLUS,
                     GCSPY_GC_STREAM_PAINT_STYLE_ZERO, 0,
                     gcspy_colorDBGetColorWithName("Green"));

    stream = gcspy_driverAddStream(driver_, DUMMY_MARKING_STREAM);
    gcspy_streamInit(stream, DUMMY_MARKING_STREAM,
                     GCSPY_GC_STREAM_INT_TYPE,
                     "Marking",
                     0, gcspy_d_utils_objects_per_block(blockSize),
                     0, 0,
                     "Marked: ", " objects",
                     GCSPY_GC_STREAM_PRESENTATION_PLUS,
                     GCSPY_GC_STREAM_PAINT_STYLE_ZERO, 0,
                     gcspy_colorDBGetColorWithName("Yellow"));
  }

  void Update() {
    static int count = 0;
    uintptr_t limit;
    uintptr_t end;
    ++count;
    switch (count) {
      case 0:
        limit = FIRST;
        end = ONE;
        break;
      case 1:
        limit = END;
        end = TWO;
        count = -1;
        break;
    }

    Reset(limit);
    SetEnd(end);

    switch (count % 2) {
      case 0:
        Root(START + 8);
        Root(START + 256);
        Root(START + 2 * (kDefaultBlockSize) + 8);
        Root(START + 2 * (kDefaultBlockSize) + 32);
        break;
      case 1:
        Root(START + 160);
        Root(START + 2 * (kDefaultBlockSize) + 64);
        Root(START + 2 * (kDefaultBlockSize) + 512);
        break;
    }

    Card(START, kCardSize, DUMMY_CARD_STATE_DIRTY);
    Card(START + kCardSize, kCardSize, DUMMY_CARD_STATE_CLEAN);

    Card(ONE, kCardSize,  DUMMY_CARD_STATE_SUMMARISED);
    Card((ONE + kCardSize), kCardSize, DUMMY_CARD_STATE_CLEAN);
  }

  void Send() {
    DummyTile *tile;
    int tileNum = record_->GetTileNum();
    int size;

    gcspy_driverStartComm(driver_);

    gcspy_driverStream(driver_, DUMMY_USED_SPACE_STREAM, tileNum);
    for (int i = 0; i < tileNum; ++i) {
      tile = (DummyTile *)GetTile(i);
      gcspy_driverStreamIntValue(driver_, tile->usedSpace);
    }
    gcspy_driverStreamEnd(driver_);

    gcspy_driverSummary(driver_, DUMMY_USED_SPACE_STREAM, 2);
    gcspy_driverSummaryValue(driver_, totalUsedSpace_[0]);
    gcspy_driverSummaryValue(driver_, totalUsedSpace_[1]);
    gcspy_driverSummaryEnd(driver_);

    gcspy_driverStream(driver_, DUMMY_CARD_TABLE_STREAM, tileNum);
    for (int i = 0; i < tileNum; ++i) {
      tile = (DummyTile *)GetTile(i);
      gcspy_driverStreamByteValue(driver_, tile->cards);
    }
    gcspy_driverStreamEnd(driver_);

    gcspy_driverSummary(driver_, DUMMY_CARD_TABLE_STREAM, 3);
    gcspy_driverSummaryValue(driver_, totalCardTable_[0]);
    gcspy_driverSummaryValue(driver_, totalCardTable_[1]);
    gcspy_driverSummaryValue(driver_, totalCardTable_[2]);
    gcspy_driverSummaryEnd(driver_);

    gcspy_driverStream(driver_, DUMMY_ROOTS_STREAM, tileNum);
    for (int i = 0; i < tileNum; ++i) {
      tile = (DummyTile *)GetTile(i);
      gcspy_driverStreamShortValue(driver_, tile->roots);
    }
    gcspy_driverStreamEnd(driver_);

    gcspy_driverSummary(driver_, DUMMY_ROOTS_STREAM, 1);
    gcspy_driverSummaryValue(driver_, totalRoots_);
    gcspy_driverSummaryEnd(driver_);

    gcspy_driverStream(driver_, DUMMY_MARKING_STREAM, tileNum);
    for (int i = 0; i < tileNum; ++i) {
      tile = (DummyTile *)GetTile(i);
      gcspy_driverStreamIntValue(driver_, tile->marking);
    }
    gcspy_driverStreamEnd(driver_);

#if 0
    gcspy_driverSummary(driver_, DUMMY_MARKING_STREAM, 0);
    gcspy_driverSummaryValue(driver_, ->totalMarking);
    gcspy_driverSummaryEnd(driver_);
#endif //0

    size = record_->GetSize();
    char spaceInfo[128];
    sprintf(spaceInfo, "Current Size: %s\n", gcspy_formatSize(size));
    gcspy_driverSpaceInfo(driver_, spaceInfo);

    gcspy_driverEndComm(driver_);
  }

  void AddSpace(space::Space *space) {
    CHECK(space != nullptr);
  }
  void RemoveSpace(space::Space *space) {
    CHECK(space != nullptr);
  }

  void SetEnd(uintptr_t end) {
    totalUsedSpace_[0] += (end - record_->End());
    record_->SetPerc(end, OFFSETOF_MEMBER(DummyTile, usedSpace));
  }

  void Card(uintptr_t start, int size, int state) {
    ++totalCardTable_[state];
    record_->UpdateEnumDesc(start, start+size, OFFSETOF_MEMBER(DummyTile, cards), state);
  }

  void Root(uintptr_t start) {
    ++totalRoots_;
    record_->AddSingle(start, OFFSETOF_MEMBER(DummyTile, roots));
  }

  void Marked(uintptr_t start) {
    ++totalMarking_;
    record_->AddSingle(start, OFFSETOF_MEMBER(DummyTile, marking));
  }

  void Reset(uintptr_t limit) {
    if (record_->Reset(limit)) {
      int tileNum = record_->GetTileNum();
      gcspy_driverResize(driver_, tileNum);
      SetupTileNames(0, tileNum);
    }

    totalUsedSpace_[0] = 0;
    totalUsedSpace_[1] = limit - record_->Begin();
    for (unsigned i = 0; i < sizeof(totalCardTable_)/sizeof(totalCardTable_[0]); ++i) {
      totalCardTable_[i] = 0;
    }
    totalRoots_ = 0;
    totalMarking_ = 0;
  }

 private:
  void *GetTile(int index) {
    return record_->GetTile(index);
  }

  int GetTileIndex(char *addr) {
    return record_->GetTileIndex(addr);
  }

  char *GetTileAddr(int index) {
    return record_->GetTileAddr(index);
  }

  void SetupTileNames(int from, int to) {
    for (int i = from; i < to; i++) {
      std::string s = record_->GetRangeString(i);
      gcspy_driverSetTileName(driver_, i, s.c_str());
    }
  }

  void *AllocStats(size_t tileNum, size_t statsSize) {
    size_t len = tileNum * statsSize;
    void *tiles = malloc(len);
    if (tiles == nullptr) {
      LOG(ERROR) << "M&C Driver: could not allocate %d bytes for tile data";
    }
    return tiles;
  }


 private:
  SpaceRecord *record_;
  int totalUsedSpace_[2];
  int totalCardTable_[3];
  int totalRoots_;
  int totalMarking_;
};

class GcSpyDummyLoop {
 public:
  NO_RETURN void operator() () {
    gcspy_main_server_t *server = GcSpy::Current()->GetServer();
    DummyDriver *dummyDriver = (DummyDriver *)GcSpy::Current()->GetDriver(kDummy);
    GcSpyEvents event = kStartGc;
    int count = 0;
    uintptr_t limit;
    uintptr_t end;

    while (true) {
      gcspy_wait(1000);

      if (gcspy_mainServerIsConnected(server, event)) {

        gcspy_mainServerStartCompensationTimer(server);

        ++count;
        switch (count) {
          case 0:
            limit = DummyDriver::FIRST;
            end = DummyDriver::ONE;
            break;
          case 1:
            limit = DummyDriver::END;
            end = DummyDriver::TWO;
            count = -1;
            break;
        }

        dummyDriver->Reset(limit);
        dummyDriver->SetEnd(end);

        switch (count % 2) {
          case 0:
            dummyDriver->Root(DummyDriver::START + 8);
            dummyDriver->Root(DummyDriver::START + 256);
            dummyDriver->Root(DummyDriver::START + 2 * (DummyDriver::kDefaultBlockSize) + 8);
            dummyDriver->Root(DummyDriver::START + 2 * (DummyDriver::kDefaultBlockSize) + 32);
            break;
          case 1:
            dummyDriver->Root(DummyDriver::START + 160);
            dummyDriver->Root(DummyDriver::START + 2 * (DummyDriver::kDefaultBlockSize) + 64);
            dummyDriver->Root(DummyDriver::START + 2 * (DummyDriver::kDefaultBlockSize) + 512);
            break;
        }

        dummyDriver->Card(DummyDriver::START, DummyDriver::kCardSize,
                          dummyDriver->DUMMY_CARD_STATE_DIRTY);
        dummyDriver->Card(DummyDriver::START + DummyDriver::kCardSize, DummyDriver::kCardSize,
                          dummyDriver->DUMMY_CARD_STATE_CLEAN);

        dummyDriver->Card((DummyDriver::ONE), DummyDriver::kCardSize,
                          dummyDriver->DUMMY_CARD_STATE_SUMMARISED);
        dummyDriver->Card((DummyDriver::ONE + DummyDriver::kCardSize), DummyDriver::kCardSize,
                          dummyDriver->DUMMY_CARD_STATE_CLEAN);

        gcspy_mainServerStopCompensationTimer(server);

        if (gcspy_mainServerIsConnected(server, event)) {
          LOG(INFO) << "GCSPY CLIENT CONNECTED!";
          dummyDriver->Send();
        }
      }

      gcspy_mainServerSafepoint(server, event);
      if (event == kStartGc) {
        event = kFinishGc;
      } else {
        event = kStartGc;
      }
    }
  }
};

} // namespace gcspy
} // namespace gc
} // namespace art

#endif // GCSPY_WITH_TESTING_THREAD

#endif // _DUMMY_DRIVER_H_
