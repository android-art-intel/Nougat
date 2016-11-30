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
	.global	Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit
	.type	Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit, %function
Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit:
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
	b	.L4
.L3:
	ldr	r3, [fp, #-16]
	ldr	r2, [fp, #-20]
	mul	r2, r3, r2
	ldr	r3, [fp, #4]
	cmp	r2, r3
	bne	.L4
	ldr	r3, [fp, #-16]
	str	r3, [fp, #4]
	b	.L4
.L2:
	ldr	r2, [fp, #-16]
	ldr	r3, [fp, #-20]
	cmp	r2, r3
	beq	.L5
	ldr	r3, [fp, #-16]
	ldr	r2, [fp, #-20]
	mul	r3, r2, r3
	str	r3, [fp, #-16]
	b	.L4
.L5:
	ldr	r3, [fp, #-20]
	ldr	r2, [fp, #-16]
	mul	r3, r2, r3
	str	r3, [fp, #-20]
.L4:
	ldr	r2, [fp, #-16]
	ldr	r3, [fp, #-20]
	cmp	r2, r3
	beq	.L6
	ldr	r2, [fp, #4]
	ldr	r3, [fp, #-20]
	rsb	r2, r3, r2
	ldr	r3, [fp, #-16]
	cmp	r2, r3
	blt	.L7
	ldr	r3, [fp, #-20]
	str	r3, [fp, #-16]
	b	.L7
.L6:
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
	ldr	r3, [fp, #4]
	cmp	r2, r3
	beq	.L9
	ldr	r3, [fp, #-20]
	add	r3, r3, #1
	str	r3, [fp, #-16]
	b	.L10
.L9:
	mov	r3, #0
	str	r3, [fp, #-20]
.L10:
	ldr	r3, [fp, #-16]
	ldr	r2, [fp, #-20]
	mul	r2, r3, r2
	ldr	r3, [fp, #4]
	cmp	r2, r3
	bne	.L11
	ldr	r3, [fp, #-16]
	str	r3, [fp, #4]
	b	.L12
.L11:
	ldr	r2, [fp, #-16]
	ldr	r3, [fp, #4]
	cmp	r2, r3
	beq	.L13
	ldr	r3, [fp, #-20]
	add	r3, r3, #5
	str	r3, [fp, #-16]
	b	.L12
.L13:
	mov	r3, #0
	str	r3, [fp, #-20]
.L12:
	ldr	r3, [fp, #-16]
	cmp	r3, #0
	bgt	.L14
	ldr	r3, [fp, #-16]
	add	r3, r3, #1
	str	r3, [fp, #-16]
	b	.L15
.L14:
	ldr	r3, [fp, #-20]
	add	r3, r3, #2
	str	r3, [fp, #-20]
.L15:
	ldr	r3, [fp, #-16]
	cmp	r3, #13
	ble	.L16
	ldr	r3, [fp, #-16]
	add	r3, r3, #13
	str	r3, [fp, #-16]
	b	.L17
.L16:
	ldr	r3, [fp, #-20]
	add	r3, r3, #5
	str	r3, [fp, #-20]
.L17:
	ldr	r3, [fp, #4]
	cmp	r3, #13
	ble	.L18
	ldr	r3, [fp, #4]
	add	r3, r3, #13
	str	r3, [fp, #4]
	b	.L19
.L18:
	ldr	r3, [fp, #4]
	add	r3, r3, #5
	str	r3, [fp, #4]
.L19:
	ldr	r3, [fp, #-20]
	cmp	r3, #0
	bgt	.L20
	ldr	r3, [fp, #-20]
	add	r3, r3, #1
	str	r3, [fp, #-20]
	b	.L21
.L20:
	ldr	r3, [fp, #-20]
	add	r3, r3, #2
	str	r3, [fp, #-20]
.L21:
	ldr	r3, [fp, #-20]
	cmp	r3, #13
	ble	.L22
	ldr	r3, [fp, #-20]
	add	r3, r3, #13
	str	r3, [fp, #-20]
	b	.L23
.L22:
	ldr	r3, [fp, #-20]
	add	r3, r3, #5
	str	r3, [fp, #-20]
.L23:
	ldr	r3, [fp, #4]
	cmp	r3, #13
	ble	.L24
	ldr	r3, [fp, #4]
	add	r3, r3, #13
	str	r3, [fp, #4]
	b	.L25
.L24:
	ldr	r3, [fp, #4]
	add	r3, r3, #5
	str	r3, [fp, #4]
.L25:
	ldr	r3, [fp, #-16]
	cmp	r3, #0
	beq	.L26
	ldr	r2, [fp, #4]
	ldr	r3, [fp, #-16]
	rsb	r3, r3, r2
	str	r3, [fp, #4]
	b	.L27
.L26:
	mov	r3, #0
	str	r3, [fp, #4]
.L27:
	ldr	r2, [fp, #-20]
	ldr	r3, [fp, #4]
	cmp	r2, r3
	ble	.L28
	ldr	r3, [fp, #-20]
	cmp	r3, #0
	bge	.L29
	ldr	r2, [fp, #-20]
	ldr	r3, [fp, #4]
	add	r3, r2, r3
	str	r3, [fp, #-20]
	b	.L30
.L29:
	ldr	r3, [fp, #-20]
	ldr	r2, [fp, #4]
	mul	r2, r3, r2
	ldr	r3, [fp, #4]
	cmp	r2, r3
	bne	.L30
	ldr	r3, [fp, #-20]
	str	r3, [fp, #4]
	b	.L30
.L28:
	ldr	r2, [fp, #-20]
	ldr	r3, [fp, #4]
	cmp	r2, r3
	beq	.L31
	ldr	r3, [fp, #-20]
	ldr	r2, [fp, #4]
	mul	r3, r2, r3
	str	r3, [fp, #-20]
	b	.L30
.L31:
	ldr	r3, [fp, #4]
	ldr	r2, [fp, #-20]
	mul	r3, r2, r3
	str	r3, [fp, #4]
.L30:
	ldr	r3, [fp, #-16]
	cmp	r3, #99
	bgt	.L32
	ldr	r3, [fp, #-16]
	mov	r3, r3, asl #1
	str	r3, [fp, #-16]
	b	.L33
.L32:
	ldr	r3, [fp, #-16]
	add	r3, r3, #2
	str	r3, [fp, #-16]
.L33:
	ldr	r3, [fp, #-20]
	cmp	r3, #1000
	bgt	.L34
	ldr	r3, [fp, #-20]
	mov	r3, r3, asl #1
	str	r3, [fp, #-20]
	b	.L35
.L34:
	ldr	r3, [fp, #-20]
	add	r3, r3, #2
	str	r3, [fp, #-20]
.L35:
	ldr	r3, [fp, #4]
	cmp	r3, #9
	bgt	.L36
	ldr	r3, [fp, #4]
	mov	r3, r3, asl #1
	str	r3, [fp, #4]
	b	.L37
.L36:
	ldr	r3, [fp, #4]
	add	r3, r3, #2
	str	r3, [fp, #4]
.L37:
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
	.size	Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit, .-Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
