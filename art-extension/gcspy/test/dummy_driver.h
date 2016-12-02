#ifndef _DUMMY_DRIVER_H_
#define _DUMMY_DRIVER_H_

#include "gcspy_gc_driver.h"
#include "gcspy_d_utils.h"

#define DUMMY_USED_SPACE_STREAM      0
#define DUMMY_CARD_TABLE_STREAM      1
#define DUMMY_ROOTS_STREAM           2
#define DUMMY_MARKING_STREAM         3

#define DUMMY_CARD_STATE_CLEAN          2
#define DUMMY_CARD_STATE_SUMMARISED     1
#define DUMMY_CARD_STATE_DIRTY          0

typedef struct {
  int usedSpace;
  int cards;
  int roots;
  int marking;
} dummy_driver_tile_t;

typedef struct {
  gcspy_gc_driver_t    *driver;
  gcspy_d_utils_area_t  area;
  int totalUsedSpace[2];
  int totalCardTable[3];
  int totalRoots;
  int totalMarking;
} dummy_driver_t;

void
dummyDriverInit (dummy_driver_t *dummyDriver,
                 gcspy_gc_driver_t *gcDriver,
                 const char *name,
                 unsigned tileSize,
                 char *start,
                 char *end);

void
dummyDriverZero (dummy_driver_t *dummyDriver,
                 char *limit);

#if 0
void
dummyDriverLimit (dummy_driver_t *dummyDriver,
                  char *limit);
#endif //0

void
dummyDriverSetEnd (dummy_driver_t *dummyDriver,
                   char *end);

void
dummyDriverCard (dummy_driver_t *dummyDriver,
                 char *start,
                 int size,
                 int state);

void
dummyDriverRoot (dummy_driver_t *dummyDriver,
                 char *start);

void
dummyDriverMarked (dummy_driver_t *dummyDriver,
                   char *start);

void
dummyDriverSend (dummy_driver_t *dummyDriver,
                 unsigned event);


#endif // _DUMMY_DRIVER_H_
