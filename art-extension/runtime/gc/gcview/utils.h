// Copyright (c) 2013 Adobe Systems Incorporated. All rights reserved.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ART_RUNTIME_GC_GCVIEW_UTILS_H_

#define ART_RUNTIME_GC_GCVIEW_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mm.h"

#define GCVIEW_ENABLE_ASSERT 1

#define GCVIEW_RAISE_ERROR(__str__) \
do { \
  Utils::raiseError((__str__), __FILE__, __LINE__); \
} while (false)

#if GCVIEW_ENABLE_ASSERT

#define GCVIEW_ASSERT(__cond__) \
do { \
  if (!(__cond__)) { \
    GCVIEW_RAISE_ERROR("assertion failure : " # __cond__); \
  } \
} while (false)

#else // GCVIEW_ENABLE_ASSERT

#define GCVIEW_ASSERT(__cond__) \
do { \
} while (false)

#endif // GCVIEW_ENABLE_ASSERT

#define GCVIEW_GUARANTEE(__cond__, __msg__) \
do { \
  if (!(__cond__)) { \
    GCVIEW_RAISE_ERROR("guarantee failure : " # __cond__ " (" __msg__ ")"); \
  } \
} while (false)

#define GCVIEW_ALLOC_GUARANTEE(__ptr__) \
do { \
  GCVIEW_GUARANTEE((__ptr__) != nullptr, "allocation failed"); \
} while (false)

#define GCVIEW_UNREACHABLE(__msg__) \
do { \
  GCVIEW_RAISE_ERROR("should not reach here : " __msg__); \
} while (false)

#define GCVIEW_UNREACHABLE_NULL(__msg__) \
do { \
  GCVIEW_UNREACHABLE(__msg__); \
  return nullptr; \
} while (false)

#define GCVIEW_UNREACHABLE_0(__msg__) \
do { \
  GCVIEW_UNREACHABLE(__msg__); \
  return 0; \
} while (false)

#define GCVIEW_UNREACHABLE_BREAK(__msg__) \
do { \
  GCVIEW_UNREACHABLE(__msg__); \
  break; \
} while (false)

namespace art {
namespace gc {
namespace gcview {

class Utils {
public:
  static const char* cloneStr(const char* str) {
    if (str != nullptr) {
      size_t len = strlen(str);
      if (len > 0) {
        const char* ret = new char[len + 1];
        GCVIEW_ALLOC_GUARANTEE(ret);
        strcpy((char*) ret, str);
        return ret;
      } else {
        return nullptr;
      }
    } else {
      return nullptr;
    }
  }

  static const char* getStrOrEmptyStr(const char* str) {
    return (str != nullptr) ? str : "";
  }

  static bool areStrsEqual(const char* str0, const char* str1) {
    const char* s0 = getStrOrEmptyStr(str0);
    const char* s1 = getStrOrEmptyStr(str1);
    return strcmp(s0, s1) == 0;
  }

  static void formatStr(char* buffer, size_t buffer_size,
                        const char* format, ...);

  static void raiseError(const char* str, const char* file, unsigned line);
};

class ScopePrinter {
private:
  const char* _class_name;
  const char* _method_name;

  void printScope(bool start) {
    printf("%s %s::%s\n", (start) ? ">>" : "<<",
           Utils::getStrOrEmptyStr(_class_name),
           Utils::getStrOrEmptyStr(_method_name));
  }

public:
  ScopePrinter(const char* class_name, const char* method_name)
      : _class_name(class_name), _method_name(method_name) {
    printScope(true);
  }

  ~ScopePrinter() {
    printScope(false);
  }
};

}    // namespace gcview
}    // namespace gc
}    // namespace art

#endif // ART_RUNTIME_GC_GCVIEW_UTILS_H_
