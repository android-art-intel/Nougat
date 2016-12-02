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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "gcspy_comm.h"
#include "gcspy_timer.h"
#include "gcspy_main_server.h"
#include "dummy_driver.h"

static gcspy_main_server_t server;
static dummy_driver_t dummyDriver;

#define START     0
#define FIRST   ( ONE - (5 * 1024) )
#define ONE     ( 2 * 1024 * 1024 )
#define END     ( 4 * 1024 * 1024 )

#define CARD_SIZE  1024

#define START_GC_EVENT      0
#define END_GC_EVENT        1

#define MAX_SIZE       ( 128 * 1024 )
#define TILE_SIZE      ( 32 * 1024 )

static void *
mainLoop(void *arg)
{
  int count = -1;
  char *limit;
  char *end;
  int event = START_GC_EVENT;

  while ( 1 ) {
    gcspy_wait(1000);

    if (gcspy_mainServerIsConnected(&server, event)) {
      gcspy_mainServerStartCompensationTimer(&server);

      ++count;
      switch (count) {
        case 0:
          limit = (char *) FIRST;
          end = (char *) START;
          break;
        case 1:
          limit = (char *) ONE;
          end = (char *) (ONE * 2 / 3);
          break;
        case 2:
          limit = (char *) END;
          end = (char *) (3 * ONE / 2);
          break;
        case 3:
          limit = (char *) END;
          end = (char *) ((3 * ONE / 2) + 4000);
          count = -1;
          break;
      }

      dummyDriverZero(&dummyDriver, limit);
#if 0
      dummyDriverLimit(&dummyDriver, limit);
#endif //0
      dummyDriverSetEnd(&dummyDriver, end);

      switch ((count+1) % 2) {
        case 0:
          dummyDriverRoot(&dummyDriver, (char *) START + 2);
          dummyDriverRoot(&dummyDriver, (char *) START + 100);
          dummyDriverRoot(&dummyDriver, (char *) START + 100);
          dummyDriverRoot(&dummyDriver, (char *) START + 100);
          dummyDriverRoot(&dummyDriver, (char *) START + 101);
          dummyDriverRoot(&dummyDriver, (char *) START + 2 * (TILE_SIZE) + 2);
          dummyDriverRoot(&dummyDriver, (char *) START + 2 * (TILE_SIZE) + 10);
          break;
        case 1:
          dummyDriverRoot(&dummyDriver, (char *) START + 101);
          dummyDriverRoot(&dummyDriver, (char *) START + 2 * (TILE_SIZE) + 10);
          dummyDriverRoot(&dummyDriver, (char *) START + 2 * (TILE_SIZE) + 2);
          break;
      }

      dummyDriverCard(&dummyDriver, (char *) START, CARD_SIZE,
                   DUMMY_CARD_STATE_DIRTY);
      dummyDriverCard(&dummyDriver, (char *) START + CARD_SIZE, CARD_SIZE,
                   DUMMY_CARD_STATE_CLEAN);

      dummyDriverCard(&dummyDriver, (char *) (ONE / 2), CARD_SIZE,
                   DUMMY_CARD_STATE_SUMMARISED);
      dummyDriverCard(&dummyDriver, (char *) (ONE / 2 + CARD_SIZE), CARD_SIZE,
                   DUMMY_CARD_STATE_CLEAN);

      gcspy_mainServerStopCompensationTimer(&server);

      if (gcspy_mainServerIsConnected(&server, event)) {
        printf("CONNECTED\n");
        dummyDriverSend(&dummyDriver, event);
      }
    }
    gcspy_mainServerSafepoint(&server, event);
    if (event == START_GC_EVENT)
      event = END_GC_EVENT;
    else
      event = START_GC_EVENT;
  }

  return NULL;
}


int
main (int argc, char *argv[]) {
  char generalInfo[256];
  gcspy_date_time_t time;
  gcspy_gc_driver_t *driver;
  int port;
  int i, size;
  char *buffer;

  if (argc != 2) {
    gcspy_raise_error("Wrong number of arguments");
  }

  port = atoi(argv[1]);

  printf("-- Starting server on port %d\n", port);
  gcspy_mainServerInit(&server, port, MAX_SIZE, "GCSPY@DUMMY", 1);

  time = gcspy_getDateTime();
  strcpy(generalInfo, "GCspy Test\n\n");
  strcat(generalInfo, "Start Time:\n  ");
  strcat(generalInfo, gcspy_formatDateTime(time));
  strcat(generalInfo, "\nApplication:\n  Dummy Driver Test\n\n");
  strcat(generalInfo, "1 Space");
  gcspy_mainServerSetGeneralInfo(&server, generalInfo);

  printf("--   Setting event 0\n");
  gcspy_mainServerAddEvent(&server, START_GC_EVENT, "Start M&C GC");
  printf("--   Setting event 1\n");
  gcspy_mainServerAddEvent(&server, END_GC_EVENT, "End M&C GC");

  printf("--   Setting up driver 0\n");
  driver = gcspy_mainServerAddDriver(&server);

  dummyDriverInit(&dummyDriver, driver, "Single Generation",
                  TILE_SIZE, (char *)START, (char *)FIRST);

  pthread_t tid;
  int res = pthread_create(&tid, NULL, mainLoop, NULL);
  if (res != 0) return -1;

#if 1
  gcspy_mainServerMainLoop(&server);
#else
  gcspy_mainServerWaitForClient(&server);

  printf("-- Connection established\n");
  printf("\n");
  gcspy_wait(300 * 1000);  /* run 300 seconds? */

the_end:
  printf("\n");
  printf("-- Shutting down server\n");
  gcspy_mainServerShutdown(&server);
#endif
  return 0;
}
