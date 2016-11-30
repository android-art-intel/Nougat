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

#include "OptimizationTests_AutoFastJNIDetection_Fopen_Main.h"
#include<stdio.h>
#include<stdlib.h>

extern "C" JNIEXPORT jint JNICALL Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen (JNIEnv *env, jobject myobject, jint a) {
    FILE * fp;
    fp = fopen ("file.txt", "w+");
    fprintf(fp, "%s %s %s %d", "We", "are", "in", 2016);
    fclose(fp);

/*
    fopen("file.txt","r");
    int ch;
    ch = fgetc(fp);
    if( feof(fp) )
    { 
        fclose(fp);
        return -1 ;
    }
    printf("%c", ch);

    ch = fgetc(fp);
    if( feof(fp) )
    { 
        fclose(fp);
        return -2;
    }
    printf("%c", ch);
    fclose(fp);
    
*/   
    return a;

    }

