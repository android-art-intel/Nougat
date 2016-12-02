/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_comm.c
 **
 **  Data structures for the socket connections
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "gcspy_timer.h"
#include "gcspy_comm.h"
#include "gcspy_buffered_input.h"
#include "gcspy_buffered_output.h"
#include "gcspy_array_input.h"
#include "gcspy_array_output.h"


/** SERVER STUFF **/

void
gcspy_serverInit (gcspy_comm_server_t *server, int port, int maxBackLog) {
  int res;

  server->fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server->fd < 0)
    gcspy_raise_error("server: could not create socket");
  {
    int x = 1;
    setsockopt(server->fd, SOL_SOCKET,SO_REUSEADDR, &x, sizeof(int));
  }

  server->addr.sin_family = AF_INET;
  server->addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server->addr.sin_port = htons(port);

  res = bind(server->fd,
             (struct sockaddr *) &(server->addr),
             sizeof(server->addr));
  if (res < 0)
    gcspy_raise_error("server: bind failed");

  listen(server->fd, maxBackLog);
}

void
gcspy_serverWaitForNewClient(gcspy_comm_server_t *server,
                             gcspy_comm_client_t *client,
                             int maxLen) {
  socklen_t clientLen;

  clientLen = sizeof(client->addr);
  client->fd = accept(server->fd,
                      (struct sockaddr *) &(client->addr),
                      &clientLen);
  if (client->fd < 0)
    gcspy_raise_error("server: accept failed");

  client->maxLen = maxLen;

  client->inBuffer = (char *) malloc(maxLen);
  if (client->inBuffer == NULL) {
    char buffer[256];
    sprintf(buffer, "client: malloc failed for a buffer of len %d", maxLen);
    gcspy_raise_error(buffer);
  }

  client->tmpLen = 0;
  client->tmpBuffer = (char *) malloc(2*maxLen);
  if (client->tmpBuffer == NULL) {
    char buffer[256];
    sprintf(buffer, "client: malloc failed for a buffer of len %d", 2*maxLen);
    gcspy_raise_error(buffer);
  }

  client->outBuffer = (char *) malloc(maxLen);
  if (client->outBuffer == NULL) {
    char buffer[256];
    sprintf(buffer, "client: malloc failed for a buffer of len %d", maxLen);
    gcspy_raise_error(buffer);
  }
}

void
gcspy_serverClose (gcspy_comm_server_t *server) {
  gcspy_wait(100);
  close(server->fd);
}

/** DELIMITERS **/

#define GCSPY_COMM_DEL_SIZE    4

/*
  static int
  gcspy_clientReadStreamLenOLD(gcspy_comm_client_t *client) {
  gcspy_buffered_input_t input;
  gcspy_bufferedInputInit(&input, client->tmpBuffer, GCSPY_COMM_DEL_SIZE);
  return gcspy_bufferedInputReadInt(&input);
  }
*/

/* is this nicer than the OLD version? */
static int
gcspy_clientReadStreamLen(gcspy_comm_client_t *client) {
  return gcspy_arrayInputReadInt(client->tmpBuffer);
}

/*
  static void
  gcspy_clientWriteStreamLenOLD(gcspy_comm_client_t *client, int len) {
  gcspy_buffered_output_t output;
  char buffer[GCSPY_COMM_DEL_SIZE];
  int res;
  gcspy_bufferedOutputInit(&output, buffer, GCSPY_COMM_DEL_SIZE);
  gcspy_bufferedOutputWriteInt(&output, len);
  res = send(client->fd, buffer, GCSPY_COMM_DEL_SIZE, 0);
  if (res < GCSPY_COMM_DEL_SIZE)
  gcspy_raise_error("client: send failed");
  }
*/

/* is this nicer than the OLD version? */
static void
gcspy_clientWriteStreamLen(gcspy_comm_client_t *client, int len) {
  char buffer[GCSPY_COMM_DEL_SIZE];
  int res;
  gcspy_arrayOutputWriteInt(buffer, len);
  res = send(client->fd, buffer, GCSPY_COMM_DEL_SIZE, 0);
  if (res < GCSPY_COMM_DEL_SIZE)
    gcspy_raise_error("client: send failed");
}

/** CLIENT STUFF **/

void
gcspy_clientReceive(gcspy_comm_client_t *client) {
  int res;
  int totalLen;

  while (client->tmpLen < GCSPY_COMM_DEL_SIZE) {
    res = recv(client->fd, client->tmpBuffer + client->tmpLen,
               client->maxLen, 0);
    if (res < 0)
      gcspy_raise_error("client: receive failed");
    if (res == 0) {
      client->len = 0;
      client->tmpLen = 0;
      return;
    }
    client->tmpLen += res;
  }
  client->len = gcspy_clientReadStreamLen(client);

  totalLen = client->len + GCSPY_COMM_DEL_SIZE;
  while (client->tmpLen < totalLen) {
    res = recv(client->fd, client->tmpBuffer + client->tmpLen,
               client->maxLen, 0);
    if (res <= 0)
      gcspy_raise_error("client: receive failed");
    client->tmpLen += res;
  }

  memcpy(client->inBuffer,
         client->tmpBuffer + GCSPY_COMM_DEL_SIZE,
         client->len);
  if (client->tmpLen > totalLen) {
    memcpy(client->tmpBuffer,
           client->tmpBuffer + totalLen,
           client->tmpLen - totalLen);
    client->tmpLen -= totalLen;
  } else {
    client->tmpLen = 0;
  }
}

int
gcspy_clientDefinitelyReceive(gcspy_comm_client_t *client) {
  gcspy_clientReceive(client);
  if (gcspy_client_has_terminated(client))
    return 0;
  return 1;
}

void
gcspy_clientBufferedInputInit(gcspy_comm_client_t *client,
                              gcspy_buffered_input_t *input) {
  gcspy_bufferedInputInit(input,
                          gcspy_client_get_buffer_in(client),
                          gcspy_client_get_len(client));
}

void
gcspy_clientBufferedOutputInit(gcspy_comm_client_t *client,
                               gcspy_buffered_output_t *output) {
  gcspy_bufferedOutputInit(output,
                           gcspy_client_get_buffer_out(client),
                           gcspy_client_get_max_len(client));
}

void
gcspy_clientSend(gcspy_comm_client_t *client, int len) {
  int res;

  gcspy_clientWriteStreamLen(client, len);
  res = send(client->fd, client->outBuffer, len, 0);
  if (res != len)
    gcspy_raise_error("client: send failed");
}

void
gcspy_clientSendBuffered(gcspy_comm_client_t *client,
                         gcspy_buffered_output_t *output) {
  gcspy_clientSend(client, gcspy_bufferedOutputGetLen(output));
}

void
gcspy_clientClose(gcspy_comm_client_t *client) {
  gcspy_wait(100);

  free(client->inBuffer);
  client->inBuffer = NULL;

  free(client->outBuffer);
  client->outBuffer = NULL;

  free(client->tmpBuffer);
  client->tmpBuffer = NULL;

  close(client->fd);
}
