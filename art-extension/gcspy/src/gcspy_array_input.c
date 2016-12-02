/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_array_input.c
 **
 **  Input from a byte array
 **/

#include "gcspy_array_input.h"

static int
gcspy_arrayInputBaseRead (char *buffer) {
  int res;
  res = (int) *buffer;
  if (res < 0)
    res += 256;
  return res;
}

int
gcspy_arrayInputReadInt (char *buffer) {
  int res = 0;

  res = (int) gcspy_arrayInputBaseRead(buffer);
  res <<= 8;
  res |= (int) gcspy_arrayInputBaseRead(buffer+1);
  res <<= 8;
  res |= (int) gcspy_arrayInputBaseRead(buffer+2);
  res <<= 8;
  res |= (int) gcspy_arrayInputBaseRead(buffer+3);

  return res;
}
