/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_test_simple_comm.c
 **
 **  Tests the buffered I/O facilities and the comms framework
 **/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "gcspy_buffered_input.h"
#include "gcspy_buffered_output.h"
#include "gcspy_comm.h"

#define MAX_SIZE ( 32 * 1024 )

void
client_send (gcspy_comm_client_t *client,
             char b, unsigned char ub, short s, unsigned short us, int i,
             const char *string, int mult) {
  gcspy_buffered_output_t output;
  int len;
  int n;
  char *buffer;

  buffer = gcspy_client_get_buffer_out(client);
  gcspy_clientBufferedOutputInit(client, &output);

  gcspy_bufferedOutputWriteByte(&output, b);
  gcspy_bufferedOutputWriteUByte(&output, ub);
  gcspy_bufferedOutputWriteShort(&output, s);
  gcspy_bufferedOutputWriteUShort(&output, us);
  gcspy_bufferedOutputWriteInt(&output, i);
  gcspy_bufferedOutputWriteString(&output, string);
  for (n = 0; n < 15; ++n) {
    gcspy_bufferedOutputWriteInt(&output, n * mult);
  }
  len = gcspy_bufferedOutputGetLen(&output);

  printf("-- Sending %d bytes\n", len);
  gcspy_clientSend(client, len);
  printf("\n");
}

void
client_receive (gcspy_comm_client_t *client) {
  gcspy_buffered_input_t input;
  int len;
  char *buffer;
  char b;
  unsigned char ub;
  short s;
  unsigned short us;
  int i;
  char string[256];

  printf("-- Receiving data\n");
  gcspy_clientDefinitelyReceive(client);

  len = gcspy_client_get_len(client);
  printf("--   got %d bytes\n", len);

  buffer = gcspy_client_get_buffer_in(client);
  gcspy_bufferedInputInit(&input, buffer, len);

  b = gcspy_bufferedInputReadByte(&input);
  ub = gcspy_bufferedInputReadUByte(&input);
  s = gcspy_bufferedInputReadShort(&input);
  us = gcspy_bufferedInputReadUShort(&input);
  i = gcspy_bufferedInputReadInt(&input);
  gcspy_bufferedInputReadString(&input, string);

  printf("--   b = %d, ub = %d, s = %d, us = %d, i = %d, string = %s\n",
         (int) b, (int) ub, (int) s, (int) us, i, string);
  printf("--   Rest:");
  while (!gcspy_bufferedInputFinished(&input)) {
    i = gcspy_bufferedInputReadInt(&input);
    printf(" %d", i);
  }
  printf("\n");
  printf("\n");
}

int
main (int argc, char *argv[]) {
  gcspy_comm_server_t server;
  gcspy_comm_client_t client;
  int port;

  if (argc != 2) {
    gcspy_raise_error("Wrong number of arguments");
  }
  port = atoi(argv[1]);

  printf("-- Starting server on port %d\n", port);
  gcspy_serverInit(&server, port, 5);

  printf("-- Waiting for a new client\n");
  gcspy_serverWaitForNewClient(&server, &client, MAX_SIZE);

  printf("-- Connection established\n");
  printf("\n");

  client_receive(&client);
  client_receive(&client);
  client_receive(&client);
  client_send(&client, -1, 130, -511, 1076, -1000000, "a C string!", 2);
  client_send(&client, 5, 5, 513, 100, 2000666, "another C string!", 1);
  client_send(&client, 3, 255, 100, 50000, 100, "final C string!", -1);

  printf("-- Closing client\n");
  gcspy_clientClose(&client);

  printf("-- Closing server\n");
  gcspy_serverClose(&server);
}
