/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_main_main_server.h
 **
 **  The main server running on the VM
 **/

#ifndef _GCSPY_MAIN_SERVER_H_

#define _GCSPY_MAIN_SERVER_H_

#include "gcspy_comm.h"
#include "gcspy_gc_driver.h"
#include "gcspy_interpreter.h"

#include <limits.h>


#define GCSPY_MAIN_SERVER_NAME_LEN                40
#define GCSPY_MAIN_SERVER_EVENT_NAME_LEN          40
#define GCSPY_MAIN_SERVER_MAX_DRIVERS             CHAR_BIT
#define GCSPY_MAIN_SERVER_MAX_EVENTS              GCSPY_INT_MAX_EVENTS

typedef struct {
  gcspy_comm_server_t           server;
  gcspy_comm_client_t           client;

  uint32_t                      maxLen;
  int                           verbose;

  int                           connected;

  int                           shutdownNow;
  int                           shutdownSent;

  int                           pauseReq;
  int                           playOne;

  size_t                        elapsedTimeStart;
  size_t                        compensationTimeStart;
  int                           elapsedTime;
  int                           compensationTime;

  uint32_t                      eventNum;
  uint8_t                       driverMap;

  char                          name[GCSPY_MAIN_SERVER_NAME_LEN+1];

  gcspy_gc_driver_t             drivers[GCSPY_MAIN_SERVER_MAX_DRIVERS];
  char                          events[GCSPY_MAIN_SERVER_MAX_EVENTS][GCSPY_MAIN_SERVER_EVENT_NAME_LEN + 1];

  char                          generalInfo[4096];

  int                           eventCount[GCSPY_MAIN_SERVER_MAX_EVENTS];

  char                          enabled[GCSPY_MAIN_SERVER_MAX_EVENTS];
  int                           delays[GCSPY_MAIN_SERVER_MAX_EVENTS];
  char                          paused[GCSPY_MAIN_SERVER_MAX_EVENTS];
  int                           skip[GCSPY_MAIN_SERVER_MAX_EVENTS];

  gcspy_command_stream_t        interpreter;
} gcspy_main_server_t;

void
gcspy_mainServerInit (gcspy_main_server_t *server,
                      int port,
                      uint32_t maxLen,
                      const char *name,
                      int verbose);

void
gcspy_mainServerSetGeneralInfo (gcspy_main_server_t *server,
                                char *generalInfo);

gcspy_gc_driver_t *
gcspy_mainServerAddDriver (gcspy_main_server_t *server);

void
gcspy_mainServerRemoveDriver (gcspy_main_server_t *server, int id);

void
gcspy_mainServerAddEvent (gcspy_main_server_t *server,
                          uint32_t id,
                          const char *name);

gcspy_gc_driver_t *
gcspy_mainServerGetDriver (gcspy_main_server_t *server,
                           uint32_t id);

void
gcspy_mainServerShutdown (gcspy_main_server_t *server);

void
gcspy_mainServerStartCompensationTimer (gcspy_main_server_t *server);

void
gcspy_mainServerStopCompensationTimer (gcspy_main_server_t *server);

int
gcspy_mainServerConnect (gcspy_main_server_t *server);

int
gcspy_mainServerIsConnected (gcspy_main_server_t *server,
                             uint32_t event);

void
gcspy_mainServerMainLoop (gcspy_main_server_t *server);

void
gcspy_mainServerSafepoint (gcspy_main_server_t *server,
                           uint32_t event);

void
gcspy_mainServerWaitForClient (gcspy_main_server_t *server);

#endif //_GCSPY_MAIN_SERVER_H_
