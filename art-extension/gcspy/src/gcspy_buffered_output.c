/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_buffered_output.c
 **
 **  Facilities for buffered output
 **/

#include <stdio.h>
#include <stdint.h>
#include "gcspy_buffered_output.h"

#define GCSPY_BUFFERED_LOW_8_MASK       ( (1 << 8) - 1 )

void
gcspy_bufferedOutputInit(gcspy_buffered_output_t *output,
                         char *buffer,
                         int len) {
  output->buffer = buffer;
  output->len = len;
  output->i = 0;
}

int
gcspy_bufferedOutputGetLen(gcspy_buffered_output_t *output) {
  return output->i;
}

static void
gcspy_bufferedOutputRaiseError(gcspy_buffered_output_t *output) {
  char buffer[256];
  sprintf(buffer, "Buffer is full, buffer length = %d", output->len);
  gcspy_raise_error(buffer);
}

static void
gcspy_bufferedOutputBaseWrite(gcspy_buffered_output_t *output, short v) {
  if (v > 127)
    v -= 256;
  output->buffer[output->i++] = (char) v;
}

void
gcspy_bufferedOutputWriteBoolean(gcspy_buffered_output_t *output, int v) {
  if (v)
    gcspy_bufferedOutputWriteByte(output, (char) 1);
  else
    gcspy_bufferedOutputWriteByte(output, (char) 0);
}

void
gcspy_bufferedOutputWriteByte(gcspy_buffered_output_t *output, char v) {
  if ((output->i + 1) > output->len)
    gcspy_bufferedOutputRaiseError(output);

  gcspy_bufferedOutputBaseWrite(output, (short) v);
}

void
gcspy_bufferedOutputWriteUByte(gcspy_buffered_output_t *output,
                               unsigned char v) {
  if ((output->i + 1) > output->len)
    gcspy_bufferedOutputRaiseError(output);

  gcspy_bufferedOutputBaseWrite(output, (short) v);
}

void
gcspy_bufferedOutputWriteShort(gcspy_buffered_output_t *output, short v) {
  if ((output->i + 2) > output->len)
    gcspy_bufferedOutputRaiseError(output);

  gcspy_bufferedOutputBaseWrite(output, (v >> 8));
  gcspy_bufferedOutputBaseWrite(output, (v & GCSPY_BUFFERED_LOW_8_MASK));
}

void
gcspy_bufferedOutputWriteUShort(gcspy_buffered_output_t *output,
                                unsigned short v) {
  if ((output->i + 2) > output->len)
    gcspy_bufferedOutputRaiseError(output);

  gcspy_bufferedOutputBaseWrite(output, (v >> 8));
  gcspy_bufferedOutputBaseWrite(output, (v & GCSPY_BUFFERED_LOW_8_MASK));
}

void
gcspy_bufferedOutputWriteInt(gcspy_buffered_output_t *output, int v) {
  if ((output->i + 4) > output->len)
    gcspy_bufferedOutputRaiseError(output);

  gcspy_bufferedOutputBaseWrite(output, (short) (v >> 24));
  gcspy_bufferedOutputBaseWrite(output, (short)
                                ((v >> 16) & GCSPY_BUFFERED_LOW_8_MASK));
  gcspy_bufferedOutputBaseWrite(output, (short)
                                ((v >> 8) & GCSPY_BUFFERED_LOW_8_MASK));
  gcspy_bufferedOutputBaseWrite(output, (short)
                                (v & GCSPY_BUFFERED_LOW_8_MASK));
}


void
gcspy_bufferedOutputWriteString(gcspy_buffered_output_t *output,
                                const char *v) {
  const char *curr = v;
  if (v != NULL) {
    while (*curr != '\0') {
      gcspy_bufferedOutputWriteByte(output, *curr);
      ++curr;
    }
  }
  gcspy_bufferedOutputWriteByte(output, 0);
}


void
gcspy_bufferedOutputWriteArrayLen(gcspy_buffered_output_t *output,
                                  int len) {
  gcspy_bufferedOutputWriteInt(output, len);
}

void
gcspy_bufferedOutputWriteByteArray(gcspy_buffered_output_t *output,
                                   char *v,
                                   int len) {
  int i;
  char *curr = v;
  gcspy_bufferedOutputWriteArrayLen(output, len);
  for (i = 0; i < len; ++i) {
    gcspy_bufferedOutputWriteByte(output, *curr);
    ++curr;
  }
}

void
gcspy_bufferedOutputWriteUByteArray(gcspy_buffered_output_t *output,
                                    unsigned char *v,
                                    int len) {
  int i;
  unsigned char *curr = v;
  gcspy_bufferedOutputWriteArrayLen(output, len);
  for (i = 0; i < len; ++i) {
    gcspy_bufferedOutputWriteUByte(output, *curr);
    ++curr;
  }
}

void
gcspy_bufferedOutputWriteShortArray(gcspy_buffered_output_t *output,
                                    short *v,
                                    int len) {
  int i;
  short *curr = v;
  gcspy_bufferedOutputWriteArrayLen(output, len);
  for (i = 0; i < len; ++i) {
    gcspy_bufferedOutputWriteShort(output, *curr);
    ++curr;
  }
}

void
gcspy_bufferedOutputWriteUShortArray(gcspy_buffered_output_t *output,
                                     unsigned short *v,
                                     int len) {
  int i;
  unsigned short *curr = v;
  gcspy_bufferedOutputWriteArrayLen(output, len);
  for (i = 0; i < len; ++i) {
    gcspy_bufferedOutputWriteUShort(output, *curr);
    ++curr;
  }
}

void
gcspy_bufferedOutputWriteIntArray(gcspy_buffered_output_t *output,
                                  int *v,
                                  int len) {
  int i;
  int *curr = v;
  gcspy_bufferedOutputWriteArrayLen(output, len);
  for (i = 0; i < len; ++i) {
    gcspy_bufferedOutputWriteInt(output, *curr);
    ++curr;
  }
}

void
gcspy_bufferedOutputWriteColor(gcspy_buffered_output_t *output,
                               gcspy_color_t *color) {
  gcspy_bufferedOutputWriteUByte(output, gcspy_color_get_red(color));
  gcspy_bufferedOutputWriteUByte(output, gcspy_color_get_green(color));
  gcspy_bufferedOutputWriteUByte(output, gcspy_color_get_blue(color));
}
