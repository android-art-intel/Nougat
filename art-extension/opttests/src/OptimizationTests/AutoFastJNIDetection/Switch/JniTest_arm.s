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
	.global	Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch
	.type	Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch, %function
Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch:
	.fnstart
.LFB234:
	@ args = 4, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #20
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	str	r2, [fp, #-16]
	str	r3, [fp, #-20]
	ldr	r3, [fp, #-16]
	add	r3, r3, #1
	cmp	r3, #5
	addls	pc, pc, r3, asl #2
	b	.L2
.L4:
	b	.L3
	b	.L5
	b	.L6
	b	.L7
	b	.L8
	b	.L9
.L3:
	mov	r3, #10
	str	r3, [fp, #-20]
	b	.L10
.L5:
	ldr	r3, [fp, #-20]
	add	r3, r3, #1
	str	r3, [fp, #-20]
	b	.L10
.L6:
	ldr	r2, [fp, #-20]
	ldr	r3, [fp, #4]
	add	r3, r2, r3
	str	r3, [fp, #-20]
	b	.L10
.L7:
	ldr	r2, [fp, #-20]
	ldr	r3, [fp, #4]
	rsb	r3, r3, r2
	str	r3, [fp, #-20]
	ldr	r3, [fp, #4]
	add	r3, r3, #5
	str	r3, [fp, #4]
	b	.L10
.L8:
	ldr	r2, [fp, #-20]
	ldr	r3, [fp, #4]
	rsb	r3, r3, r2
	str	r3, [fp, #-20]
	ldr	r3, [fp, #4]
	add	r3, r3, #6
	str	r3, [fp, #4]
	b	.L10
.L9:
	ldr	r2, [fp, #-20]
	ldr	r3, [fp, #4]
	rsb	r3, r3, r2
	str	r3, [fp, #-20]
	ldr	r3, [fp, #4]
	add	r3, r3, #6
	str	r3, [fp, #4]
	b	.L10
.L2:
	ldr	r2, [fp, #-20]
	ldr	r3, [fp, #4]
	rsb	r3, r3, r2
	str	r3, [fp, #-20]
	mov	r0, r0	@ nop
.L10:
	ldr	r3, [fp, #-20]
	mov	r0, r3
	sub	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.cantunwind
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch, .-Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
