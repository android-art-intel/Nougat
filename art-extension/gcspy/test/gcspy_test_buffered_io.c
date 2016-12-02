/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_test_buffered_io.c
 **
 **  Tests the buffered I/O facilities
 **/

#include <stdio.h>
#include "gcspy_buffered_input.h"
#include "gcspy_buffered_output.h"

int
main (int argc, char *argv[]) {
  char buffer[1024];
  gcspy_buffered_input_t input;
  gcspy_buffered_output_t output;
  int n;
  int len;
  char b;
  unsigned char ub;
  short s;
  unsigned short us;
  int i;
  char string[256];

  gcspy_bufferedOutputInit(&output, buffer, 1024);

  gcspy_bufferedOutputWriteByte(&output, 1);
  gcspy_bufferedOutputWriteUByte(&output, 255);
  gcspy_bufferedOutputWriteShort(&output, 511);
  gcspy_bufferedOutputWriteShort(&output, 1034);
  gcspy_bufferedOutputWriteInt(&output, 2000666);
  gcspy_bufferedOutputWriteString(&output, "This is a string!");
  for (n = 0; n < 15; ++n) {
    gcspy_bufferedOutputWriteInt(&output, n);
  }
  len = gcspy_bufferedOutputGetLen(&output);

  gcspy_bufferedInputInit(&input, buffer, len);

  b = gcspy_bufferedInputReadByte(&input);
  ub = gcspy_bufferedInputReadUByte(&input);
  s = gcspy_bufferedInputReadShort(&input);
  us = gcspy_bufferedInputReadUShort(&input);
  i = gcspy_bufferedInputReadInt(&input);
  gcspy_bufferedInputReadString(&input, string);

  printf("b = %d, ub = %d, s = %d, us = %d, i = %d, string = %s\n",
	 (int) b, (int) ub, (int) s, (int) us, i, string);
  printf("Rest:");
  while (!gcspy_bufferedInputFinished(&input)) {
    i = gcspy_bufferedInputReadInt(&input);
    printf(" %d", i);
  }
  printf("\n");
}
