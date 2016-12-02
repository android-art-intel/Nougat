/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_command_stream.h
 **
 **  Facilities for buffering commands
 **/

#ifndef _GCSPY_COMMAND_STREAM_H_

#define _GCSPY_COMMAND_STREAM_H_

#include "gcspy_buffered_input.h"
#include "gcspy_buffered_output.h"

#define GCSPY_COMMAND_END_STREAM       ( (gcspy_command_tag_t) 0 )
#define GCSPY_COMMAND_FIRST_AVAILABLE  ( GCSPY_COMMAND_END_STREAM + 1 )

typedef void gcspy_command_t (gcspy_buffered_input_t *input,
                              void *aux);
typedef uint8_t gcspy_command_tag_t;

typedef struct {
  gcspy_command_t         **cmds;
  int                       max;
  gcspy_buffered_output_t *output;
} gcspy_command_stream_t;

#define gcspy_command_stream_get_output(_stream)    ( (_stream)->output )

void
gcspy_commandStreamInit(gcspy_command_stream_t *stream,
                        gcspy_command_t **cmds,
                        int len);

void
gcspy_commandStreamExecute(gcspy_command_stream_t *stream,
                           gcspy_buffered_input_t *input,
                           void *aux);

void
gcspy_commandStreamSetBufferedOutput(gcspy_command_stream_t *stream,
                                     gcspy_buffered_output_t *output);

void
gcspy_commandStreamStart(gcspy_command_stream_t *stream);

void
gcspy_commandStreamFinish(gcspy_command_stream_t *stream);

void
gcspy_commandStreamPutCmd(gcspy_command_stream_t *stream,
                          gcspy_command_tag_t tag);

gcspy_command_tag_t
gcspy_commandStreamGetCmd(gcspy_command_stream_t *stream,
                          gcspy_buffered_input_t *input);

#endif //_GCSPY_COMMAND_STREAM_H_
