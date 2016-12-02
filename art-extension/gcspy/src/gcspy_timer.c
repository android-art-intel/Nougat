/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_timer.c
 **
 **  Timer facilities
 **/

#include <sys/time.h>
#ifdef _SPARC_
#include <unistd.h>
#endif //_SPARC_
#ifdef _LINUX_
#include <time.h>
#endif

#include "gcspy_timer.h"

void
gcspy_wait(int ms) {
#ifdef _SPARC_
  {
    hrtime_t timer;
    timer = gethrtime();
    while ((gethrtime() - timer) < (long long) ms * 1000000) {
      yield();
    }
  }
#endif //_SPARC_

#ifdef _LINUX_
  {
    struct timespec _interval;
    struct timespec _spec;

    time_t secs = ms / 1000;
    long nanos = (ms % 1000) * (1000 * 1000);

    _interval.tv_sec  = secs;           /* num of seconds     */
    _interval.tv_nsec = nanos;          /* num of nanoseconds */

    nanosleep(&_interval, &_spec);
  }
#endif //_LINUX_
}

/* time in milliseconds */
size_t gcspy_getTime () {
#ifdef _SPARC_
  {
    long long ret = (long long) gethrtime() / (long long) 1000000;
    return ret;
  }
#endif //_SPARC_

#ifdef _LINUX_
  {
    struct timeval tv;
    struct timezone tz;
    size_t diff;

    gettimeofday(&tv, &tz);
    diff = ((size_t) tv.tv_sec) * 1000 + ((size_t) tv.tv_usec) / 1000;

    return diff;
  }
#endif //_LINUX_
}
