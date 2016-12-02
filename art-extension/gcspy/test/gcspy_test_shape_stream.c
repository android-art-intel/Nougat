/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_test_shape_stream.c
 **
 **  Tests the shape stream
 **/

#include <stdio.h>
#include "gcspy_shape_stream.h"

int main () {
  char buffer[32 * 1024];
  gcspy_command_stream_t stream;
  gcspy_buffered_output_t output;
  gcspy_buffered_input_t input;

  gcspy_shapeStreamInit(&stream);
  gcspy_bufferedOutputInit(&output, buffer, 32 * 1024);
  gcspy_commandStreamSetBufferedOutput(&stream, &output);

  gcspy_commandStreamStart(&stream);

  gcspy_shapeStreamRectangle(&stream, 2, 3, 10, 12);
  gcspy_shapeStreamCircle(&stream, 1, 2, 3);
  gcspy_shapeStreamPoint(&stream, 10, 12);
  gcspy_shapeStreamCircle(&stream, 3, 5, 2);
  gcspy_shapeStreamRectangle(&stream, -2, -3, 3, 4);
  gcspy_shapeStreamPoint(&stream, 1, 2);
  gcspy_shapeStreamPoint(&stream, 2, 3);
  gcspy_shapeStreamPoint(&stream, 4, 3);

  gcspy_commandStreamFinish(&stream);

  gcspy_bufferedInputInit(&input, buffer,
			  gcspy_bufferedOutputGetLen(&output));
  gcspy_commandStreamExecute(&stream, &input, NULL);
}
