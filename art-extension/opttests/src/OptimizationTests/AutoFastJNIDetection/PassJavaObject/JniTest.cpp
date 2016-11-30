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

#include "OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main.h"

// Pass java object to c
extern "C" JNIEXPORT jdouble JNICALL Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj(JNIEnv *env, jobject, jobject aObject) {
    jclass aClass = env->GetObjectClass(aObject);
    jfieldID iValueId = env->GetFieldID(aClass, "iValue", "I");
    int iValue = env -> GetIntField(aObject, iValueId);
    jfieldID fValueId = env->GetFieldID(aClass, "fValue", "F");
    float fValue = env -> GetFloatField(aObject, fValueId);
    jfieldID dValueId = env->GetFieldID(aClass, "dValue", "D");
    double dValue = env -> GetDoubleField(aObject, dValueId);


/*
    jmethodID getIValue = env->GetMethodID(aClass, "getIValue", "()I");
    int iValue =  (*env)->CallIntMethod(env, aObject, getIValue);
    jmethodID getFValue = (*env)->GetMethodID(env, aClass, "getFValue", "()F");
    float fValue =  (*env)->CallIntMethod(env, aObject, getFValue);
    jmethodID getDValue = (*env)->GetMethodID(env, aClass, "getDValue", "()D");
    double dValue =  (*env)->CallIntMethod(env, aObject, getDValue);
*/
    return iValue + fValue + dValue;
    }

