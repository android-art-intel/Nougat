/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_test_server.c
 **
 **  A test server to test the framework
 **/

#include <stdio.h>
#include <stdlib.h>

#include "gcspy_comm.h"
#include "gcspy_timer.h"

#define MAX_SIZE       ( 32 * 1024 )

int
main (int argc, char *argv[]) {
  gcspy_comm_server_t server;
  gcspy_comm_client_t client;
  int port;
  int i, size;
  char *buffer;

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

  printf("-- Receiving data\n");
  gcspy_clientReceive(&client);
  while ( 1 ) {
    if (gcspy_client_has_terminated(&client)) {
      printf("--   Client closed\n");
      goto the_end;
    }

    size = gcspy_client_get_len(&client);
    buffer = gcspy_client_get_buffer_in(&client);
    printf("--   Received %d bytes\n", size);
    printf("--   Data:");
    for (i = 0; i < size; ++i)
      printf(" %d", buffer[i]);
    printf("\n");

    if (buffer[0] == 66)
      break;

    printf("\n");
    printf("-- Receiving data\n");
    gcspy_clientReceive(&client);
  }

  printf("--   Receive finished\n");
  printf("\n");

  buffer = gcspy_client_get_buffer_out(&client);

  for (i = 0; i < 3; ++i) {
    printf("-- Sending data\n");
    buffer[0] = 1;
    buffer[1] = 2;
    buffer[2] = 3;
    gcspy_clientSend(&client, 3);

    gcspy_wait(2000);
  }

  printf("-- Sending last value\n");
  buffer[0] = 66;
  gcspy_clientSend(&client, 1);

  gcspy_wait(1000);

the_end:
  printf("\n");
  printf("-- Closing client\n");
  gcspy_clientClose(&client);

  printf("-- Closing server\n");
  gcspy_serverClose(&server);
}
