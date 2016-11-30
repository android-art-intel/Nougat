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

#include "OptimizationTests_AutoFastJNIDetection_Malloc_Main.h"
#include<stdio.h>
#include<stdlib.h>

/* IMPORTANT: in current implementation we don't use assembly inlined code, we always use C code below (see #else), see Makefile */
extern "C" JNIEXPORT jint JNICALL Java_OptimizationTests_AutoFastJNIDetection_Malloc_Main_nativeMalloc (JNIEnv *env, jobject myobject, jint a) {
#ifdef _MY_X86_64_HOST_
    __asm__(
            "pushq   %rbp;"
"            .cfi_def_cfa_offset 16;"
"            .cfi_offset 6, -16;"
"            movq    %rsp, %rbp;"
"            .cfi_def_cfa_register 6;"
"            subq    $48, %rsp;"
"            movq    %rdi, -24(%rbp);"
"            movq    %rsi, -32(%rbp);"
"            movl    %edx, -36(%rbp);"
"            movl    $4, %edi;"
"            call    malloc@PLT;"
"            movq    %rax, -16(%rbp);"
"            cmpq    $0, -16(%rbp);"
"            jne     .L2;"
"            movl    $-1, %eax;"
"            jmp     .L3;"
"            .L2:;"
"            movq    -16(%rbp), %rax;"
"            movl    -36(%rbp), %edx;"
"            movl    %edx, (%rax);"
"            movq    -16(%rbp), %rax;"
"            movl    (%rax), %eax;"
"            movl    %eax, -4(%rbp);"
"            movq    -16(%rbp), %rax;"
"            movq    %rax, %rdi;"
"            call    free@PLT;"
"            movl    -4(%rbp), %eax;"
"            .L3:;"
"            leave;"

       );
#else
    int *ptr;
    int b;
    ptr = (int*)malloc(sizeof(int));
    if (ptr == 0) {
        return -1;
    } else  {
        *ptr = a;
        b = *ptr;
        free(ptr);
        return b;
    }

#endif

    }

