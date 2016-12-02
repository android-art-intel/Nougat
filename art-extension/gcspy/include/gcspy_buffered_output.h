/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_buffered_output.h
 **
 **  Facilities for buffered output
 **/

#ifndef _GCSPY_BUFFERED_OUTPUT_H_

#define _GCSPY_BUFFERED_OUTPUT_H_

#include "gcspy_utils.h"
#include "gcspy_color_db.h"

typedef struct {
  char *buffer;
  int i;
  int len;
} gcspy_buffered_output_t;

void
gcspy_bufferedOutputInit(gcspy_buffered_output_t *output,
                         char *buffer,
                         int len);

int
gcspy_bufferedOutputGetLen(gcspy_buffered_output_t *output);


void
gcspy_bufferedOutputWriteByte(gcspy_buffered_output_t *output, char v);

void
gcspy_bufferedOutputWriteBoolean(gcspy_buffered_output_t *output, int v);

void
gcspy_bufferedOutputWriteUByte(gcspy_buffered_output_t *output,
                               uint8_t v);

void
gcspy_bufferedOutputWriteShort(gcspy_buffered_output_t *output, short v);

void
gcspy_bufferedOutputWriteUShort(gcspy_buffered_output_t *output,
                                uint16_t v);

void
gcspy_bufferedOutputWriteInt(gcspy_buffered_output_t *output, int v);


void
gcspy_bufferedOutputWriteString(gcspy_buffered_output_t *output,
                                const char *v);

void
gcspy_bufferedOutputWriteArrayLen(gcspy_buffered_output_t *output,
                                  int len);

void
gcspy_bufferedOutputWriteByteArray(gcspy_buffered_output_t *output,
                                   char *v,
                                   int len);

void
gcspy_bufferedOutputWriteUByteArray(gcspy_buffered_output_t *output,
                                    uint8_t *v,
                                    int len);

void
gcspy_bufferedOutputWriteShortArray(gcspy_buffered_output_t *output,
                                    short *v,
                                    int len);

void
gcspy_bufferedOutputWriteUShortArray(gcspy_buffered_output_t *output,
                                     uint16_t *v,
                                     int len);

void
gcspy_bufferedOutputWriteIntArray(gcspy_buffered_output_t *output,
                                  int *v,
                                  int len);

void
gcspy_bufferedOutputWriteColor(gcspy_buffered_output_t *output,
                               gcspy_color_t *color);

#endif //_GCSPY_BUFFERED_OUTPUT_H_
