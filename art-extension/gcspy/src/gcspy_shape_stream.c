/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_shape_stream.c
 **
 **  Instantiation of the command stream
 **/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "gcspy_shape_stream.h"

#define GCSPY_SHAPE_STREAM_POINT      ( GCSPY_COMMAND_FIRST_AVAILABLE )
#define GCSPY_SHAPE_STREAM_CIRCLE     ( GCSPY_COMMAND_FIRST_AVAILABLE + 1 )
#define GCSPY_SHAPE_STREAM_RECTANGLE  ( GCSPY_COMMAND_FIRST_AVAILABLE + 2 )
#define GCSPY_SHAPE_STREAM_LAST_CMD   ( GCSPY_SHAPE_STREAM_RECTANGLE )

void
gcspy_shapeStreamPoint(gcspy_command_stream_t *stream,
                       int x, int y) {
  gcspy_buffered_output_t *output = gcspy_command_stream_get_output(stream);
  gcspy_commandStreamPutCmd(stream, GCSPY_SHAPE_STREAM_POINT);
  gcspy_bufferedOutputWriteInt(output, x);
  gcspy_bufferedOutputWriteInt(output, y);
}

static void
gcspy_shapeStreamPointCommand(gcspy_buffered_input_t *input,
                              void *aux UNUSED_) {
  int x, y;
  x = gcspy_bufferedInputReadInt(input);
  y = gcspy_bufferedInputReadInt(input);
  gcspy_print_mesg("Point (%d, %d)\n", x, y);
}


void
gcspy_shapeStreamCircle(gcspy_command_stream_t *stream,
                        int x, int y, int rad) {
  gcspy_buffered_output_t *output = gcspy_command_stream_get_output(stream);
  gcspy_commandStreamPutCmd(stream, GCSPY_SHAPE_STREAM_CIRCLE);
  gcspy_bufferedOutputWriteInt(output, x);
  gcspy_bufferedOutputWriteInt(output, y);
  gcspy_bufferedOutputWriteInt(output, rad);
}

static void
gcspy_shapeStreamCircleCommand(gcspy_buffered_input_t *input,
                               void *aux UNUSED_) {
  int x, y, rad;
  x = gcspy_bufferedInputReadInt(input);
  y = gcspy_bufferedInputReadInt(input);
  rad = gcspy_bufferedInputReadInt(input);
  gcspy_print_mesg("Circle (%d, %d), %d\n", x, y, rad);
}


void
gcspy_shapeStreamRectangle(gcspy_command_stream_t *stream,
                           int x1, int y1, int x2, int y2) {
  gcspy_buffered_output_t *output = gcspy_command_stream_get_output(stream);
  gcspy_commandStreamPutCmd(stream, GCSPY_SHAPE_STREAM_RECTANGLE);
  gcspy_bufferedOutputWriteInt(output, x1);
  gcspy_bufferedOutputWriteInt(output, y1);
  gcspy_bufferedOutputWriteInt(output, x2);
  gcspy_bufferedOutputWriteInt(output, y2);
}

static void
gcspy_shapeStreamRectangleCommand(gcspy_buffered_input_t *input,
                                  void *aux UNUSED_) {
  int x1, y1, x2, y2;
  x1 = gcspy_bufferedInputReadInt(input);
  y1 = gcspy_bufferedInputReadInt(input);
  x2 = gcspy_bufferedInputReadInt(input);
  y2 = gcspy_bufferedInputReadInt(input);
  gcspy_print_mesg("Rectangle (%d, %d), (%d, %d)\n", x1, y1, x2, y2);
}

void
gcspy_shapeStreamInit(gcspy_command_stream_t *stream) {
  gcspy_command_t **cmds;
  cmds = (gcspy_command_t **)
         malloc(sizeof(gcspy_command_t *) * (GCSPY_SHAPE_STREAM_LAST_CMD+1));
  if (cmds == NULL)
    gcspy_raise_error("malloc failed for the commands of the shape stream");

  cmds[GCSPY_SHAPE_STREAM_POINT] = gcspy_shapeStreamPointCommand;
  cmds[GCSPY_SHAPE_STREAM_CIRCLE] = gcspy_shapeStreamCircleCommand;
  cmds[GCSPY_SHAPE_STREAM_RECTANGLE] = gcspy_shapeStreamRectangleCommand;

  gcspy_commandStreamInit(stream, cmds, GCSPY_SHAPE_STREAM_LAST_CMD+1);
}
