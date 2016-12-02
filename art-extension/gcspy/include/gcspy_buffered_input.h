/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_buffered_input.h
 **
 **  Facilities for buffered input
 **/

#ifndef _GCSPY_BUFFERED_INPUT_H_

#define _GCSPY_BUFFERED_INPUT_H_

#include "gcspy_utils.h"
#include "gcspy_color_db.h"

typedef struct {
  char *buffer;
  int i;
  int len;
} gcspy_buffered_input_t;

void
gcspy_bufferedInputInit(gcspy_buffered_input_t *input,
                        char *buffer,
                        int len);

int
gcspy_bufferedInputFinished(gcspy_buffered_input_t *input);


int
gcspy_bufferedInputReadBoolean(gcspy_buffered_input_t *input);

char
gcspy_bufferedInputReadByte(gcspy_buffered_input_t *input);

uint8_t
gcspy_bufferedInputReadUByte(gcspy_buffered_input_t *input);

short
gcspy_bufferedInputReadShort(gcspy_buffered_input_t *input);

uint16_t
gcspy_bufferedInputReadUShort(gcspy_buffered_input_t *input);

int
gcspy_bufferedInputReadInt(gcspy_buffered_input_t *input);


void
gcspy_bufferedInputReadString(gcspy_buffered_input_t *input, char *v);


void
gcspy_bufferedInputReadByteArray(gcspy_buffered_input_t *input,
                                 char *v,
                                 int *len);

void
gcspy_bufferedInputReadUByteArray(gcspy_buffered_input_t *input,
                                  uint8_t *v,
                                  int *len);

void
gcspy_bufferedInputReadShortArray(gcspy_buffered_input_t *input,
                                  short *v,
                                  int *len);

void
gcspy_bufferedInputReadUShortArray(gcspy_buffered_input_t *input,
                                   uint16_t *v,
                                   int *len);

void
gcspy_bufferedInputReadIntArray(gcspy_buffered_input_t *input,
                                int *v,
                                int *len);

void
gcspy_bufferedInputReadColor(gcspy_buffered_input_t *input,
                             gcspy_color_t *color);

void
gcspy_bufferedInputClose(gcspy_buffered_input_t *input);

#endif //_GCSPY_BUFFERED_INPUT_H_
