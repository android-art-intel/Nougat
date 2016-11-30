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

#include "OptimizationTests_AutoFastJNIDetection_VirtualCall_Main.h"

class A {

    public:
    virtual int getValue() {
        return 1;
    }
};

class B : public A {
    virtual int getValue() {
        return 100;
    }
};


extern "C" JNIEXPORT jint JNICALL Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall (JNIEnv *env, jobject myobject, jint a, jint b, jint c) {
       B obj_b;
       A &obj_a = obj_b;
       return obj_a.getValue();

    }

