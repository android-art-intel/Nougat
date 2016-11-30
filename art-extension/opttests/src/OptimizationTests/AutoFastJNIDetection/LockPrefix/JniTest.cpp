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

#include "OptimizationTests_AutoFastJNIDetection_LockPrefix_Main.h"

volatile int count = 5;

/* IMPORTANT: lock prefix is inserted to asm code MANUALLY :
   xchgl -> lock xchgl
*/
extern "C" JNIEXPORT jlong JNICALL Java_OptimizationTests_AutoFastJNIDetection_LockPrefix_Main_nativeLockPrefix (JNIEnv *env, jobject myobject, jlong a) {
//    return a + count + __sync_lock_test_and_set(&count, 1);
    return a + count + __sync_fetch_and_add(&count, 1);
}

