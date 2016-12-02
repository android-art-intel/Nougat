/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_command_stream.c
 **
 **  Facilities for buffered commands
 **/

#include <stdio.h>
#include <stdint.h>
#include "gcspy_command_stream.h"

#define GCSPY_COMMAND_MAGIC_TAG_START  ( 666666 )
#define GCSPY_COMMAND_MAGIC_TAG_END    ( -666 )

void
gcspy_commandStreamInit(gcspy_command_stream_t *stream,
                        gcspy_command_t **cmds,
                        int len) {
  stream->cmds = cmds;
  stream->max = len - 1;
}

void
gcspy_commandStreamExecute(gcspy_command_stream_t *stream,
                           gcspy_buffered_input_t *input,
                           void *aux) {
  int magic;
  gcspy_command_tag_t cmd;

  magic = gcspy_bufferedInputReadInt(input);
  if (magic != GCSPY_COMMAND_MAGIC_TAG_START) {
    char buffer[256];
    sprintf(buffer, "Wrong first magic number: %d", magic);
    gcspy_raise_error(buffer);
  }

  cmd = gcspy_commandStreamGetCmd(stream, input);
  while (cmd != GCSPY_COMMAND_END_STREAM) {
    if (cmd > stream->max) {
      char buffer[256];
      sprintf(buffer, "Command out of bounds: %d", cmd);
      gcspy_raise_error(buffer);
    }
    stream->cmds[cmd](input, aux);
    cmd = gcspy_commandStreamGetCmd(stream, input);
  }

  magic = gcspy_bufferedInputReadInt(input);
  if (magic != GCSPY_COMMAND_MAGIC_TAG_END) {
    char buffer[256];
    sprintf(buffer, "Wrong second magic number: %d", magic);
    gcspy_raise_error(buffer);
  }
  gcspy_bufferedInputClose(input);
}

void
gcspy_commandStreamSetBufferedOutput(gcspy_command_stream_t *stream,
                                     gcspy_buffered_output_t *output) {
  stream->output = output;
}

void
gcspy_commandStreamStart(gcspy_command_stream_t *stream) {
  gcspy_bufferedOutputWriteInt(stream->output, GCSPY_COMMAND_MAGIC_TAG_START);
}

void
gcspy_commandStreamFinish(gcspy_command_stream_t *stream) {
  gcspy_commandStreamPutCmd(stream, GCSPY_COMMAND_END_STREAM);
  gcspy_bufferedOutputWriteInt(stream->output, GCSPY_COMMAND_MAGIC_TAG_END);
}

void
gcspy_commandStreamPutCmd(gcspy_command_stream_t *stream,
                          gcspy_command_tag_t tag) {
  gcspy_bufferedOutputWriteByte(stream->output, tag);
}

gcspy_command_tag_t
gcspy_commandStreamGetCmd(gcspy_command_stream_t *stream UNUSED_,
                          gcspy_buffered_input_t *input) {
  return gcspy_bufferedInputReadByte(input);
}
