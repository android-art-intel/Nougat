/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_utils.h
 **
 **  Misc Utilities
 **/

#ifndef _GCSPY_UTILS_H_

#define _GCSPY_UTILS_H_

#include <time.h>
#ifdef ANDROID
#include "gcspy_android_logs.h"
#endif

typedef time_t gcspy_date_time_t;

#define UNUSED_ __attribute__((__unused__))
#define NORETURN __attribute__((__noreturn__))

#define KILOBYTE 1024
#define MEGABYTE (1024 * 1024)

#define KILOBYTE_D ((double) KILOBYTE)
#define MEGABYTE_D ((double) MEGABYTE)

#define gcspy_raise_error(_mesg)                    \
  do {                                              \
    gcspy_raiseError(__FILE__, __LINE__, (_mesg));  \
  } while (0)



#define GCSPY_ASSERT           1

#if GCSPY_ASSERT
#define gcspy_assert(_expr)                           \
  do {                                                \
    if (!(_expr)) {                                   \
      gcspy_raise_error("assertion failure: "#_expr); \
    }                                                 \
  } while (0)
#else //GCSPY_ASSERT
#define gcspy_assert(_expr)
#endif //GCSPY_ASSERT


void
gcspy_raiseError (const char *file, int line, const char *mesg);

#ifdef ANDROID
#define gcspy_print_mesg(F, ...) __LOG(F, __VA_ARGS__)
#else
#define gcspy_print_mesg(F, ...) printf(F, __VA_ARGS__)
#endif

gcspy_date_time_t
gcspy_getDateTime (void);

char *
gcspy_formatDateTime (gcspy_date_time_t time);

char *
gcspy_formatTime (gcspy_date_time_t time);

char *
gcspy_formatDate (gcspy_date_time_t time);

char *
gcspy_formatSize (uint32_t size);

char *
gcspy_formatLargeNumber (uint32_t num);

#endif //_GCSPY_UTILS_H_
