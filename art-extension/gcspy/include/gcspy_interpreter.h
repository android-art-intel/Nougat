/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_interpreter.h
 **
 **  The interpreter communicating with the client
 **/

#ifndef _GCSPY_INTERPRETER_H_

#define _GCSPY_INTERPRETER_H_

#include "gcspy_gc_driver.h"
#include "gcspy_command_stream.h"

#define GCSPY_INT_MAX_EVENTS  20

typedef struct {
  int pauseReq;
  int restart;
  int playOne;
  int shutdownReq;

  int filter;
  char enabled[GCSPY_INT_MAX_EVENTS];
  int delays[GCSPY_INT_MAX_EVENTS];
  char paused[GCSPY_INT_MAX_EVENTS];
  int skip[GCSPY_INT_MAX_EVENTS];
} gcspy_int_aux_t;



void
gcspy_intWritePauseReq (gcspy_command_stream_t *stream);

void
gcspy_intWritePause (gcspy_command_stream_t *stream);

void
gcspy_intWriteRestart (gcspy_command_stream_t *stream);

void
gcspy_intWritePlayOne (gcspy_command_stream_t *stream);

void
gcspy_intWriteShutdownReq (gcspy_command_stream_t *stream);

void
gcspy_intWriteShutdown (gcspy_command_stream_t *stream);

void
gcspy_intWriteStream (gcspy_command_stream_t *stream,
                      uint32_t driverID,
                      uint32_t streamID,
                      uint32_t len);

void
gcspy_intWriteEvent (gcspy_command_stream_t *stream,
                     uint32_t id);

void
gcspy_intWriteControl (gcspy_command_stream_t *stream,
                       uint32_t driverID,
                       uint32_t len);

void
gcspy_intWriteFilter (gcspy_command_stream_t *stream,
                      uint32_t driverID,
                      uint32_t len);

void
gcspy_intWriteEventCount (gcspy_command_stream_t *stream);

void
gcspy_intWriteSummary (gcspy_command_stream_t *stream,
                       uint32_t driverID,
                       uint32_t streamID,
                       uint32_t len);

void
gcspy_intWriteSpaceInfo (gcspy_command_stream_t *stream,
                         uint32_t driverID,
                         char *spaceInfo);

void
gcspy_intWriteSpace (gcspy_command_stream_t *stream,
                     gcspy_gc_driver_t *driver);



void
gcspy_intInit (gcspy_command_stream_t *stream);

void
gcspy_intAuxInit (gcspy_int_aux_t *aux);

#endif //_GCSPY_INTERPRETER_H_
