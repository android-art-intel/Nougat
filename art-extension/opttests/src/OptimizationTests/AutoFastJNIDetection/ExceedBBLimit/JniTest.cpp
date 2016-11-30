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

#include "OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main.h"

extern "C" JNIEXPORT jint JNICALL Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit (JNIEnv *env, jobject myobject, jint a, jint b, jint c) {
     if (a > b) {
         if ( c - b > a) {
             a += b;
         } else {
             if ( a*b == c) {
                 c = a;
             } 
         }
     } else {
         if (a != b ) {
             a *= b;
         } else {
             b *= a;
         }
     }
     if (a != b) {
         if ( c - b >= a) {
             a = b;
         } 
     } else {
         if (a != b ) {
             a *= b;
         } else {
             b *= a;
         }
     }
     if (a != c) {
         a = b + 1;
     } else {
         b = 0;
     }
     if ( a*b == c) {
         c = a;
     } else {
         if (a != c) {
             a = b + 5;
         } else {
             b = 0;
         }
     }


     if (a < 1) {
         a += 1;
     } else {
         b +=2;
     }

     if (a > 13) {
         a += 13;
     } else {
         b += 5;
     }
     if (c > 13) {
         c += 13;
     } else {
         c += 5;
     }
     if (b < 1) {
         b += 1;
     } else {
         b +=2;
     }

     if (b > 13) {
         b += 13;
     } else {
         b += 5;
     }
     if (c > 13) {
         c += 13;
     } else {
         c += 5;
     }

    if (c + a != c) {
        c -= a;
    } else {
        c -= c;
    }
    if (b > c) {
         if ( c - c > b) {
             b += c;
         } else {
             if ( b*c == c) {
                 c = b;
             } 
         }
     } else {
         if (b != c ) {
             b *= c;
         } else {
             c *= b;
         }
     }

    if (a < 100) {
        a *= 2;
    } else {
        a += 2;
    }
   
    if (b < 1001) {
        b *= 2;
    } else {
        b += 2;
    }

    if (c < 10) {
        c *= 2;
    } else {
        c += 2;
    }
 
    return a + b + c;
    }

