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
	.global	__aeabi_f2d
	.global	__aeabi_dadd
	.global	__aeabi_i2d
	.global	__aeabi_ddiv
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble
	.type	Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble, %function
Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble:
	.fnstart
.LFB234:
	@ args = 8, pretend = 0, frame = 24
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {r4, r5, fp, lr}
	add	fp, sp, #12
	sub	sp, sp, #24
	str	r0, [fp, #-24]
	str	r1, [fp, #-28]
	strd	r2, [fp, #-36]
	ldr	r0, [fp, #4]	@ float
	bl	__aeabi_f2d(PLT)
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	ldrd	r2, [fp, #-36]
	bl	__aeabi_dadd(PLT)
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	ldr	r3, [fp, #8]
	sub	r3, r3, #5
	mov	r0, r3
	bl	__aeabi_i2d(PLT)
	mov	r2, r0
	mov	r3, r1
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_ddiv(PLT)
	mov	r2, r0
	mov	r3, r1
	strd	r2, [fp, #-20]
	ldrd	r2, [fp, #-20]
	mov	r0, r2
	mov	r1, r3
	sub	sp, fp, #12
	@ sp needed
	ldmfd	sp!, {r4, r5, fp, pc}
	.cantunwind
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble, .-Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
