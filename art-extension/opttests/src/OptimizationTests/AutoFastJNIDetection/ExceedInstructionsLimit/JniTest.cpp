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

#include "OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main.h"

extern "C" JNIEXPORT jint JNICALL Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit (JNIEnv *env, jobject myobject, jint a, jint b, jint c) {

    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    int c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;

    a1 = a + 1;
    a2 = a1 + 2 + b;
    a3 = a2 + 3 + c;
    a4 = a1 - 4 + b;
    a5 = a3 - 4 - a*b;
    a6 = a3*a2 - 4 - a*b;
    a7 = a3*a1 - 4*b - a*b;
    a8 = a3*a2 - 4*c - a*4;
    a9 = a7*a1 - 6*a - b*5;
    a10 = a7 - a6 - a5;

    b1 = b + 1;
    b2 = b1 + 2 + b;
    b3 = b2 + 3 + c;
    b4 = b1 - 4 + b;
    b5 = b3 - 4 - b*b;
    b6 = b3*b2 - 4 - b*b;
    b7 = b3*b1 - 4*b - b*b;
    b8 = b3*b2 - 4*c - b*4;
    b9 = b7*b1 - 6*b - b*5;
    b10 = b7 - b6 - b5;

    c1 = c + 1;
    c2 = c1 + 2 + b;
    c3 = c2 + 3 + c;
    c4 = c1 - 4 + b;
    c5 = c3 - 4 - c*b;
    c6 = c3*c2 - 4 - c*b;
    c7 = c3*c1 - 4*b - c*b;
    c8 = c3*c2 - 4*c - c*4;
    c9 = c7*c1 - 6*c - b*5;
    c10 = c7 - c6 - c5;

 
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +  c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9 + c10;
    }

