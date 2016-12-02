/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_interpreter.c
 **
 **  The interpreter communicating with the client
 **/

#include <stdio.h>
#include <stdlib.h>
#include "gcspy_interpreter.h"

#define GCSPY_INTERPRETER_PAUSE_REQ    ( GCSPY_COMMAND_FIRST_AVAILABLE )
#define GCSPY_INTERPRETER_PAUSE        ( GCSPY_COMMAND_FIRST_AVAILABLE +  1 )
#define GCSPY_INTERPRETER_RESTART      ( GCSPY_COMMAND_FIRST_AVAILABLE +  2 )
#define GCSPY_INTERPRETER_PLAY_ONE     ( GCSPY_COMMAND_FIRST_AVAILABLE +  3 )
#define GCSPY_INTERPRETER_SHUTDOWN_REQ ( GCSPY_COMMAND_FIRST_AVAILABLE +  4 )
#define GCSPY_INTERPRETER_SHUTDOWN     ( GCSPY_COMMAND_FIRST_AVAILABLE +  5 )
#define GCSPY_INTERPRETER_STREAM       ( GCSPY_COMMAND_FIRST_AVAILABLE +  6 )
#define GCSPY_INTERPRETER_EVENT        ( GCSPY_COMMAND_FIRST_AVAILABLE +  7 )
#define GCSPY_INTERPRETER_CONTROL      ( GCSPY_COMMAND_FIRST_AVAILABLE +  8 )
#define GCSPY_INTERPRETER_FILTER       ( GCSPY_COMMAND_FIRST_AVAILABLE +  9 )
#define GCSPY_INTERPRETER_EVENT_COUNT  ( GCSPY_COMMAND_FIRST_AVAILABLE + 10 )
#define GCSPY_INTERPRETER_SUMMARY      ( GCSPY_COMMAND_FIRST_AVAILABLE + 11 )
#define GCSPY_INTERPRETER_SPACE_INFO   ( GCSPY_COMMAND_FIRST_AVAILABLE + 12 )
#define GCSPY_INTERPRETER_SPACE        ( GCSPY_COMMAND_FIRST_AVAILABLE + 13 )

#define GCSPY_INTERPRETER_CMD_LEN      ( GCSPY_INTERPRETER_SPACE + 1)

/***** PAUSE REQ *****/

static void
gcspy_intPauseReqCommand (gcspy_buffered_input_t *input UNUSED_,
                          void *data) {
  gcspy_int_aux_t *aux = (gcspy_int_aux_t *) data;
  aux->pauseReq = 1;
}

void
gcspy_intWritePauseReq (gcspy_command_stream_t *stream UNUSED_) {
  gcspy_raise_error("write PAUSE REQ not implemented");
}

/***** PAUSE *****/

static void
gcspy_intPauseCommand (gcspy_buffered_input_t *input UNUSED_,
                       void *data UNUSED_) {
  gcspy_raise_error("PAUSE not implemented");
}

void
gcspy_intWritePause (gcspy_command_stream_t *stream) {
  gcspy_commandStreamPutCmd(stream, GCSPY_INTERPRETER_PAUSE);
}

/***** RESTART *****/

static void
gcspy_intRestartCommand (gcspy_buffered_input_t *input UNUSED_,
                         void *data) {
  gcspy_int_aux_t *aux = (gcspy_int_aux_t *) data;
  aux->restart = 1;
}

void
gcspy_intWriteRestart (gcspy_command_stream_t *stream UNUSED_) {
  gcspy_raise_error("write RESTART not implemented");
}

/***** PLAY ONE *****/

static void
gcspy_intPlayOneCommand (gcspy_buffered_input_t *input UNUSED_,
                         void *data) {
  gcspy_int_aux_t *aux = (gcspy_int_aux_t *) data;
  aux->playOne = 1;
}

void
gcspy_intWritePlayOne (gcspy_command_stream_t *stream UNUSED_) {
  gcspy_raise_error("write PLAY ONE not implemented");
}

/***** SHUTDOWN REQ *****/

static void
gcspy_intShutdownReqCommand (gcspy_buffered_input_t *input UNUSED_,
                             void *data) {
  gcspy_int_aux_t *aux = (gcspy_int_aux_t *) data;
  aux->shutdownReq = 1;
}

void
gcspy_intWriteShutdownReq (gcspy_command_stream_t *stream UNUSED_) {
  gcspy_raise_error("write SHUTDOWN REQ not implemented");
}

/***** SHUTDOWN *****/

static void
gcspy_intShutdownCommand (gcspy_buffered_input_t *input UNUSED_,
                          void *data UNUSED_) {
  gcspy_raise_error("SHUTDOWN not implemented");
}

void
gcspy_intWriteShutdown (gcspy_command_stream_t *stream) {
  gcspy_commandStreamPutCmd(stream, GCSPY_INTERPRETER_SHUTDOWN);
}

/***** STREAM *****/

static void
gcspy_intStreamCommand (gcspy_buffered_input_t *input UNUSED_,
                        void *data UNUSED_) {
  gcspy_raise_error("STREAM not implemented");
}

void
gcspy_intWriteStream (gcspy_command_stream_t *stream,
                      uint32_t driverID,
                      uint32_t streamID,
                      uint32_t len) {
  gcspy_buffered_output_t *output = gcspy_command_stream_get_output(stream);
  gcspy_commandStreamPutCmd(stream, GCSPY_INTERPRETER_STREAM);
  gcspy_bufferedOutputWriteByte(output, (char) driverID);
  gcspy_bufferedOutputWriteByte(output, (char) streamID);
  gcspy_bufferedOutputWriteArrayLen(output, len);
}

/***** EVENT *****/

static void
gcspy_intEventCommand (gcspy_buffered_input_t *input UNUSED_,
                       void *data UNUSED_) {
  gcspy_raise_error("EVENT not implemented");
}

void
gcspy_intWriteEvent (gcspy_command_stream_t *stream,
                     uint32_t event) {
  gcspy_buffered_output_t *output = gcspy_command_stream_get_output(stream);
  gcspy_commandStreamPutCmd(stream, GCSPY_INTERPRETER_EVENT);
  gcspy_bufferedOutputWriteByte(output, (char) event);
}

/***** CONTROL *****/

static void
gcspy_intControlCommand (gcspy_buffered_input_t *input UNUSED_,
                         void *data UNUSED_) {
  gcspy_raise_error("CONTROL not implemented");
}

void
gcspy_intWriteControl (gcspy_command_stream_t *stream,
                       uint32_t driverID,
                       uint32_t len) {
  gcspy_buffered_output_t *output = gcspy_command_stream_get_output(stream);
  gcspy_commandStreamPutCmd(stream, GCSPY_INTERPRETER_CONTROL);
  gcspy_bufferedOutputWriteByte(output, (char) driverID);
  gcspy_bufferedOutputWriteArrayLen(output, len);
}

/***** FILTER *****/

static void
gcspy_intFilterCommand (gcspy_buffered_input_t *input,
                        void *data) {
  gcspy_int_aux_t *aux = (gcspy_int_aux_t *) data;
  int i;
  int len = (int) gcspy_bufferedInputReadShort(input);
  aux->filter = len;
  for (i = 0; i < len; ++i) {
    aux->enabled[i] = gcspy_bufferedInputReadBoolean(input);
    aux->delays[i] = gcspy_bufferedInputReadInt(input);
    aux->paused[i] = gcspy_bufferedInputReadBoolean(input);
    aux->skip[i] = gcspy_bufferedInputReadInt(input);
  }
}

void
gcspy_intWriteFilter (gcspy_command_stream_t *stream UNUSED_,
                      uint32_t driverID UNUSED_,
                      uint32_t len UNUSED_) {
  gcspy_raise_error("write FILTER not implemented");
}

/***** EVENT COUNT *****/

static void
gcspy_intEventCountCommand (gcspy_buffered_input_t *input UNUSED_,
                            void *data UNUSED_) {
  gcspy_raise_error("EVENT COUNT not implemented");
}

void
gcspy_intWriteEventCount (gcspy_command_stream_t *stream) {
  gcspy_commandStreamPutCmd(stream, GCSPY_INTERPRETER_EVENT_COUNT);
}

/***** SUMMARY *****/

static void
gcspy_intSummaryCommand (gcspy_buffered_input_t *input UNUSED_,
                         void *data UNUSED_) {
  gcspy_raise_error("SUMMARY not implemented");
}

void
gcspy_intWriteSummary (gcspy_command_stream_t *stream,
                       uint32_t driverID,
                       uint32_t streamID,
                       uint32_t len) {
  gcspy_buffered_output_t *output = gcspy_command_stream_get_output(stream);
  gcspy_commandStreamPutCmd(stream, GCSPY_INTERPRETER_SUMMARY);
  gcspy_bufferedOutputWriteByte(output, (char) driverID);
  gcspy_bufferedOutputWriteByte(output, (char) streamID);
  gcspy_bufferedOutputWriteArrayLen(output, len);
}

/***** SPACE INFO *****/

static void
gcspy_intSpaceInfoCommand (gcspy_buffered_input_t *input UNUSED_,
                           void *data UNUSED_) {
  gcspy_raise_error("SPACE INFO not implemented");
}

void
gcspy_intWriteSpaceInfo (gcspy_command_stream_t *stream,
                         uint32_t driverID,
                         char *spaceInfo) {
  gcspy_buffered_output_t *output = gcspy_command_stream_get_output(stream);
  gcspy_commandStreamPutCmd(stream, GCSPY_INTERPRETER_SPACE_INFO);
  gcspy_bufferedOutputWriteByte(output, (char) driverID);
  gcspy_bufferedOutputWriteString(output, spaceInfo);
}

/***** SPACE *****/

static void
gcspy_intSpaceCommand (gcspy_buffered_input_t *input UNUSED_,
                       void *data UNUSED_) {
  gcspy_raise_error("SPACE not implemented");
}

void
gcspy_intWriteSpace (gcspy_command_stream_t *stream,
                     gcspy_gc_driver_t *driver) {
  gcspy_buffered_output_t *output = gcspy_command_stream_get_output(stream);
  gcspy_commandStreamPutCmd(stream, GCSPY_INTERPRETER_SPACE);
  gcspy_driverSerialiseToOutput(driver, output);
}

/*****/

void
gcspy_intAuxInit (gcspy_int_aux_t *aux) {
  aux->pauseReq = 0;
  aux->restart = 0;
  aux->playOne = 0;
  aux->shutdownReq = 0;
  aux->filter = 0;
}

void
gcspy_intInit (gcspy_command_stream_t *stream) {
  gcspy_command_t **cmds;
  cmds = (gcspy_command_t **)
         malloc(sizeof(gcspy_command_t *) * GCSPY_INTERPRETER_CMD_LEN);
  if (cmds == NULL)
    gcspy_raise_error("malloc failed for the commands of the interpreter");

  cmds[GCSPY_INTERPRETER_PAUSE_REQ] = gcspy_intPauseReqCommand;
  cmds[GCSPY_INTERPRETER_PAUSE] = gcspy_intPauseCommand;
  cmds[GCSPY_INTERPRETER_RESTART] = gcspy_intRestartCommand;
  cmds[GCSPY_INTERPRETER_PLAY_ONE] = gcspy_intPlayOneCommand;
  cmds[GCSPY_INTERPRETER_SHUTDOWN_REQ] = gcspy_intShutdownReqCommand;
  cmds[GCSPY_INTERPRETER_SHUTDOWN] = gcspy_intShutdownCommand;

  cmds[GCSPY_INTERPRETER_STREAM] = gcspy_intStreamCommand;
  cmds[GCSPY_INTERPRETER_EVENT] = gcspy_intEventCommand;
  cmds[GCSPY_INTERPRETER_CONTROL] = gcspy_intControlCommand;
  cmds[GCSPY_INTERPRETER_FILTER] = gcspy_intFilterCommand;
  cmds[GCSPY_INTERPRETER_EVENT_COUNT] = gcspy_intEventCountCommand;
  cmds[GCSPY_INTERPRETER_SUMMARY] = gcspy_intSummaryCommand;
  cmds[GCSPY_INTERPRETER_SPACE_INFO] = gcspy_intSpaceInfoCommand;
  cmds[GCSPY_INTERPRETER_SPACE] = gcspy_intSpaceCommand;

  gcspy_commandStreamInit(stream, cmds, GCSPY_INTERPRETER_CMD_LEN);
}
