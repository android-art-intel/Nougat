#ifndef _ANDROID_LOGS_H_
#define _ANDROID_LOGS_H_

#include <android/log.h>

#define LOGTAG "_GCSPY_"

#define __LOG(F, ...) \
  __android_log_print(ANDROID_LOG_INFO, LOGTAG, F, __VA_ARGS__)

#define __MSG(msg) \
  __android_log_write(ANDROID_LOG_INFO, LOGTAG, msg)

#define __ERR(F, ...) \
  __android_log_print(ANDROID_LOG_ERROR, LOGTAG, F, __VA_ARGS__)

#define __ERRMSG(msg) \
  __android_log_write(ANDROID_LOG_ERROR, LOGTAG, msg)

#endif // _ANDROID_LOGS_H_
