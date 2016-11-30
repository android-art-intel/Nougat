/*
 * Copyright (C) 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "OptimizationTests_AutoFastJNIDetection_Mutex_Main.h"
#include <pthread.h>

pthread_mutex_t count_mutex;
int count = 5;

extern "C" JNIEXPORT jint JNICALL Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex (JNIEnv *env, jobject myobject, jint a) {
      pthread_mutex_lock(&count_mutex);
      count = count + 1;
      pthread_mutex_unlock(&count_mutex);
    return a + count;

    }

