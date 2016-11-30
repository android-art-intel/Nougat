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
	.global	Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching
	.type	Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching, %function
Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching:
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
	ldr	r2, [fp, #-16]
	ldr	r3, [fp, #-20]
	cmp	r2, r3
	ble	.L2
	ldr	r2, [fp, #4]
	ldr	r3, [fp, #-20]
	rsb	r2, r3, r2
	ldr	r3, [fp, #-16]
	cmp	r2, r3
	ble	.L3
	ldr	r2, [fp, #-16]
	ldr	r3, [fp, #-20]
	add	r3, r2, r3
	str	r3, [fp, #-16]
	b	.L7
.L3:
	ldr	r3, [fp, #-16]
	ldr	r2, [fp, #-20]
	mul	r2, r3, r2
	ldr	r3, [fp, #4]
	cmp	r2, r3
	bne	.L5
	ldr	r3, [fp, #-16]
	str	r3, [fp, #4]
	b	.L7
.L5:
	ldr	r2, [fp, #-16]
	ldr	r3, [fp, #4]
	cmp	r2, r3
	beq	.L6
	ldr	r3, [fp, #-20]
	add	r3, r3, #1
	str	r3, [fp, #-16]
	b	.L7
.L6:
	mov	r3, #0
	str	r3, [fp, #-20]
	b	.L7
.L2:
	ldr	r2, [fp, #-16]
	ldr	r3, [fp, #-20]
	cmp	r2, r3
	beq	.L8
	ldr	r3, [fp, #-16]
	ldr	r2, [fp, #-20]
	mul	r3, r2, r3
	str	r3, [fp, #-16]
	b	.L7
.L8:
	ldr	r3, [fp, #-20]
	ldr	r2, [fp, #-16]
	mul	r3, r2, r3
	str	r3, [fp, #-20]
.L7:
	ldr	r2, [fp, #-16]
	ldr	r3, [fp, #-20]
	add	r2, r2, r3
	ldr	r3, [fp, #4]
	add	r3, r2, r3
	mov	r0, r3
	sub	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.cantunwind
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching, .-Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
