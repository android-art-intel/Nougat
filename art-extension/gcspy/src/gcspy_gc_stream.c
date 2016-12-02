/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_gc_stream.c
 **
 **  Stuff shared by the GC streams
 **/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "gcspy_utils.h"
#include "gcspy_gc_stream.h"

#define gcspy_stream_get_enum_name(_stream, _id)                        \
  ( &((_stream)->enumNames[(_id) * (GCSPY_GC_STREAM_ENUM_MAX_LEN + 1)]) )

void
gcspy_streamInit (gcspy_gc_stream_t *stream,
                  unsigned id,
                  unsigned dataType,
                  const char *name,
                  unsigned minValue,
                  unsigned maxValue,
                  unsigned zeroValue,
                  unsigned defaultValue,
                  const char *stringPre,
                  const char *stringPost,
                  unsigned presentation,
                  unsigned paintStyle,
                  unsigned maxStreamIndex,
                  gcspy_color_t *color) {
  stream->id = id;
  if (strlen(name) > GCSPY_GC_STREAM_NAME_LEN) {
    char buffer[256];
    sprintf(buffer,
            "GC Stream(%d): name length too long (length %u)",
            stream->id,
            (unsigned)strlen(name));
    gcspy_raise_error(buffer);
  }
  stream->dataType = dataType;
  strcpy(stream->name, name);
  stream->minValue = minValue;
  stream->maxValue = maxValue;
  stream->zeroValue = zeroValue;
  stream->defaultValue = defaultValue;
  stream->stringPre = stringPre;
  stream->stringPost = stringPost;
  stream->presentation = presentation;
  stream->paintStyle = paintStyle;
  stream->maxStreamIndex = maxStreamIndex;
  gcspy_color_copy(&stream->color, color);
  stream->enumNum = 0;
  stream->enumNames = NULL;
}

void
gcspy_streamAddEnumName (gcspy_gc_stream_t *stream,
                         unsigned id,
                         const char *name) {
  unsigned len;
  char *enumName;

  if (stream->enumNum != id) {
    char buffer[256];
    sprintf(buffer,
            "GC Stream(%d): allocated enum id (%d) different to given (%d)",
            stream->id,
            stream->enumNum,
            id);
    gcspy_raise_error(buffer);
  }

  if (stream->enumNum == 0) {
    len = GCSPY_GC_STREAM_ENUM_MAX_NUM * (GCSPY_GC_STREAM_ENUM_MAX_LEN+1);
    stream->enumNames = (char *) malloc(len);
    if (stream->enumNames == NULL) {
      char buffer[256];
      sprintf(buffer,
              "GC Stream(%d): malloc failed for a buffer of len %d",
              stream->id,
              len);
      gcspy_raise_error(buffer);
    }
  }

  if (stream->enumNum == GCSPY_GC_STREAM_ENUM_MAX_NUM) {
    char buffer[256];
    sprintf(buffer,
            "GC Stream(%d): max enum num reached (%d)",
            stream->id,
            GCSPY_GC_STREAM_ENUM_MAX_NUM);
    gcspy_raise_error(buffer);
  }
  if (strlen(name) > GCSPY_GC_STREAM_ENUM_MAX_LEN) {
    char buffer[256];
    sprintf(buffer,
            "GC Stream(%d): enum name length too long (length %u)",
            stream->id,
            (unsigned)strlen(name));
    gcspy_raise_error(buffer);
  }

  enumName = gcspy_stream_get_enum_name(stream, stream->enumNum);
  strcpy(enumName, name);
  ++stream->enumNum;
}

/**
 **    Stream info format when serialised
 **    ----------------------------------
 **      id                        byte
 **      dataType                  byte
 **      name                      string
 **      minValue                  int
 **      maxValue                  int
 **      zeroValue                 int
 **      defaultValue              int
 **      stringPre                 string
 **      stringPost                string
 **      presentation              byte
 **      paintStyle                byte
 **      maxStreamIndex            byte
 **      color                     color
 **      enum name num             byte
 **    ----------------------------------
 **      ( enum names )*           string
 **/

void
gcspy_streamSerialise (gcspy_gc_stream_t *stream,
                       gcspy_buffered_output_t *output) {
  unsigned i;
  char *name;
  gcspy_bufferedOutputWriteByte(output, (char) stream->id);
  gcspy_bufferedOutputWriteByte(output, (char) stream->dataType);
  gcspy_bufferedOutputWriteString(output, stream->name);
  gcspy_bufferedOutputWriteInt(output, stream->minValue);
  gcspy_bufferedOutputWriteInt(output, stream->maxValue);
  gcspy_bufferedOutputWriteInt(output, stream->zeroValue);
  gcspy_bufferedOutputWriteInt(output, stream->defaultValue);
  gcspy_bufferedOutputWriteString(output, stream->stringPre);
  gcspy_bufferedOutputWriteString(output, stream->stringPost);
  gcspy_bufferedOutputWriteByte(output, (char) stream->presentation);
  gcspy_bufferedOutputWriteByte(output, (char) stream->paintStyle);
  gcspy_bufferedOutputWriteByte(output, (char) stream->maxStreamIndex);
  gcspy_bufferedOutputWriteColor(output, &stream->color);
  gcspy_bufferedOutputWriteByte(output, (char) stream->enumNum);
  for (i = 0; i < stream->enumNum; ++i) {
    name = gcspy_stream_get_enum_name(stream, i);
    gcspy_bufferedOutputWriteString(output, name);
  }
}
