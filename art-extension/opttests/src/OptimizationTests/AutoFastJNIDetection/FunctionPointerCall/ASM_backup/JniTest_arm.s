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
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall
	.type	Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall, %function
Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall:
	.fnstart
.LFB234:
	@ args = 4, pretend = 0, frame = 24
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	.save {fp, lr}
	.setfp fp, sp, #4
	add	fp, sp, #4
	.pad #24
	sub	sp, sp, #24
	str	r0, [fp, #-16]
	str	r1, [fp, #-20]
	str	r2, [fp, #-24]
	str	r3, [fp, #-28]
	ldr	r3, .L3
.LPIC0:
	add	r3, pc, r3
	str	r3, [fp, #-8]
	ldr	r3, [fp, #-8]
	mov	r0, #2
	mov	r1, #3
	mov	r2, #4
	blx	r3
	mov	r3, r0
	str	r3, [fp, #-12]
	ldr	r3, [fp, #-12]
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
.L4:
	.align	2
.L3:
	.word	addInt-(.LPIC0+8)
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall, .-Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall
	.align	2
	.global	addInt
	.hidden	addInt
	.type	addInt, %function
addInt:
	.fnstart
.LFB235:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #20
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	str	r2, [fp, #-16]
	ldr	r2, [fp, #-8]
	ldr	r3, [fp, #-12]
	add	r2, r2, r3
	ldr	r3, [fp, #-16]
	add	r3, r2, r3
	mov	r0, r3
	sub	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.cantunwind
	.fnend
	.size	addInt, .-addInt
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
