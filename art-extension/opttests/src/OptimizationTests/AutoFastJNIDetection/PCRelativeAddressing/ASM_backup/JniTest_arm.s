#
# Copyright (C) 2016 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

	.arch armv5te
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 6
	.eabi_attribute 34, 0
	.eabi_attribute 18, 4
	.file	"JniTest.cpp"
	.local	_ZL10global_var
	.comm	_ZL10global_var,4,4
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal
	.type	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal, %function
Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal:
	.fnstart
.LFB234:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #20
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	str	r2, [fp, #-16]
	ldr	r3, .L2
.LPIC0:
	add	r3, pc, r3
	ldr	r2, [fp, #-16]
	str	r2, [r3]
	sub	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
.L3:
	.align	2
.L2:
	.word	_ZL10global_var-(.LPIC0+8)
	.cantunwind
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal, .-Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal
	.type	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal, %function
Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal:
	.fnstart
.LFB235:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	ldr	r3, .L6
.LPIC1:
	add	r3, pc, r3
	ldr	r3, [r3]
	mov	r0, r3
	sub	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
.L7:
	.align	2
.L6:
	.word	_ZL10global_var-(.LPIC1+8)
	.cantunwind
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal, .-Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
