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

#include "OptimizationTests_AutoFastJNIDetection_RecursiveCallMutual_Main.h"


int bar(int a, int b);

int foo(int a, int b) {
    if (a > 0) {
        return bar(a - 100, b + 5);
    } else {
      return b;
    }
}


int bar (int a, int b)  {
    if (a > 1) {
        return foo(a - 200, b - 2);
    } else {
        return b - 1;
    }
}
// Test add
extern "C" JNIEXPORT jint JNICALL Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallMutual_Main_nativeRecursiveCallMutual(JNIEnv *env, jobject myobject, jint a, jint b, jint c) {
    return foo(c, a + b);
}

