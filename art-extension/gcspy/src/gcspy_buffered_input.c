/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_buffered_input.c
 **
 **  Facilities for buffered input
 **/

#include <stdio.h>
#include <stdint.h>
#include "gcspy_buffered_input.h"

#define GCSPY_BUFFERED_LOW_8_MASK       ( (1 << 8) - 1 )

void
gcspy_bufferedInputInit(gcspy_buffered_input_t *input,
                        char *buffer,
                        int len) {
  input->buffer = buffer;
  input->len = len;
  input->i = 0;
}

int
gcspy_bufferedInputFinished(gcspy_buffered_input_t *input) {
  return input->i == input->len;
}

static void
gcspy_bufferedInputRaiseError(gcspy_buffered_input_t *input) {
  char buffer[256];
  sprintf(buffer, "End of buffer reached, buffer length = %d", input->len);
  gcspy_raise_error(buffer);
}

static short
gcspy_bufferedInputBaseRead(gcspy_buffered_input_t *input) {
  short res;
  res = (short) input->buffer[input->i++];
  if (res < 0)
    res += 256;
  return res;
}

int
gcspy_bufferedInputReadBoolean(gcspy_buffered_input_t *input) {
  return (int) gcspy_bufferedInputReadByte(input);
}

char
gcspy_bufferedInputReadByte(gcspy_buffered_input_t *input) {
  char res;

  if ((input->i + 1) > input->len)
    gcspy_bufferedInputRaiseError(input);

  res = (char) gcspy_bufferedInputBaseRead(input);

  return res;
}

unsigned char
gcspy_bufferedInputReadUByte(gcspy_buffered_input_t *input) {
  unsigned char res;

  if ((input->i + 1) > input->len)
    gcspy_bufferedInputRaiseError(input);

  res = (unsigned char) gcspy_bufferedInputBaseRead(input);

  return res;
}

short
gcspy_bufferedInputReadShort(gcspy_buffered_input_t *input) {
  short res;

  if ((input->i + 2) > input->len)
    gcspy_bufferedInputRaiseError(input);

  res = gcspy_bufferedInputBaseRead(input);
  res <<= 8;
  res |= gcspy_bufferedInputBaseRead(input);

  return res;
}

unsigned short
gcspy_bufferedInputReadUShort(gcspy_buffered_input_t *input) {
  unsigned short res;

  if ((input->i + 2) > input->len)
    gcspy_bufferedInputRaiseError(input);

  res = (unsigned short) gcspy_bufferedInputBaseRead(input);
  res <<= 8;
  res |= (unsigned short) gcspy_bufferedInputBaseRead(input);

  return res;
}

int
gcspy_bufferedInputReadInt(gcspy_buffered_input_t *input) {
  int res = 0;

  if ((input->i + 4) > input->len)
    gcspy_bufferedInputRaiseError(input);

  res = (int) gcspy_bufferedInputBaseRead(input);
  res <<= 8;
  res |= (int) gcspy_bufferedInputBaseRead(input);
  res <<= 8;
  res |= (int) gcspy_bufferedInputBaseRead(input);
  res <<= 8;
  res |= (int) gcspy_bufferedInputBaseRead(input);

  return res;
}

void
gcspy_bufferedInputReadString(gcspy_buffered_input_t *input, char *v) {
  char b;
  char *curr = v;

  b = gcspy_bufferedInputReadByte(input);
  while (b != 0) {
    *curr = b;
    ++curr;
    b = gcspy_bufferedInputReadByte(input);
  }
  *curr = '\0';
}

static int
gcspy_bufferedInputReadArrayLen (gcspy_buffered_input_t *input) {
  return (int) gcspy_bufferedInputReadInt(input);
}

/* I might also want to pass a max len parameter too */
void
gcspy_bufferedInputReadByteArray(gcspy_buffered_input_t *input,
                                 char *v,
                                 int *len) {
  int i, length;
  char *curr = v;

  length = gcspy_bufferedInputReadArrayLen(input);
  *len = length;
  for (i = 0; i < length; ++i) {
    *curr = gcspy_bufferedInputReadByte(input);
    ++curr;
  }
}

/* I might also want to pass a max len parameter too */
void
gcspy_bufferedInputReadUByteArray(gcspy_buffered_input_t *input,
                                  unsigned char *v,
                                  int *len) {
  int i, length;
  unsigned char *curr = v;

  length = gcspy_bufferedInputReadArrayLen(input);
  *len = length;
  for (i = 0; i < length; ++i) {
    *curr = gcspy_bufferedInputReadUByte(input);
    ++curr;
  }
}

/* I might also want to pass a max len parameter too */
void
gcspy_bufferedInputReadShortArray(gcspy_buffered_input_t *input,
                                  short *v,
                                  int *len) {
  int i, length;
  short *curr = v;

  length = gcspy_bufferedInputReadArrayLen(input);
  *len = length;
  for (i = 0; i < length; ++i) {
    *curr = gcspy_bufferedInputReadShort(input);
    ++curr;
  }
}

/* I might also want to pass a max len parameter too */
void
gcspy_bufferedInputReadUShortArray(gcspy_buffered_input_t *input,
                                   unsigned short *v,
                                   int *len) {
  int i, length;
  unsigned short *curr = v;

  length = gcspy_bufferedInputReadArrayLen(input);
  *len = length;
  for (i = 0; i < length; ++i) {
    *curr = gcspy_bufferedInputReadUShort(input);
    ++curr;
  }
}

/* I might also want to pass a max len parameter too */
void
gcspy_bufferedInputReadIntArray(gcspy_buffered_input_t *input,
                                int *v,
                                int *len) {
  int i, length;
  int *curr = v;

  length = gcspy_bufferedInputReadArrayLen(input);
  *len = length;
  for (i = 0; i < length; ++i) {
    *curr = gcspy_bufferedInputReadInt(input);
    ++curr;
  }
}

void
gcspy_bufferedInputReadColor(gcspy_buffered_input_t *input,
                             gcspy_color_t *color) {
  gcspy_color_set_red(color, gcspy_bufferedInputReadUByte(input));
  gcspy_color_set_green(color, gcspy_bufferedInputReadUByte(input));
  gcspy_color_set_blue(color, gcspy_bufferedInputReadUByte(input));
}

void
gcspy_bufferedInputClose(gcspy_buffered_input_t *input) {
  if (!gcspy_bufferedInputFinished(input))
    gcspy_raise_error("Buffered input not finished");
}
