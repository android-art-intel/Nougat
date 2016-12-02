/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_gc_driver.h
 **
 **  Stuff shared by the GC drivers
 **/

#ifndef _GCSPY_GC_DRIVER_H_

#define _GCSPY_GC_DRIVER_H_

#include "gcspy_comm.h"
#include "gcspy_command_stream.h"
#include "gcspy_gc_stream.h"

#define GCSPY_GC_DRIVER_NAME_LEN                 40
#define GCSPY_GC_DRIVER_UNUSED_STRING_LEN        40
#define GCSPY_GC_DRIVER_TILE_NAME_LEN            32
#define GCSPY_GC_DRIVER_MAX_STREAMS              20
#define GCSPY_GC_DRIVER_BLOCK_INFO_LEN          128

#define GCSPY_GC_DRIVER_DEFAULT_UNUSED_STRING   "NOT USED"

#define GCSPY_GC_DRIVER_CONTROL_USED               1
#define GCSPY_GC_DRIVER_CONTROL_BACKGROUND         2
#define GCSPY_GC_DRIVER_CONTROL_UNUSED             4
#define GCSPY_GC_DRIVER_CONTROL_SEPARATOR          8
#define GCSPY_GC_DRIVER_CONTROL_LINK              16

typedef struct {
  uint32_t                 id;
  char                     name[GCSPY_GC_DRIVER_NAME_LEN + 1];
  char                     driverName[GCSPY_GC_DRIVER_NAME_LEN + 1];
  uint32_t                 tileNum;
  char                   **tileNames;
  uint32_t                 streamNum;
  gcspy_gc_stream_t        streams[GCSPY_GC_DRIVER_MAX_STREAMS];
  char                     unusedString[GCSPY_GC_DRIVER_UNUSED_STRING_LEN + 1];
  const char              *title;
  char                     blockInfo[GCSPY_GC_DRIVER_BLOCK_INFO_LEN + 1];
  int                      mainSpace;

  int                      changed;

  gcspy_comm_client_t     *client;
  gcspy_command_stream_t  *interpreter;

  gcspy_buffered_output_t  output;

  char                    *control;
} gcspy_gc_driver_t;

#define gcspy_driver_set_id(_driver, _id)       \
  do {                                          \
    (_driver)->id = (_id);                      \
  } while (0)
#define gcspy_driver_get_id(_driver)                  ( (_driver)->id )

void
gcspy_driverInit (gcspy_gc_driver_t *driver,
                  int id,
                  const char *name,
                  const char *driverName,
                  const char *title,
                  const char *blockInfo,
                  uint32_t tileNum,
                  const char *unusedString,
                  int mainSpace);

void gcspy_driverReset(gcspy_gc_driver_t *driver);

void
gcspy_driverSetTileName (gcspy_gc_driver_t *driver,
                         uint32_t tile,
                         const char *name);

gcspy_gc_stream_t *
gcspy_driverAddStream (gcspy_gc_driver_t *driver,
                       uint32_t id);

gcspy_gc_stream_t *
gcspy_driverGetStream (gcspy_gc_driver_t *driver,
                       uint32_t id);

void
gcspy_driverSerialise (gcspy_gc_driver_t *driver,
                       gcspy_comm_client_t *client);

void
gcspy_driverSerialiseToOutput (gcspy_gc_driver_t *driver,
                               gcspy_buffered_output_t *output);

void
gcspy_driverSetComm (gcspy_gc_driver_t *driver,
                     gcspy_comm_client_t *client,
                     gcspy_command_stream_t *interpreter);

void
gcspy_driverInitOutput (gcspy_gc_driver_t *driver);

void
gcspy_driverEndOutput (gcspy_gc_driver_t *driver);

void
gcspy_driverResize (gcspy_gc_driver_t *driver, uint32_t tileNum);

void
gcspy_driverStartComm (gcspy_gc_driver_t *driver);



void
gcspy_driverStream (gcspy_gc_driver_t *driver,
                    uint32_t id,
                    uint32_t len);

void
gcspy_driverStreamByteValue (gcspy_gc_driver_t *driver, int val);

void
gcspy_driverStreamShortValue (gcspy_gc_driver_t *driver, int val);

void
gcspy_driverStreamIntValue (gcspy_gc_driver_t *driver, int val);

void
gcspy_driverStreamEnd (gcspy_gc_driver_t *driver);



void
gcspy_driverSummary (gcspy_gc_driver_t *driver,
                     uint32_t id,
                     uint32_t len);

void
gcspy_driverSummaryValue (gcspy_gc_driver_t *driver, int val);

void
gcspy_driverSummaryEnd (gcspy_gc_driver_t *driver);


int
gcspy_driverControlIsUsed (char val);

int
gcspy_driverControlIsBackground (char val);

int
gcspy_driverControlIsUnused (char val);

int
gcspy_driverControlIsSeparator (char val);



void
gcspy_driverControl (gcspy_gc_driver_t *driver);

void
gcspy_driverControlValues (gcspy_gc_driver_t *driver,
                           char tag,
                           int start,
                           int len);

void
gcspy_driverControlEnd (gcspy_gc_driver_t *driver);

void
gcspy_driverSpaceInfo (gcspy_gc_driver_t *driver, char *spaceInfo);

void
gcspy_driverEvent (gcspy_gc_driver_t *driver, uint32_t event);

void
gcspy_driverRedraw (gcspy_gc_driver_t *driver);

void
gcspy_driverEndComm (gcspy_gc_driver_t *driver);

#endif //_GCSPY_GC_DRIVER_H_
