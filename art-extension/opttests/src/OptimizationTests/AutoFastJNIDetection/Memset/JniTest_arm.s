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
	.global	Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset
	.type	Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset, %function
Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset:
	.fnstart
.LFB234:
	@ args = 0, pretend = 0, frame = 416
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #416
	str	r0, [fp, #-408]
	str	r1, [fp, #-412]
	str	r2, [fp, #-416]
	sub	r3, fp, #404
	mov	r0, r3
	ldr	r1, [fp, #-416]
	mov	r2, #400
	bl	memset(PLT)
	ldr	r3, [fp, #-384]
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
	.cantunwind
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset, .-Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
