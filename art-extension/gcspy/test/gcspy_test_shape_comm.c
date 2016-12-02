/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_test_shape_comm.c
 **
 **  Tests the comm and shape stream facilities
 **/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "gcspy_buffered_input.h"
#include "gcspy_buffered_output.h"
#include "gcspy_shape_stream.h"
#include "gcspy_comm.h"

#define MAX_SIZE ( 32 * 1024 )

void
client_send (gcspy_comm_client_t *client,
             gcspy_command_stream_t *stream) {
  gcspy_buffered_output_t output;
  int len;

  gcspy_clientBufferedOutputInit(client, &output);
  gcspy_commandStreamSetBufferedOutput(stream, &output);

  gcspy_commandStreamStart(stream);

  gcspy_shapeStreamRectangle(stream, 2, 3, 10, 12);
  gcspy_shapeStreamCircle(stream, 1, 2, 3);
  gcspy_shapeStreamPoint(stream, 10, 12);
  gcspy_shapeStreamCircle(stream, 3, 5, 2);
  gcspy_shapeStreamRectangle(stream, -2, -3, 3, 4);
  gcspy_shapeStreamPoint(stream, 1, 2);
  gcspy_shapeStreamPoint(stream, 2, 3);
  gcspy_shapeStreamPoint(stream, 4, 3);

  gcspy_commandStreamFinish(stream);

  len = gcspy_bufferedOutputGetLen(&output);
  printf("-- Sending %d bytes\n", len);
  gcspy_clientSend(client, len);
  printf("\n");
}

void
client_receive (gcspy_comm_client_t *client,
                gcspy_command_stream_t *stream) {
  gcspy_buffered_input_t input;
  int len;

  printf("-- Receiving data\n");
  gcspy_clientDefinitelyReceive(client);

  len = gcspy_client_get_len(client);
  printf("--   got %d bytes\n", len);

  gcspy_bufferedInputInit(&input,
                          gcspy_client_get_buffer_in(client),
                          len);
  gcspy_commandStreamExecute(stream, &input, NULL);

  printf("\n");
}

int
main (int argc, char *argv[]) {
  gcspy_comm_server_t server;
  gcspy_comm_client_t client;
  gcspy_command_stream_t stream;
  int port;
  int reps;
  int i;

  gcspy_shapeStreamInit(&stream);
  if (argc != 3) {
    gcspy_raise_error("Wrong number of arguments");
  }
  port = atoi(argv[1]);
  reps = atoi(argv[2]);

  printf("-- Starting server on port %d\n", port);
  gcspy_serverInit(&server, port, 5);

  printf("-- Waiting for a new client\n");
  gcspy_serverWaitForNewClient(&server, &client, MAX_SIZE);

  printf("-- Connection established\n");
  printf("\n");

  for (i = 0; i < reps; ++i) {
    printf("#### Repetition %d out of %d\n", i+1, reps);
    printf("\n");
    client_receive(&client, &stream);
    client_receive(&client, &stream);
    client_send(&client, &stream);
    client_send(&client, &stream);
  }

  printf("-- Closing client\n");
  gcspy_clientClose(&client);

  printf("-- Closing server\n");
  gcspy_serverClose(&server);
}
