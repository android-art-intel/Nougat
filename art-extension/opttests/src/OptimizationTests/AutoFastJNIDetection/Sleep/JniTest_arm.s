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
	.global	Java_OptimizationTests_AutoFastJNIDetection_Sleep_Main_nativeSleep
	.type	Java_OptimizationTests_AutoFastJNIDetection_Sleep_Main_nativeSleep, %function
Java_OptimizationTests_AutoFastJNIDetection_Sleep_Main_nativeSleep:
	.fnstart
.LFB245:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	.save {fp, lr}
	.setfp fp, sp, #4
	add	fp, sp, #4
	.pad #16
	sub	sp, sp, #16
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	str	r2, [fp, #-16]
	ldr	r3, [fp, #-16]
	cmp	r3, #4
	bgt	.L2
	mov	r0, #1
	bl	sleep(PLT)
.L2:
	ldr	r3, [fp, #-16]
	add	r3, r3, #1
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_Sleep_Main_nativeSleep, .-Java_OptimizationTests_AutoFastJNIDetection_Sleep_Main_nativeSleep
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
