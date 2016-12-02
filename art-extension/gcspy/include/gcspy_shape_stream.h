/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_shape_stream.h
 **
 **  Instantiation of the command stream
 **/

#ifndef _GCSPY_SHAPE_STREAM_H_

#define _GCSPY_SHAPE_STREAM_H_

#include "gcspy_command_stream.h"

void
gcspy_shapeStreamPoint(gcspy_command_stream_t *stream,
                       int x, int y);

void
gcspy_shapeStreamCircle(gcspy_command_stream_t *stream,
                        int x, int y, int rad);

void
gcspy_shapeStreamRectangle(gcspy_command_stream_t *stream,
                           int x1, int y1, int x2, int y2);

void
gcspy_shapeStreamInit(gcspy_command_stream_t *stream);

#endif //_GCSPY_SHAPE_STREAM_H_
