/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_gc_stream.h
 **
 **  Stuff shared by the GC streams
 **/

#ifndef _GCSPY_GC_STREAM_H_

#define _GCSPY_GC_STREAM_H_

#include "gcspy_color_db.h"
#include "gcspy_buffered_output.h"

#define GCSPY_GC_STREAM_PRESENTATION_PLAIN              0
#define GCSPY_GC_STREAM_PRESENTATION_PLUS               1
#define GCSPY_GC_STREAM_PRESENTATION_MAX_VAR            2
#define GCSPY_GC_STREAM_PRESENTATION_PERCENT            3
#define GCSPY_GC_STREAM_PRESENTATION_PERCENT_VAR        4
#define GCSPY_GC_STREAM_PRESENTATION_ENUM               5

#define GCSPY_GC_STREAM_PAINT_STYLE_PLAIN               0
#define GCSPY_GC_STREAM_PAINT_STYLE_ZERO                1

#define GCSPY_GC_STREAM_BYTE_TYPE                       0
#define GCSPY_GC_STREAM_SHORT_TYPE                      1
#define GCSPY_GC_STREAM_INT_TYPE                        2

#define GCSPY_GC_STREAM_NAME_LEN                       48
#define GCSPY_GC_STREAM_ENUM_MAX_LEN                   24
#define GCSPY_GC_STREAM_ENUM_MAX_NUM                   10

typedef struct {
  uint32_t                     id;
  int                          dataType;
  char                         name[GCSPY_GC_STREAM_NAME_LEN + 1];
  uint32_t                     minValue;
  uint32_t                     maxValue;
  uint32_t                     zeroValue;
  uint32_t                     defaultValue;
  const char                  *stringPre;
  const char                  *stringPost;
  uint32_t                     presentation;
  uint32_t                     paintStyle;
  uint32_t                     maxStreamIndex;
  gcspy_color_t                color;
  uint32_t                     enumNum;
  char                        *enumNames;
} gcspy_gc_stream_t;

void
gcspy_streamInit (gcspy_gc_stream_t *stream,
                  uint32_t id,
                  uint32_t dataType,
                  const char *name,
                  uint32_t minValue,
                  uint32_t maxValue,
                  uint32_t zeroValue,
                  uint32_t defaultValue,
                  const char *stringPre,
                  const char *stringPost,
                  uint32_t presentation,
                  uint32_t paintStyle,
                  uint32_t maxStreamIndex,
                  gcspy_color_t *color);

void
gcspy_streamAddEnumName (gcspy_gc_stream_t *stream,
                         uint32_t enumID,
                         const char *name);

void
gcspy_streamSerialise (gcspy_gc_stream_t *stream,
                       gcspy_buffered_output_t *output);

#endif //_GCSPY_GC_STREAM_H_
