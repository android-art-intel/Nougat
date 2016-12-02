/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_utils.c
 **
 **  Misc Utilities
 **/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gcspy_utils.h"
#ifdef ANDROID
#include "gcspy_android_logs.h"
#endif

void
gcspy_raiseError(const char *file, int line, const char *mesg) {
#ifdef ANDROID
  __ERR("## (%s, line %d)\n## %s", file, line, mesg);
#else
  printf("## GCspy Error  (%s, line %d)\n", file, line);
  printf("## %s\n", mesg);
#endif

  exit(-1);
}

#define gcspy_set_date_time_vars(_date_time_)   \
  char _tmp[128];                               \
  char *day_month;                              \
  char *year;                                   \
  char *time;                                   \
                                                \
  strcpy(_tmp, ctime(&_date_time_));            \
  _tmp[10] = '\0';                              \
  _tmp[19] = '\0';                              \
  _tmp[24] = '\0';                              \
  day_month = _tmp;                             \
  time = _tmp+11;                               \
  year = _tmp+20

gcspy_date_time_t
gcspy_getDateTime () {
  return time(NULL);
}

char *
gcspy_formatDateTime (gcspy_date_time_t dateTime) {
  static char buffer[128];
  sprintf(buffer, "%s, %s",
          gcspy_formatTime(dateTime), gcspy_formatDate(dateTime));
  return buffer;
}

char *
gcspy_formatTime (gcspy_date_time_t dateTime) {
  static char buffer[128];
  gcspy_set_date_time_vars(dateTime);
  strcpy(buffer, time);
  return buffer;
}

char *
gcspy_formatDate (gcspy_date_time_t dateTime) {
  static char buffer[128];
  gcspy_set_date_time_vars(dateTime);
  sprintf(buffer, "%s, %s", day_month, year);
  return buffer;
}

char *
gcspy_formatSize (unsigned int size) {
  static char buffer[128];
  if (size < (1024 * 1024))
    sprintf(buffer, "%1.1lfKB (%s bytes)",
            ((double) size / KILOBYTE_D),
            gcspy_formatLargeNumber(size));
  else
    sprintf(buffer, "%1.1lfMB (%s bytes)",
            ((double) size / MEGABYTE_D),
            gcspy_formatLargeNumber(size));
  return buffer;
}

char *
gcspy_formatLargeNumber (unsigned int num) {
  static char buffer[128];
  int curr;
  sprintf(buffer, "%u", num);
  curr = strlen(buffer) - 3;
  while (curr > 0) {
    memmove(&buffer[curr+1], &buffer[curr], strlen(buffer) - curr+1);
    buffer[curr] = ',';

    curr -= 3;
  }
  return buffer;
}
