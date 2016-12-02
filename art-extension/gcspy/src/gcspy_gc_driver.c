/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_gc_driver.c
 **
 **  Stuff shared by the GC drivers
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "gcspy_utils.h"
#include "gcspy_interpreter.h"
#include "gcspy_gc_driver.h"

#define gcspy_driver_get_tile_name(_driver, _tile)  \
  (_driver)->tileNames[(_tile)]

#define gcspy_driver_set_tile_name(_driver, _tile, _name) \
  do {                                                    \
    (_driver)->tileNames[(_tile)] = (_name);              \
  } while (0)

#define gcspy_driver_get_stream(_driver, _id)   \
  ( &((_driver)->streams[(_id)]) )

static void
gcspy_driverSetupControl (gcspy_gc_driver_t *driver) {
  uint32_t tileNum = driver->tileNum;
  if (driver->control != NULL)
    free(driver->control);
  driver->control = (char *) malloc(tileNum); // When is control freed?
  if (driver->control == NULL) {
    char buffer[256];
    sprintf(buffer, "GC Driver(%d): malloc failed for a buffer of size %d",
            driver->id, tileNum);
    gcspy_raise_error(buffer);
  }
}

static void
gcspy_driverSetupTileNames (gcspy_gc_driver_t *driver,
                            uint32_t oldTileNum) {
  uint32_t tileNum = driver->tileNum;
  char **oldTileNames = driver->tileNames;
  uint32_t size = tileNum * sizeof(char *);
  char **tileNames;
  gcspy_assert( (driver->tileNames != NULL) || (oldTileNum == 0) );

  if (oldTileNames != NULL) {
    for (uint32_t i = 0; i < oldTileNum; ++i) {
      if (oldTileNames[i] != NULL)
        free(oldTileNames[i]);
    }
    free(oldTileNames);
  }

  tileNames = (char **) malloc(size);
  if (tileNames == NULL) {
    char buffer[256];
    sprintf(buffer, "GC Driver(%d): malloc failed for a buffer of size %d",
            driver->id, size);
    gcspy_raise_error(buffer);
  }
  for (uint32_t i = 0; i < tileNum; ++i)
    tileNames[i] = NULL;
  driver->tileNames = tileNames;
}

void
gcspy_driverInit (gcspy_gc_driver_t *driver,
                  int id,
                  const char *name,
                  const char *driverName,
                  const char *title,
                  const char *blockInfo,
                  uint32_t tileNum,
                  const char *unusedString,
                  int mainSpace) {
  driver->changed = 0;
  if (id != -1)
    driver->id = id;
  /* otherwise, it is assumed the id has already been set up */
  if (strlen(name) > GCSPY_GC_DRIVER_NAME_LEN) {
    char buffer[256];
    sprintf(buffer,
            "GC Driver(%d): name length too long (length %d)",
            driver->id,
            (uint32_t)strlen(name));
    gcspy_raise_error(buffer);
  }
  strcpy(driver->name, name);
  if (strlen(driverName) > GCSPY_GC_DRIVER_NAME_LEN) {
    char buffer[256];
    sprintf(buffer,
            "GC Driver(%d): driver name length too long (length %d)",
            driver->id,
            (uint32_t)strlen(driverName));
    gcspy_raise_error(buffer);
  }
  strcpy(driver->driverName, driverName);
  if (strlen(blockInfo) > GCSPY_GC_DRIVER_BLOCK_INFO_LEN) {
    char buffer[256];
    sprintf(buffer,
            "GC Driver(%d): gen info length too long (length %d)",
            driver->id,
            (uint32_t)strlen(blockInfo));
    gcspy_raise_error(buffer);
  }
  strcpy(driver->blockInfo, blockInfo);
  driver->title = title;
  driver->tileNum = tileNum;
  if (unusedString != NULL) {
    if (strlen(unusedString) > GCSPY_GC_DRIVER_UNUSED_STRING_LEN) {
      char buffer[256];
      sprintf(buffer,
              "GC Driver(%d): unused string length too long (length %d)",
              driver->id,
              (uint32_t)strlen(unusedString));
      gcspy_raise_error(buffer);
    }
    strcpy(driver->unusedString, unusedString);
  } else {
    strcpy(driver->unusedString, GCSPY_GC_DRIVER_DEFAULT_UNUSED_STRING);
  }
  driver->streamNum = 0;
  driver->tileNames = NULL;
  gcspy_driverSetupTileNames(driver, 0);
  driver->control = NULL;
  gcspy_driverSetupControl(driver);
  driver->mainSpace = mainSpace;
}

void
gcspy_driverReset(gcspy_gc_driver_t *driver) {
  /*
    This is set by const char*, no need to free.
     if (driver->title) free(title);
  */
  if (driver->tileNames) {
    for (uint32_t i = 0; i < driver->tileNum; i++) {
      if (driver->tileNames[i] != NULL) free(driver->tileNames[i]);
    }
    free(driver->tileNames);
  }
  if (driver->control) free(driver->control);
}

#if 0
void
gcspy_driverSetTileName (gcspy_gc_driver_t *driver,
                         uint32_t tile,
                         const char *name) {
  char *tileName;
  if (strlen(name) > GCSPY_GC_DRIVER_TILE_NAME_LEN) {
    char buffer[256];
    sprintf(buffer,
            "GC Driver(%d): tile %d name length too long (length %d)",
            driver->id,
            tile,
            strlen(name));
    gcspy_raise_error(buffer);
  }
  tileName = gcspy_driver_get_tile_name(driver, tile);
  strcpy(tileName, name);
}
#endif //0

void
gcspy_driverSetTileName (gcspy_gc_driver_t *driver,
                         uint32_t tile,
                         const char *name) {
  int size = strlen(name) + 1;
  char *tileName = (char *) malloc(size);
  if (tileName == NULL) {
    char buffer[256];
    sprintf(buffer, "GC Driver(%d): malloc failed for a buffer of size %d",
            driver->id, size);
    gcspy_raise_error(buffer);
  }
  strcpy(tileName, name);
  if (gcspy_driver_get_tile_name(driver, tile) != NULL)
    free(gcspy_driver_get_tile_name(driver, tile));
  gcspy_driver_set_tile_name(driver, tile, tileName);
}

gcspy_gc_stream_t *
gcspy_driverAddStream (gcspy_gc_driver_t *driver,
                       uint32_t id) {
  gcspy_gc_stream_t *stream;
  if (driver->streamNum == GCSPY_GC_DRIVER_MAX_STREAMS) {
    char buffer[256];
    sprintf(buffer, "GC Driver(%d): max stream num reached (%d)",
            driver->id,
            GCSPY_GC_DRIVER_MAX_STREAMS);
    gcspy_raise_error(buffer);
  }
  if (driver->streamNum != id) {
    char buffer[256];
    sprintf(buffer,
            "GC Driver(%d): expected stream id (%d) different to calculacated (%d)",
            driver->id,
            id,
            driver->streamNum);
    gcspy_raise_error(buffer);
  }
  stream = gcspy_driver_get_stream(driver, driver->streamNum);
  ++driver->streamNum;
  return stream;
}

gcspy_gc_stream_t *
gcspy_driverGetStream (gcspy_gc_driver_t *driver,
                       uint32_t id) {
  return gcspy_driver_get_stream(driver, id);
}

/**
 **    Driver info format when serialised
 **    ----------------------------------
 **      id               short
 **      name             string
 **      driverName       string
 **      title            string
 **      blockInfo        string
 **      tileNum          int
 **      unusedString     string
 **      mainSpace        bool
 **      streamNum        short
 **      ( streamInfo )*
 **      ( tileName )*    string
 **    ----------------------------------
 **/

void
gcspy_driverSerialise (gcspy_gc_driver_t *driver,
                       gcspy_comm_client_t *client) {
  gcspy_buffered_output_t output;

  gcspy_clientBufferedOutputInit(client, &output);
  gcspy_driverSerialiseToOutput(driver, &output);
  gcspy_clientSendBuffered(client, &output);
}

void
gcspy_driverSerialiseToOutput (gcspy_gc_driver_t *driver,
                               gcspy_buffered_output_t *output) {
  gcspy_bufferedOutputWriteShort(output, (short) driver->id);
  gcspy_bufferedOutputWriteString(output, driver->name);
  gcspy_bufferedOutputWriteString(output, driver->driverName);
  gcspy_bufferedOutputWriteString(output, driver->title);
  gcspy_bufferedOutputWriteString(output, driver->blockInfo);
  gcspy_bufferedOutputWriteInt(output, driver->tileNum);
  gcspy_bufferedOutputWriteString(output, driver->unusedString);
  gcspy_bufferedOutputWriteBoolean(output, driver->mainSpace);
  gcspy_bufferedOutputWriteShort(output, (short) driver->streamNum);
  for (uint32_t i = 0; i < driver->streamNum; ++i)
    gcspy_streamSerialise(&driver->streams[i], output);
  for (uint32_t i = 0; i < driver->tileNum; ++i)
    gcspy_bufferedOutputWriteString(output,
                                    gcspy_driver_get_tile_name(driver, i));
}

void
gcspy_driverSetComm (gcspy_gc_driver_t *driver,
                     gcspy_comm_client_t *client,
                     gcspy_command_stream_t *interpreter) {
  driver->client = client;
  driver->interpreter = interpreter;
}

/*************************************************************************/

void
gcspy_driverInitOutput (gcspy_gc_driver_t *driver) {
  gcspy_buffered_output_t *output = &driver->output;

  gcspy_clientBufferedOutputInit(driver->client, output);
  gcspy_commandStreamSetBufferedOutput(driver->interpreter, output);
  gcspy_commandStreamStart(driver->interpreter);
}

void
gcspy_driverEndOutput (gcspy_gc_driver_t *driver) {
  gcspy_buffered_output_t *output =
      gcspy_command_stream_get_output(driver->interpreter);
  gcspy_commandStreamFinish(driver->interpreter);
  gcspy_clientSendBuffered(driver->client, output);
}



void
gcspy_driverResize (gcspy_gc_driver_t *driver, uint32_t tileNum) {
  uint32_t oldTileNum = driver->tileNum;
  driver->tileNum = tileNum;
  gcspy_driverSetupControl(driver);
  gcspy_driverSetupTileNames(driver, oldTileNum);
  driver->changed = 1;
}


void
gcspy_driverStartComm (gcspy_gc_driver_t *driver) {
  if (driver->changed) {
    gcspy_driverInitOutput(driver);
    gcspy_intWriteSpace(driver->interpreter, driver);
    gcspy_driverEndOutput(driver);
    driver->changed = 0;
  }
}

/**
 **    Data for stream
 **    -------------------------
 **      STREAM_CMD
 **      driver ID           byte
 **      stream ID           byte
 **      len                 int (or whetever the array len type is)
 **      <value 1>           byte/short/int
 **      <value 2>           byte/short/int
 **      ...
 **      <value len>         byte/short/int
 **/

void
gcspy_driverStream (gcspy_gc_driver_t *driver,
                    uint32_t id,
                    uint32_t len) {
  gcspy_driverInitOutput(driver);
  gcspy_intWriteStream(driver->interpreter, driver->id, id, len);
}

void
gcspy_driverStreamByteValue (gcspy_gc_driver_t *driver,
                             int val) {
  gcspy_buffered_output_t *output =
      gcspy_command_stream_get_output(driver->interpreter);
  gcspy_bufferedOutputWriteByte(output, (char) val);
}

void
gcspy_driverStreamShortValue (gcspy_gc_driver_t *driver,
                              int val) {
  gcspy_buffered_output_t *output =
      gcspy_command_stream_get_output(driver->interpreter);
  gcspy_bufferedOutputWriteShort(output, (short) val);
}

void
gcspy_driverStreamIntValue (gcspy_gc_driver_t *driver,
                            int val) {
  gcspy_buffered_output_t *output =
      gcspy_command_stream_get_output(driver->interpreter);
  gcspy_bufferedOutputWriteInt(output, val);
}

void
gcspy_driverStreamEnd (gcspy_gc_driver_t *driver) {
  gcspy_driverEndOutput(driver);
}


/**
 **    Data for summary
 **    -------------------------
 **      SUMMARY_CMD
 **      driver ID           byte
 **      stream ID           byte
 **      len                 int (or whetever the array len type is)
 **      <value 1>           int
 **      <value 2>           int
 **      ...
 **      <value len>         int
 **/

void
gcspy_driverSummary (gcspy_gc_driver_t *driver,
                     uint32_t id,
                     uint32_t len) {
  gcspy_driverInitOutput(driver);
  gcspy_intWriteSummary(driver->interpreter, driver->id, id, len);
}

void
gcspy_driverSummaryValue (gcspy_gc_driver_t *driver,
                          int val) {
  gcspy_buffered_output_t *output =
      gcspy_command_stream_get_output(driver->interpreter);
  gcspy_bufferedOutputWriteInt(output, val);
}

void
gcspy_driverSummaryEnd (gcspy_gc_driver_t *driver) {
  gcspy_driverEndOutput(driver);
}


/**
 **    Data for control
 **    -------------------------
 **      CONTROL_CMD
 **      driver ID           byte
 **      len                 int (or whetever the array len type is)
 **      <value 1>           byte
 **      <value 2>           byte
 **      ...
 **      <value len>         byte
 **/

int
gcspy_driverControlIsUsed (char val) {
  return (val & GCSPY_GC_DRIVER_CONTROL_USED) != 0;
}

int
gcspy_driverControlIsBackground (char val) {
  return (val & GCSPY_GC_DRIVER_CONTROL_BACKGROUND) != 0;
}

int
gcspy_driverControlIsUnused (char val) {
  return (val & GCSPY_GC_DRIVER_CONTROL_UNUSED) != 0;
}

int
gcspy_driverControlIsSeparator (char val) {
  return (val & GCSPY_GC_DRIVER_CONTROL_SEPARATOR) != 0;
}

void
gcspy_driverControl (gcspy_gc_driver_t *driver) {
  for (uint32_t i = 0; i < driver->tileNum; ++i) {
    driver->control[i] = GCSPY_GC_DRIVER_CONTROL_USED;
  }
}

void
gcspy_driverControlValues (gcspy_gc_driver_t *driver,
                           char tag,
                           int start,
                           int len) {
  for (int i = start; i < (start+len); ++i) {
    if (gcspy_driverControlIsBackground(tag) ||
        gcspy_driverControlIsUnused(tag)) {
      if (gcspy_driverControlIsUsed(driver->control[i]))
        driver->control[i] &= (~GCSPY_GC_DRIVER_CONTROL_USED);
    }

    driver->control[i] |= tag;
  }
}

void
gcspy_driverControlEnd (gcspy_gc_driver_t *driver) {
  gcspy_buffered_output_t *output;
  gcspy_driverInitOutput(driver);
  output = gcspy_command_stream_get_output(driver->interpreter);
  gcspy_intWriteControl(driver->interpreter, driver->id, driver->tileNum);
  for (uint32_t i = 0; i < driver->tileNum; ++i) {
    gcspy_bufferedOutputWriteByte(output, driver->control[i]);
  }
  gcspy_driverEndOutput(driver);
}

/**
 **    Data for space
 **    --------------
 **      SPACE_CMD
 **      space ID         byte
 **      space info       string
 **/

void
gcspy_driverSpaceInfo (gcspy_gc_driver_t *driver,
                       char *spaceInfo) {
  gcspy_driverInitOutput(driver);
  gcspy_intWriteSpaceInfo(driver->interpreter, driver->id,
                          spaceInfo);
  gcspy_driverEndOutput(driver);
}

/**
 **    Data for event
 **    --------------
 **      EVENT_CMD
 **      event ID         byte
 **/

void
gcspy_driverEvent (gcspy_gc_driver_t *driver,
                   uint32_t event) {
  gcspy_intWriteEvent(driver->interpreter, event);
}

void
gcspy_driverEndComm (gcspy_gc_driver_t *driver UNUSED_) {
  /* Just now a NOP */
}
