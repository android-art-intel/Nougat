/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_comm.h
 **
 **  Data structures for the socket connections
 **/

#ifndef _GCSPY_COMM_H_

#define _GCSPY_COMM_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "gcspy_buffered_input.h"
#include "gcspy_buffered_output.h"

typedef struct {
  int                  fd;
  struct sockaddr_in   addr;
} gcspy_comm_server_t;

typedef struct {
  int                  fd;
  struct sockaddr_in   addr;
  int                  maxLen;

  char                *inBuffer;
  int                  len;

  char                *tmpBuffer;
  int                  tmpLen;

  char                *outBuffer;
} gcspy_comm_client_t;

#define gcspy_client_get_buffer_in(_client) ((_client)->inBuffer)
#define gcspy_client_get_buffer_out(_client) ((_client)->outBuffer)
#define gcspy_client_get_len(_client) ((_client)->len)
#define gcspy_client_get_max_len(_client) ((_client)->maxLen)
#define gcspy_client_has_terminated(_client) ((_client)->len == 0)

void
gcspy_serverInit(gcspy_comm_server_t *server,
                 int port,
                 int maxBackLog);

void
gcspy_serverWaitForNewClient(gcspy_comm_server_t *server,
                             gcspy_comm_client_t *client,
                             int maxLen);

void
gcspy_serverClose(gcspy_comm_server_t *server);



void
gcspy_clientReceive(gcspy_comm_client_t *client);

int
gcspy_clientDefinitelyReceive(gcspy_comm_client_t *client);

void
gcspy_clientBufferedInputInit(gcspy_comm_client_t *client,
                              gcspy_buffered_input_t *input);

void
gcspy_clientBufferedOutputInit(gcspy_comm_client_t *client,
                               gcspy_buffered_output_t *output);

void
gcspy_clientSend(gcspy_comm_client_t *client, int len);

void
gcspy_clientSendBuffered(gcspy_comm_client_t *client,
                         gcspy_buffered_output_t *output);

void
gcspy_clientClose(gcspy_comm_client_t *client);

#endif //_GCSPY_COMM_H_
