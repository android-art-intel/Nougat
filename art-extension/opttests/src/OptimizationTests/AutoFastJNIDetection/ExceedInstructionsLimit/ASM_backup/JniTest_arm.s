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
	.global	Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit
	.type	Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit, %function
Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit:
	.fnstart
.LFB234:
	@ args = 4, pretend = 0, frame = 136
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #140
	str	r0, [fp, #-128]
	str	r1, [fp, #-132]
	str	r2, [fp, #-136]
	str	r3, [fp, #-140]
	ldr	r3, [fp, #-136]
	add	r3, r3, #1
	str	r3, [fp, #-8]
	ldr	r3, [fp, #-8]
	add	r2, r3, #2
	ldr	r3, [fp, #-140]
	add	r3, r2, r3
	str	r3, [fp, #-12]
	ldr	r3, [fp, #-12]
	add	r2, r3, #3
	ldr	r3, [fp, #4]
	add	r3, r2, r3
	str	r3, [fp, #-16]
	ldr	r3, [fp, #-8]
	sub	r2, r3, #4
	ldr	r3, [fp, #-140]
	add	r3, r2, r3
	str	r3, [fp, #-20]
	ldr	r3, [fp, #-16]
	sub	r2, r3, #4
	ldr	r3, [fp, #-136]
	ldr	r1, [fp, #-140]
	mul	r3, r1, r3
	rsb	r3, r3, r2
	str	r3, [fp, #-24]
	ldr	r3, [fp, #-16]
	ldr	r2, [fp, #-12]
	mul	r3, r2, r3
	sub	r2, r3, #4
	ldr	r3, [fp, #-136]
	ldr	r1, [fp, #-140]
	mul	r3, r1, r3
	rsb	r3, r3, r2
	str	r3, [fp, #-28]
	ldr	r3, [fp, #-16]
	ldr	r2, [fp, #-8]
	mul	r1, r2, r3
	ldr	r2, [fp, #-140]
	mov	r3, r2
	mov	r3, r3, asl #30
	rsb	r3, r2, r3
	mov	r3, r3, asl #2
	add	r2, r1, r3
	ldr	r3, [fp, #-136]
	ldr	r1, [fp, #-140]
	mul	r3, r1, r3
	rsb	r3, r3, r2
	str	r3, [fp, #-32]
	ldr	r3, [fp, #-16]
	ldr	r2, [fp, #-12]
	mul	r1, r2, r3
	ldr	r2, [fp, #4]
	mov	r3, r2
	mov	r3, r3, asl #30
	rsb	r3, r2, r3
	mov	r3, r3, asl #2
	add	r1, r1, r3
	ldr	r2, [fp, #-136]
	mov	r3, r2
	mov	r3, r3, asl #30
	rsb	r3, r2, r3
	mov	r3, r3, asl #2
	add	r3, r1, r3
	str	r3, [fp, #-36]
	ldr	r3, [fp, #-32]
	ldr	r2, [fp, #-8]
	mul	r1, r2, r3
	ldr	r2, [fp, #-136]
	mov	r3, r2
	mov	r2, r2, asl #2
	rsb	r3, r2, r3
	mov	r3, r3, asl #1
	add	r1, r1, r3
	ldr	r2, [fp, #-140]
	mov	r3, r2
	mov	r3, r3, asl #30
	rsb	r3, r2, r3
	mov	r3, r3, asl #2
	rsb	r3, r2, r3
	add	r3, r1, r3
	str	r3, [fp, #-40]
	ldr	r2, [fp, #-32]
	ldr	r3, [fp, #-28]
	rsb	r2, r3, r2
	ldr	r3, [fp, #-24]
	rsb	r3, r3, r2
	str	r3, [fp, #-44]
	ldr	r3, [fp, #-140]
	add	r3, r3, #1
	str	r3, [fp, #-48]
	ldr	r3, [fp, #-48]
	add	r2, r3, #2
	ldr	r3, [fp, #-140]
	add	r3, r2, r3
	str	r3, [fp, #-52]
	ldr	r3, [fp, #-52]
	add	r2, r3, #3
	ldr	r3, [fp, #4]
	add	r3, r2, r3
	str	r3, [fp, #-56]
	ldr	r3, [fp, #-48]
	sub	r2, r3, #4
	ldr	r3, [fp, #-140]
	add	r3, r2, r3
	str	r3, [fp, #-60]
	ldr	r3, [fp, #-56]
	sub	r2, r3, #4
	ldr	r3, [fp, #-140]
	ldr	r1, [fp, #-140]
	mul	r3, r1, r3
	rsb	r3, r3, r2
	str	r3, [fp, #-64]
	ldr	r3, [fp, #-56]
	ldr	r2, [fp, #-52]
	mul	r3, r2, r3
	sub	r2, r3, #4
	ldr	r3, [fp, #-140]
	ldr	r1, [fp, #-140]
	mul	r3, r1, r3
	rsb	r3, r3, r2
	str	r3, [fp, #-68]
	ldr	r3, [fp, #-56]
	ldr	r2, [fp, #-48]
	mul	r1, r2, r3
	ldr	r2, [fp, #-140]
	mov	r3, r2
	mov	r3, r3, asl #30
	rsb	r3, r2, r3
	mov	r3, r3, asl #2
	add	r2, r1, r3
	ldr	r3, [fp, #-140]
	ldr	r1, [fp, #-140]
	mul	r3, r1, r3
	rsb	r3, r3, r2
	str	r3, [fp, #-72]
	ldr	r3, [fp, #-56]
	ldr	r2, [fp, #-52]
	mul	r1, r2, r3
	ldr	r2, [fp, #4]
	mov	r3, r2
	mov	r3, r3, asl #30
	rsb	r3, r2, r3
	mov	r3, r3, asl #2
	add	r1, r1, r3
	ldr	r2, [fp, #-140]
	mov	r3, r2
	mov	r3, r3, asl #30
	rsb	r3, r2, r3
	mov	r3, r3, asl #2
	add	r3, r1, r3
	str	r3, [fp, #-76]
	ldr	r3, [fp, #-72]
	ldr	r2, [fp, #-48]
	mul	r1, r2, r3
	ldr	r2, [fp, #-140]
	mov	r3, r2
	mov	r2, r2, asl #2
	rsb	r3, r2, r3
	mov	r3, r3, asl #1
	add	r1, r1, r3
	ldr	r2, [fp, #-140]
	mov	r3, r2
	mov	r3, r3, asl #30
	rsb	r3, r2, r3
	mov	r3, r3, asl #2
	rsb	r3, r2, r3
	add	r3, r1, r3
	str	r3, [fp, #-80]
	ldr	r2, [fp, #-72]
	ldr	r3, [fp, #-68]
	rsb	r2, r3, r2
	ldr	r3, [fp, #-64]
	rsb	r3, r3, r2
	str	r3, [fp, #-84]
	ldr	r3, [fp, #4]
	add	r3, r3, #1
	str	r3, [fp, #-88]
	ldr	r3, [fp, #-88]
	add	r2, r3, #2
	ldr	r3, [fp, #-140]
	add	r3, r2, r3
	str	r3, [fp, #-92]
	ldr	r3, [fp, #-92]
	add	r2, r3, #3
	ldr	r3, [fp, #4]
	add	r3, r2, r3
	str	r3, [fp, #-96]
	ldr	r3, [fp, #-88]
	sub	r2, r3, #4
	ldr	r3, [fp, #-140]
	add	r3, r2, r3
	str	r3, [fp, #-100]
	ldr	r3, [fp, #-96]
	sub	r2, r3, #4
	ldr	r3, [fp, #4]
	ldr	r1, [fp, #-140]
	mul	r3, r1, r3
	rsb	r3, r3, r2
	str	r3, [fp, #-104]
	ldr	r3, [fp, #-96]
	ldr	r2, [fp, #-92]
	mul	r3, r2, r3
	sub	r2, r3, #4
	ldr	r3, [fp, #4]
	ldr	r1, [fp, #-140]
	mul	r3, r1, r3
	rsb	r3, r3, r2
	str	r3, [fp, #-108]
	ldr	r3, [fp, #-96]
	ldr	r2, [fp, #-88]
	mul	r1, r2, r3
	ldr	r2, [fp, #-140]
	mov	r3, r2
	mov	r3, r3, asl #30
	rsb	r3, r2, r3
	mov	r3, r3, asl #2
	add	r2, r1, r3
	ldr	r3, [fp, #4]
	ldr	r1, [fp, #-140]
	mul	r3, r1, r3
	rsb	r3, r3, r2
	str	r3, [fp, #-112]
	ldr	r3, [fp, #-96]
	ldr	r2, [fp, #-92]
	mul	r1, r2, r3
	ldr	r2, [fp, #4]
	mov	r3, r2
	mov	r3, r3, asl #30
	rsb	r3, r2, r3
	mov	r3, r3, asl #2
	add	r1, r1, r3
	ldr	r2, [fp, #4]
	mov	r3, r2
	mov	r3, r3, asl #30
	rsb	r3, r2, r3
	mov	r3, r3, asl #2
	add	r3, r1, r3
	str	r3, [fp, #-116]
	ldr	r3, [fp, #-112]
	ldr	r2, [fp, #-88]
	mul	r1, r2, r3
	ldr	r2, [fp, #4]
	mov	r3, r2
	mov	r2, r2, asl #2
	rsb	r3, r2, r3
	mov	r3, r3, asl #1
	add	r1, r1, r3
	ldr	r2, [fp, #-140]
	mov	r3, r2
	mov	r3, r3, asl #30
	rsb	r3, r2, r3
	mov	r3, r3, asl #2
	rsb	r3, r2, r3
	add	r3, r1, r3
	str	r3, [fp, #-120]
	ldr	r2, [fp, #-112]
	ldr	r3, [fp, #-108]
	rsb	r2, r3, r2
	ldr	r3, [fp, #-104]
	rsb	r3, r3, r2
	str	r3, [fp, #-124]
	ldr	r2, [fp, #-8]
	ldr	r3, [fp, #-12]
	add	r2, r2, r3
	ldr	r3, [fp, #-16]
	add	r2, r2, r3
	ldr	r3, [fp, #-20]
	add	r2, r2, r3
	ldr	r3, [fp, #-24]
	add	r2, r2, r3
	ldr	r3, [fp, #-28]
	add	r2, r2, r3
	ldr	r3, [fp, #-32]
	add	r2, r2, r3
	ldr	r3, [fp, #-36]
	add	r2, r2, r3
	ldr	r3, [fp, #-40]
	add	r2, r2, r3
	ldr	r3, [fp, #-44]
	add	r2, r2, r3
	ldr	r3, [fp, #-48]
	add	r2, r2, r3
	ldr	r3, [fp, #-52]
	add	r2, r2, r3
	ldr	r3, [fp, #-56]
	add	r2, r2, r3
	ldr	r3, [fp, #-60]
	add	r2, r2, r3
	ldr	r3, [fp, #-64]
	add	r2, r2, r3
	ldr	r3, [fp, #-68]
	add	r2, r2, r3
	ldr	r3, [fp, #-72]
	add	r2, r2, r3
	ldr	r3, [fp, #-76]
	add	r2, r2, r3
	ldr	r3, [fp, #-80]
	add	r2, r2, r3
	ldr	r3, [fp, #-84]
	add	r2, r2, r3
	ldr	r3, [fp, #-88]
	add	r2, r2, r3
	ldr	r3, [fp, #-92]
	add	r2, r2, r3
	ldr	r3, [fp, #-96]
	add	r2, r2, r3
	ldr	r3, [fp, #-100]
	add	r2, r2, r3
	ldr	r3, [fp, #-104]
	add	r2, r2, r3
	ldr	r3, [fp, #-108]
	add	r2, r2, r3
	ldr	r3, [fp, #-112]
	add	r2, r2, r3
	ldr	r3, [fp, #-116]
	add	r2, r2, r3
	ldr	r3, [fp, #-120]
	add	r2, r2, r3
	ldr	r3, [fp, #-124]
	add	r3, r2, r3
	mov	r0, r3
	sub	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.cantunwind
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit, .-Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
