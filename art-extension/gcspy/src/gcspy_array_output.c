/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_array_output.c
 **
 **  Output to a byte array
 **/

#include "gcspy_array_output.h"

#define GCSPY_ARRAY_LOW_8_MASK       ( (1 << 8) - 1 )

static void
gcspy_arrayOutputBaseWrite(char *buffer, int v) {
  if (v > 127)
    v -= 256;
  *buffer = (char) v;
}

void
gcspy_arrayOutputWriteInt (char *buffer, int v) {
  gcspy_arrayOutputBaseWrite(buffer,
			     (int) (v >> 24));
  gcspy_arrayOutputBaseWrite(buffer+1,
			     (int) ((v >> 16) & GCSPY_ARRAY_LOW_8_MASK));
  gcspy_arrayOutputBaseWrite(buffer+2,
			     (int) ((v >> 8) & GCSPY_ARRAY_LOW_8_MASK));
  gcspy_arrayOutputBaseWrite(buffer+3,
			     (int) (v & GCSPY_ARRAY_LOW_8_MASK));
}
