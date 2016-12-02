/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_main_server.c
 **
 **  The main server running on the VM
 **/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "gcspy_buffered_input.h"
#include "gcspy_buffered_output.h"
#include "gcspy_main_server.h"
#include "gcspy_interpreter.h"
#include "gcspy_timer.h"
#ifdef ANDROID
#include "gcspy_android_logs.h"
#endif

#define GCSPY_MAIN_SERVER_MAX_BACKLOG             5

#define GCSPY_MAIN_SERVER_MAGIC_STRING            "GCspy666"
#define GCSPY_MAIN_SERVER_ENDIANESS               1

#define gcspy_main_server_get_driver(_server, _id)  \
  ( &((_server)->drivers[(_id)]) )

/* ==================================================== */

static void
gcspy_mainServerVerbosePrint (gcspy_main_server_t *server,
                              const char *str) {
  if (server->verbose) {
    gcspy_print_mesg("GCspy Server:  %s\n", str);
  }
}

/* ==================================================== */

static void
gcspy_startElapsedTimer (gcspy_main_server_t *server) {
  server->elapsedTimeStart = gcspy_getTime();
}

static void
gcspy_stopElapsedTimer (gcspy_main_server_t *server) {
  size_t end = gcspy_getTime();
  size_t diff = end - server->elapsedTimeStart;
  server->elapsedTime = (int) diff;
}

static void
gcspy_resetCompensationTimer (gcspy_main_server_t *server) {
  server->compensationTime = 0;
}

void
gcspy_mainServerStartCompensationTimer (gcspy_main_server_t *server) {
  server->compensationTimeStart = gcspy_getTime();
}

void
gcspy_mainServerStopCompensationTimer (gcspy_main_server_t *server) {
  size_t end = gcspy_getTime();
  size_t diff = end - server->compensationTimeStart;
  server->compensationTime += (int) diff;
}

/* ==================================================== */

void
gcspy_mainServerInit (gcspy_main_server_t *server,
                      int port,
                      uint32_t maxLen,
                      const char *name,
                      int verbose) {
  server->maxLen = maxLen;
  server->driverMap = 0;
  server->eventNum = 0;
  server->connected = 0;
  server->pauseReq = 0;
  server->playOne = 0;
  server->shutdownNow = 0;
  server->shutdownSent = 0;
  server->verbose = verbose;
  if (server->verbose) {
    char buffer[512];
    sprintf(buffer, "starting server, port %d", port);
    gcspy_mainServerVerbosePrint(server, buffer);
  }
  gcspy_serverInit(&(server->server), port, GCSPY_MAIN_SERVER_MAX_BACKLOG);
  gcspy_intInit(&server->interpreter);
  if (strlen(name) > GCSPY_MAIN_SERVER_NAME_LEN) {
    char buffer[256];
    sprintf(buffer,
            "MainServer: server name length too long (length %d)",
            (uint32_t)strlen(name));
    gcspy_raise_error(buffer);
  }
  strcpy(server->name, name);
  strcpy(server->generalInfo, "");
}

void
gcspy_mainServerSetGeneralInfo (gcspy_main_server_t *server,
                                char *generalInfo) {
  strcpy(server->generalInfo, generalInfo);
}

gcspy_gc_driver_t *
gcspy_mainServerAddDriver (gcspy_main_server_t *server) {
  gcspy_gc_driver_t *driver = NULL;
  if (server->driverMap == 0xFF) {
    char buffer[256];
    sprintf(buffer, "Main Server: max driver num reached (%d)",
            GCSPY_MAIN_SERVER_MAX_DRIVERS);
    gcspy_raise_error(buffer);
  }
  uint8_t mask = 1;
  for (int i = 0; i < GCSPY_MAIN_SERVER_MAX_DRIVERS; i++) {
    if ((server->driverMap & (mask<<i)) == 0) {
      driver = gcspy_main_server_get_driver(server, i);
      gcspy_driver_set_id(driver, i);
      server->driverMap |= mask<<i;
      break;
    }
  }
  return driver;
}

void
gcspy_mainServerRemoveDriver (gcspy_main_server_t *server, int id) {
  uint8_t mask = 1;
  if ((server->driverMap & (mask << id)) == 1) {
    server->driverMap &= ~(mask<<id);
    gcspy_driverReset(&(server->drivers[id]));
  }
}

static void
gcspy_mainServerInitFilters (gcspy_main_server_t *server) {
  for (uint32_t i = 0; i < server->eventNum; ++i) {
    server->enabled[i] = 1;
    server->delays[i] = 0;
    server->paused[i] = 0;
    server->skip[i] = 1;
  }
}

void
gcspy_mainServerAddEvent (gcspy_main_server_t *server,
                          uint32_t id,
                          const char *name) {
  if (server->eventNum == GCSPY_MAIN_SERVER_MAX_EVENTS) {
    char buffer[256];
    sprintf(buffer, "Main Server: max event num reached (%d)",
            GCSPY_MAIN_SERVER_MAX_EVENTS);
    gcspy_raise_error(buffer);
  }
  if (server->eventNum != id) {
    char buffer[256];
    sprintf(buffer,
            "Main Server: allocated event id (%d) different to given (%d)",
            server->eventNum,
            id);
    gcspy_raise_error(buffer);
  }
  if (strlen(name) > GCSPY_MAIN_SERVER_EVENT_NAME_LEN) {
    char buffer[256];
    sprintf(buffer,
            "MainServer: event %d name length too long (length %d)",
            id,
            (uint32_t)strlen(name));
    gcspy_raise_error(buffer);
  }
  strcpy(server->events[server->eventNum], name);
  server->eventCount[server->eventNum] = 0;
  ++server->eventNum;
}

gcspy_gc_driver_t *
gcspy_mainServerGetDriver (gcspy_main_server_t *server,
                           uint32_t id) {
  return gcspy_main_server_get_driver(server, id);
}

static int
gcspy_mainServerCheckClient (gcspy_main_server_t *server) {
  gcspy_buffered_input_t input;
  gcspy_buffered_output_t output;
  char buffer[256];
  gcspy_comm_client_t *client = &(server->client);

  if (!gcspy_clientDefinitelyReceive(client))
    return 0;

  gcspy_clientBufferedInputInit(client, &input);
  gcspy_bufferedInputReadString(&input, buffer);
  if (strcmp(buffer, GCSPY_MAIN_SERVER_MAGIC_STRING))
    return 0;
  if (gcspy_bufferedInputReadInt(&input) != GCSPY_MAIN_SERVER_ENDIANESS)
    return 0;
  if (gcspy_bufferedInputReadBoolean(&input))
    server->pauseReq = 1;
  if (!gcspy_bufferedInputFinished(&input))
    return 0;

  gcspy_clientBufferedOutputInit(client, &output);
  gcspy_bufferedOutputWriteString(&output, GCSPY_MAIN_SERVER_MAGIC_STRING);
  gcspy_bufferedOutputWriteInt(&output, GCSPY_MAIN_SERVER_ENDIANESS);
  gcspy_bufferedOutputWriteString(&output, server->name);
  gcspy_clientSendBuffered(client, &output);

  return 1;
}

/**
 **    Boot info format when serialised
 **    ----------------------------------
 **      driverNum        short
 **      generalInfo      string
 **      eventNum         short
 **      ( eventName )*   string
 **    ----------------------------------
 **    then call each driver to serialise itself
 **    ----------------------------------
 **/

static void
gcspy_mainServerSendBootInfo (gcspy_main_server_t *server) {
  gcspy_buffered_output_t output;
  gcspy_comm_client_t *client = &server->client;

  short driverNum = 0;
  uint8_t mask = 1;
  for (int i = 0; i < GCSPY_MAIN_SERVER_MAX_DRIVERS; ++i) {
    if (server->driverMap & (mask << i)) driverNum++;
  }
  gcspy_clientBufferedOutputInit(client, &output);
  gcspy_bufferedOutputWriteShort(&output, driverNum);
  gcspy_bufferedOutputWriteString(&output, server->generalInfo);
  gcspy_bufferedOutputWriteShort(&output, (short) server->eventNum);
  for (uint32_t i = 0; i < server->eventNum; ++i) {
    gcspy_bufferedOutputWriteString(&output, server->events[i]);
  }
  gcspy_clientSendBuffered(client, &output);

  for (int i = 0; i < GCSPY_MAIN_SERVER_MAX_DRIVERS; ++i) {
    if (server->driverMap & (mask << i))
      gcspy_driverSerialise(&server->drivers[i], client);
  }
}

static void
gcspy_mainServerPause (gcspy_main_server_t *server) {
  gcspy_buffered_output_t output;

  gcspy_clientBufferedOutputInit(&server->client, &output);
  gcspy_commandStreamSetBufferedOutput(&server->interpreter, &output);
  gcspy_commandStreamStart(&server->interpreter);
  gcspy_intWritePause(&server->interpreter);
  gcspy_commandStreamFinish(&server->interpreter);
  gcspy_clientSendBuffered(&server->client, &output);
}

static void
gcspy_mainServerEvent (gcspy_main_server_t *server,
                       uint32_t event,
                       int elapsedTime,
                       int compensationTime) {
  gcspy_buffered_output_t output;

  gcspy_clientBufferedOutputInit(&server->client, &output);
  gcspy_commandStreamSetBufferedOutput(&server->interpreter, &output);
  gcspy_commandStreamStart(&server->interpreter);
  gcspy_intWriteEvent(&server->interpreter, event);
  gcspy_bufferedOutputWriteInt(&output, elapsedTime);
  gcspy_bufferedOutputWriteInt(&output, compensationTime);
  gcspy_commandStreamFinish(&server->interpreter);
  gcspy_clientSendBuffered(&server->client, &output);
}

void
gcspy_mainServerShutdown (gcspy_main_server_t *server) {
  gcspy_buffered_output_t output;

  gcspy_clientBufferedOutputInit(&server->client, &output);
  gcspy_commandStreamSetBufferedOutput(&server->interpreter, &output);
  gcspy_commandStreamStart(&server->interpreter);
  gcspy_intWriteShutdown(&server->interpreter);
  gcspy_commandStreamFinish(&server->interpreter);
  gcspy_clientSendBuffered(&server->client, &output);
  server->connected = 0;
}

void
gcspy_mainServerEventCount (gcspy_main_server_t *server) {
  gcspy_buffered_output_t output;

  gcspy_clientBufferedOutputInit(&server->client, &output);
  gcspy_commandStreamSetBufferedOutput(&server->interpreter, &output);
  gcspy_commandStreamStart(&server->interpreter);
  gcspy_intWriteEventCount(&server->interpreter);
  gcspy_bufferedOutputWriteIntArray(&output,
                                    server->eventCount,
                                    server->eventNum);
  gcspy_commandStreamFinish(&server->interpreter);
  gcspy_clientSendBuffered(&server->client, &output);
}

int
gcspy_mainServerConnect (gcspy_main_server_t *server) {
  gcspy_comm_client_t *client = &server->client;

  gcspy_mainServerVerbosePrint(server, "waiting for client to connect");
  gcspy_serverWaitForNewClient(&server->server,
                               client,
                               server->maxLen);
  gcspy_mainServerVerbosePrint(server, "  client connected");

  if (!gcspy_mainServerCheckClient(server))
    return 0;
  gcspy_mainServerVerbosePrint(server, "  sending boot info");
  gcspy_mainServerSendBootInfo(server);
  gcspy_mainServerInitFilters(server);

  uint8_t mask = 1;
  for (int i = 0; i < GCSPY_MAIN_SERVER_MAX_DRIVERS; ++i) {
    if (server->driverMap & (mask << i))
      gcspy_driverSetComm(&server->drivers[i],
                          &server->client,
                          &server->interpreter);
  }

  gcspy_resetCompensationTimer(server);
  gcspy_startElapsedTimer(server);

  server->connected = 1;

  return 1;
}

static int
gcspy_mainServerInnerLoop (gcspy_main_server_t *server) {
  gcspy_buffered_input_t input;
  gcspy_comm_client_t *client = &server->client;
  gcspy_command_stream_t *stream = &server->interpreter;
  gcspy_int_aux_t aux;
  int requestShutdown = 0;

  gcspy_mainServerVerbosePrint(server, "  starting server main loop");
  while ( 1 ) {
    gcspy_clientReceive(client);
    if (gcspy_client_has_terminated(client)) {
      gcspy_mainServerVerbosePrint(server,
                                   "  main loop terminated (client died)");
      requestShutdown = 0;
      server->connected = 0;
      break;
    }

    gcspy_intAuxInit(&aux);
    gcspy_clientBufferedInputInit(client, &input);
    gcspy_commandStreamExecute(stream, &input, (void *) &aux);
    if (aux.shutdownReq) {
      gcspy_mainServerVerbosePrint(server,
                                   "  main loop terminated (shutdown request received)");
      requestShutdown = 1;
      break;
    }
    if (aux.pauseReq) {
      server->pauseReq = 1;
    }
    if (aux.restart) {
      server->pauseReq = 0;
    }
    if (aux.playOne) {
      server->playOne = 1;
    }
    if (aux.filter > 0) {
      for (int i = 0; i < aux.filter; ++i) {
        server->enabled[i] = aux.enabled[i];
        server->delays[i] = aux.delays[i];
        server->paused[i] = aux.paused[i];
        server->skip[i] = aux.skip[i];
      }
    }
  }
  return requestShutdown;
}

NORETURN void
gcspy_mainServerMainLoop (gcspy_main_server_t *server) {
  int requestShutdown;
  server->shutdownNow = 0;
  while ( 1 ) {
    gcspy_mainServerConnect(server);

    server->shutdownNow = 0;
    server->shutdownSent = 0;
    requestShutdown = gcspy_mainServerInnerLoop(server);
    if (requestShutdown) {
      server->shutdownNow = 1;
      while (!server->shutdownSent) {
        gcspy_wait(50);
      }
    }
  }
}

static void
gcspy_mainServerInternalSafepoint (gcspy_main_server_t *server) {
  if (server->shutdownNow) {
    gcspy_mainServerShutdown(server);
    server->shutdownSent = 1;
  }
  if (server->pauseReq) {
    if (server->pauseReq == 1) {
      gcspy_mainServerVerbosePrint(server, "  paused");
      gcspy_mainServerPause(server);
    }

    while ( server->pauseReq && server->connected ) {
      gcspy_wait(10);
      if (server->shutdownNow) {
        server->pauseReq = 0;
        gcspy_mainServerShutdown(server);
        server->shutdownSent = 1;
      }
      if (server->playOne) {
        gcspy_mainServerVerbosePrint(server, "  play one");
        server->playOne = 0;
        server->pauseReq = 2;
        break;
      }
    }

    if (!server->pauseReq && !server->shutdownNow)
      gcspy_mainServerVerbosePrint(server, "  restarted");
  }
}

static int
gcspy_mainServerIsJustConnected (gcspy_main_server_t *server);

static void
gcspy_mainServerJustSafepoint (gcspy_main_server_t *server) {
  if (gcspy_mainServerIsJustConnected(server)) {
    gcspy_mainServerInternalSafepoint(server);
  }
}

void
gcspy_mainServerSafepoint (gcspy_main_server_t *server,
                           uint32_t event) {
  ++server->eventCount[event];
  if (gcspy_mainServerIsConnected(server, event)) {
    gcspy_stopElapsedTimer(server);
    gcspy_mainServerEventCount(server);

    gcspy_mainServerEvent(server, event,
                          server->elapsedTime,
                          server->compensationTime);

    if (server->delays[event]) {
      gcspy_wait(server->delays[event]);
    }
    if (server->paused[event]) {
      server->pauseReq = 1;
    }
    gcspy_mainServerInternalSafepoint(server);
    gcspy_resetCompensationTimer(server);
    gcspy_startElapsedTimer(server);
  }
}

void
gcspy_mainServerWaitForClient (gcspy_main_server_t *server) {
  gcspy_mainServerVerbosePrint(server, "blocked until client connects");

  while (!gcspy_mainServerIsJustConnected(server)) {
    gcspy_wait(50);
  }

  gcspy_mainServerJustSafepoint(server);
}

static int
gcspy_mainServerIsJustConnected (gcspy_main_server_t *server) {
  return server->connected;
}

int
gcspy_mainServerIsConnected (gcspy_main_server_t *server,
                             uint32_t event) {
  return ( server->connected && server->enabled[event] &&
           ((server->eventCount[event] % (server->skip[event])) == 0) );
}
