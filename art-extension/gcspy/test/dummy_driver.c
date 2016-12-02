/**
 **  dummy_driver.c
 **
 **  Driver instance for the EVM M&C collector
 **/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "gcspy_gc_stream.h"
#include "dummy_driver.h"

static dummy_driver_tile_t *
dummyDriverAllocateStats (int tileNum) {
  int len = tileNum * sizeof(dummy_driver_tile_t);
  dummy_driver_tile_t *tiles = (dummy_driver_tile_t *) malloc(len);
  if (tiles == NULL) {
    char buffer[256];
    sprintf(buffer, "M&C Driver: could not allocate %d bytes for tile data",
            len);
    gcspy_raise_error(buffer);
  }
  return tiles;
}

static void
dummyDriverSetupTileNames (dummy_driver_t *dummyDriver,
                           int from,
                           int to) {
  int i;
  char tmp[256];
  for (i = from; i < to; ++i) {
    gcspy_dUtilsRangeString(&(dummyDriver->area), i, tmp);
    gcspy_driverSetTileName(dummyDriver->driver, i, tmp);
  }
}

static dummy_driver_tile_t *
dummyDriverGetTile (dummy_driver_t *dummyDriver, int index) {
  return (dummy_driver_tile_t *)
      gcspy_d_utils_get_stats_struct(&(dummyDriver->area), index, 0);
}

static int
dummyDriverGetTileIndex (dummy_driver_t *dummyDriver, char *addr) {
  return gcspy_d_utils_get_index(&(dummyDriver->area), addr);
}

static char *
dummyDriverGetTileAddr (dummy_driver_t *dummyDriver, int index) {
  return gcspy_d_utils_get_addr(&(dummyDriver->area), index);
}

void
dummyDriverZero (dummy_driver_t *dummyDriver,
                 char *limit) {
  int i;
  dummy_driver_tile_t *tile;
  int totalSpace = limit - dummyDriver->area.start;

  if (limit != dummyDriver->area.end) {
    int tileNum = gcspy_dUtilsTileNum(dummyDriver->area.start,
                                      limit,
                                      dummyDriver->area.blockSize);
    dummyDriver->area.end = limit;
    dummyDriver->area.blockNum = tileNum;
    free(dummyDriver->area.stats);
    dummyDriver->area.stats = (char *) dummyDriverAllocateStats(tileNum);

    gcspy_driverResize(dummyDriver->driver, tileNum);
    dummyDriverSetupTileNames(dummyDriver, 0, tileNum);
  }

  for (i = 0; i < dummyDriver->area.blockNum; ++i) {
    tile = dummyDriverGetTile(dummyDriver, i);
    tile->usedSpace = 0;
    tile->cards = DUMMY_CARD_STATE_CLEAN;
    tile->roots = 0;
    tile->marking = 0;
  }

  dummyDriver->totalUsedSpace[0] = 0;
  dummyDriver->totalUsedSpace[1] = totalSpace;
  for (i = 0; i < 3; ++i) {
    dummyDriver->totalCardTable[i] = 0;
  }
  dummyDriver->totalRoots = 0;
  dummyDriver->totalMarking = 0;
}

void
dummyDriverSetEnd (dummy_driver_t *dummyDriver,
                   char *end) {
  dummyDriver->totalUsedSpace[0] += (end - dummyDriver->area.start);
  gcspy_dUtilsSetPerc(&(dummyDriver->area),
                      dummyDriver->area.start,
                      end,
                      offsetof(dummy_driver_tile_t, usedSpace));
}

void
dummyDriverCard (dummy_driver_t *dummyDriver,
                 char *start,
                 int size,
                 int state) {
  ++dummyDriver->totalCardTable[state];
  gcspy_dUtilsUpdateEnumDesc(&(dummyDriver->area),
                             start, start + size,
                             offsetof(dummy_driver_tile_t, cards),
                             state);
}

void
dummyDriverRoot (dummy_driver_t *dummyDriver,
                 char *start) {
  ++dummyDriver->totalRoots;
  gcspy_dUtilsAddSingle(&(dummyDriver->area), start,
                        offsetof(dummy_driver_tile_t, roots));
}

void
dummyDriverMarked (dummy_driver_t *dummyDriver,
                   char *start) {
  ++dummyDriver->totalMarking;
  gcspy_dUtilsAddSingle(&(dummyDriver->area), start,
                        offsetof(dummy_driver_tile_t, marking));
}

void
dummyDriverSend (dummy_driver_t *dummyDriver, unsigned event) {
  int i;
  dummy_driver_tile_t *tile;
  gcspy_gc_driver_t *driver = dummyDriver->driver;
  int tileNum = dummyDriver->area.blockNum;
  double perc;
  char tmp[128];
  int size;

  gcspy_driverStartComm(driver);

  gcspy_driverStream(driver, DUMMY_USED_SPACE_STREAM, tileNum);
  for (i = 0; i < tileNum; ++i) {
    tile = dummyDriverGetTile(dummyDriver, i);
    gcspy_driverStreamIntValue(driver, tile->usedSpace);
  }
  gcspy_driverStreamEnd(driver);

  gcspy_driverSummary(driver, DUMMY_USED_SPACE_STREAM, 2);
  gcspy_driverSummaryValue(driver, dummyDriver->totalUsedSpace[0]);
  gcspy_driverSummaryValue(driver, dummyDriver->totalUsedSpace[1]);
  gcspy_driverSummaryEnd(driver);

  gcspy_driverStream(driver, DUMMY_CARD_TABLE_STREAM, tileNum);
  for (i = 0; i < tileNum; ++i) {
    tile = dummyDriverGetTile(dummyDriver, i);
    gcspy_driverStreamByteValue(driver, tile->cards);
  }
  gcspy_driverStreamEnd(driver);

  gcspy_driverSummary(driver, DUMMY_CARD_TABLE_STREAM, 3);
  gcspy_driverSummaryValue(driver, dummyDriver->totalCardTable[0]);
  gcspy_driverSummaryValue(driver, dummyDriver->totalCardTable[1]);
  gcspy_driverSummaryValue(driver, dummyDriver->totalCardTable[2]);
  gcspy_driverSummaryEnd(driver);

  gcspy_driverStream(driver, DUMMY_ROOTS_STREAM, tileNum);
  for (i = 0; i < tileNum; ++i) {
    tile = dummyDriverGetTile(dummyDriver, i);
    gcspy_driverStreamShortValue(driver, tile->roots);
  }
  gcspy_driverStreamEnd(driver);

  gcspy_driverSummary(driver, DUMMY_ROOTS_STREAM, 1);
  gcspy_driverSummaryValue(driver, dummyDriver->totalRoots);
  gcspy_driverSummaryEnd(driver);

  gcspy_driverStream(driver, DUMMY_MARKING_STREAM, tileNum);
  for (i = 0; i < tileNum; ++i) {
    tile = dummyDriverGetTile(dummyDriver, i);
    gcspy_driverStreamIntValue(driver, tile->marking);
  }
  gcspy_driverStreamEnd(driver);

#if 0
  gcspy_driverSummary(driver, DUMMY_MARKING_STREAM, 0);
  /*  gcspy_driverSummaryValue(driver, dummyDriver->totalMarking); */
  gcspy_driverSummaryEnd(driver);
#endif //0



  size = dummyDriver->area.end - dummyDriver->area.start;
  sprintf(tmp, "Current Size: %s\n", gcspy_formatSize(size));
  gcspy_driverSpaceInfo(driver, tmp);

  gcspy_driverEndComm(driver);
}

void
dummyDriverInit (dummy_driver_t *dummyDriver,
                 gcspy_gc_driver_t *gcDriver,
                 const char *name,
                 unsigned blockSize,
                 char *start,
                 char *end) {
  /* int i; */
  char tmp[256];
  gcspy_gc_stream_t *stream;
  int tileNum = gcspy_dUtilsTileNum(start, end, blockSize);
  dummy_driver_tile_t *tiles = dummyDriverAllocateStats(tileNum);

  dummyDriver->driver = gcDriver;
  gcspy_dUtilsInit(&(dummyDriver->area),
                   start, end,
                   0, blockSize, tileNum,
                   (char *) tiles, sizeof(dummy_driver_tile_t));

  if (blockSize < 1024)
    sprintf(tmp, "Block Size: %d bytes\n", blockSize);
  else
    sprintf(tmp, "Block Size: %dK\n", (blockSize / 1024));
  gcspy_driverInit(gcDriver, -1, name, "M&C GC",
                   "Block ", tmp, tileNum, NULL, 1);
  dummyDriverSetupTileNames(dummyDriver, 0, tileNum);

  stream = gcspy_driverAddStream(gcDriver, DUMMY_USED_SPACE_STREAM);
  gcspy_streamInit(stream, DUMMY_USED_SPACE_STREAM,
                   GCSPY_GC_STREAM_INT_TYPE,
                   "Used Space",
                   0, blockSize,
                   0, 0,
                   "Used Space: ", " bytes",
                   GCSPY_GC_STREAM_PRESENTATION_PERCENT,
                   GCSPY_GC_STREAM_PAINT_STYLE_ZERO, 0,
                   gcspy_colorDBGetColorWithName("Red"));

  stream = gcspy_driverAddStream(gcDriver, DUMMY_CARD_TABLE_STREAM);
  gcspy_streamInit(stream, DUMMY_CARD_TABLE_STREAM,
                   GCSPY_GC_STREAM_BYTE_TYPE,
                   "Card Table",
                   0, 2,
                   2, 2,
                   "Card State: ", "",
                   GCSPY_GC_STREAM_PRESENTATION_ENUM,
                   GCSPY_GC_STREAM_PAINT_STYLE_PLAIN, 0,
                   gcspy_colorDBGetColorWithName("Off White"));
  gcspy_streamAddEnumName(stream, DUMMY_CARD_STATE_DIRTY, "DIRTY");
  gcspy_streamAddEnumName(stream, DUMMY_CARD_STATE_SUMMARISED, "SUMMARISED");
  gcspy_streamAddEnumName(stream, DUMMY_CARD_STATE_CLEAN, "CLEAN");

  stream = gcspy_driverAddStream(gcDriver, DUMMY_ROOTS_STREAM);
  gcspy_streamInit(stream, DUMMY_ROOTS_STREAM,
                   GCSPY_GC_STREAM_SHORT_TYPE,
                   "Roots",
                   0, gcspy_d_utils_roots_per_block(blockSize),
                   0, 0,
                   "Roots: ", "",
                   GCSPY_GC_STREAM_PRESENTATION_PLUS,
                   GCSPY_GC_STREAM_PAINT_STYLE_ZERO, 0,
                   gcspy_colorDBGetColorWithName("Green"));

  stream = gcspy_driverAddStream(gcDriver, DUMMY_MARKING_STREAM);
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
