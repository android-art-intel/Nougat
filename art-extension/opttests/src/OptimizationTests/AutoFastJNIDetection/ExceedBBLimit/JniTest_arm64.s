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

	.cpu generic+fp+simd
	.file	"JniTest.cpp"
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit
	.type	Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit, %function
Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit:
.LFB237:
	.cfi_startproc
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	str	x0, [sp,24]
	str	x1, [sp,16]
	str	w2, [sp,12]
	str	w3, [sp,8]
	str	w4, [sp,4]
	ldr	w1, [sp,12]
	ldr	w0, [sp,8]
	cmp	w1, w0
	ble	.L2
	ldr	w1, [sp,4]
	ldr	w0, [sp,8]
	sub	w1, w1, w0
	ldr	w0, [sp,12]
	cmp	w1, w0
	ble	.L3
	ldr	w1, [sp,12]
	ldr	w0, [sp,8]
	add	w0, w1, w0
	str	w0, [sp,12]
	b	.L4
.L3:
	ldr	w1, [sp,12]
	ldr	w0, [sp,8]
	mul	w1, w1, w0
	ldr	w0, [sp,4]
	cmp	w1, w0
	bne	.L4
	ldr	w0, [sp,12]
	str	w0, [sp,4]
	b	.L4
.L2:
	ldr	w1, [sp,12]
	ldr	w0, [sp,8]
	cmp	w1, w0
	beq	.L5
	ldr	w1, [sp,12]
	ldr	w0, [sp,8]
	mul	w0, w1, w0
	str	w0, [sp,12]
	b	.L4
.L5:
	ldr	w1, [sp,8]
	ldr	w0, [sp,12]
	mul	w0, w1, w0
	str	w0, [sp,8]
.L4:
	ldr	w1, [sp,12]
	ldr	w0, [sp,8]
	cmp	w1, w0
	beq	.L6
	ldr	w1, [sp,4]
	ldr	w0, [sp,8]
	sub	w1, w1, w0
	ldr	w0, [sp,12]
	cmp	w1, w0
	blt	.L7
	ldr	w0, [sp,8]
	str	w0, [sp,12]
	b	.L7
.L6:
	ldr	w1, [sp,12]
	ldr	w0, [sp,8]
	cmp	w1, w0
	beq	.L8
	ldr	w1, [sp,12]
	ldr	w0, [sp,8]
	mul	w0, w1, w0
	str	w0, [sp,12]
	b	.L7
.L8:
	ldr	w1, [sp,8]
	ldr	w0, [sp,12]
	mul	w0, w1, w0
	str	w0, [sp,8]
.L7:
	ldr	w1, [sp,12]
	ldr	w0, [sp,4]
	cmp	w1, w0
	beq	.L9
	ldr	w0, [sp,8]
	add	w0, w0, 1
	str	w0, [sp,12]
	b	.L10
.L9:
	str	wzr, [sp,8]
.L10:
	ldr	w1, [sp,12]
	ldr	w0, [sp,8]
	mul	w1, w1, w0
	ldr	w0, [sp,4]
	cmp	w1, w0
	bne	.L11
	ldr	w0, [sp,12]
	str	w0, [sp,4]
	b	.L12
.L11:
	ldr	w1, [sp,12]
	ldr	w0, [sp,4]
	cmp	w1, w0
	beq	.L13
	ldr	w0, [sp,8]
	add	w0, w0, 5
	str	w0, [sp,12]
	b	.L12
.L13:
	str	wzr, [sp,8]
.L12:
	ldr	w0, [sp,12]
	cmp	w0, wzr
	bgt	.L14
	ldr	w0, [sp,12]
	add	w0, w0, 1
	str	w0, [sp,12]
	b	.L15
.L14:
	ldr	w0, [sp,8]
	add	w0, w0, 2
	str	w0, [sp,8]
.L15:
	ldr	w0, [sp,12]
	cmp	w0, 13
	ble	.L16
	ldr	w0, [sp,12]
	add	w0, w0, 13
	str	w0, [sp,12]
	b	.L17
.L16:
	ldr	w0, [sp,8]
	add	w0, w0, 5
	str	w0, [sp,8]
.L17:
	ldr	w0, [sp,4]
	cmp	w0, 13
	ble	.L18
	ldr	w0, [sp,4]
	add	w0, w0, 13
	str	w0, [sp,4]
	b	.L19
.L18:
	ldr	w0, [sp,4]
	add	w0, w0, 5
	str	w0, [sp,4]
.L19:
	ldr	w0, [sp,8]
	cmp	w0, wzr
	bgt	.L20
	ldr	w0, [sp,8]
	add	w0, w0, 1
	str	w0, [sp,8]
	b	.L21
.L20:
	ldr	w0, [sp,8]
	add	w0, w0, 2
	str	w0, [sp,8]
.L21:
	ldr	w0, [sp,8]
	cmp	w0, 13
	ble	.L22
	ldr	w0, [sp,8]
	add	w0, w0, 13
	str	w0, [sp,8]
	b	.L23
.L22:
	ldr	w0, [sp,8]
	add	w0, w0, 5
	str	w0, [sp,8]
.L23:
	ldr	w0, [sp,4]
	cmp	w0, 13
	ble	.L24
	ldr	w0, [sp,4]
	add	w0, w0, 13
	str	w0, [sp,4]
	b	.L25
.L24:
	ldr	w0, [sp,4]
	add	w0, w0, 5
	str	w0, [sp,4]
.L25:
	ldr	w0, [sp,12]
	cmp	w0, wzr
	beq	.L26
	ldr	w1, [sp,4]
	ldr	w0, [sp,12]
	sub	w0, w1, w0
	str	w0, [sp,4]
	b	.L27
.L26:
	str	wzr, [sp,4]
.L27:
	ldr	w1, [sp,8]
	ldr	w0, [sp,4]
	cmp	w1, w0
	ble	.L28
	ldr	w0, [sp,8]
	cmp	w0, wzr
	bge	.L29
	ldr	w1, [sp,8]
	ldr	w0, [sp,4]
	add	w0, w1, w0
	str	w0, [sp,8]
	b	.L30
.L29:
	ldr	w1, [sp,8]
	ldr	w0, [sp,4]
	mul	w1, w1, w0
	ldr	w0, [sp,4]
	cmp	w1, w0
	bne	.L30
	ldr	w0, [sp,8]
	str	w0, [sp,4]
	b	.L30
.L28:
	ldr	w1, [sp,8]
	ldr	w0, [sp,4]
	cmp	w1, w0
	beq	.L31
	ldr	w1, [sp,8]
	ldr	w0, [sp,4]
	mul	w0, w1, w0
	str	w0, [sp,8]
	b	.L30
.L31:
	ldr	w1, [sp,4]
	ldr	w0, [sp,8]
	mul	w0, w1, w0
	str	w0, [sp,4]
.L30:
	ldr	w0, [sp,12]
	cmp	w0, 99
	bgt	.L32
	ldr	w0, [sp,12]
	lsl	w0, w0, 1
	str	w0, [sp,12]
	b	.L33
.L32:
	ldr	w0, [sp,12]
	add	w0, w0, 2
	str	w0, [sp,12]
.L33:
	ldr	w0, [sp,8]
	cmp	w0, 1000
	bgt	.L34
	ldr	w0, [sp,8]
	lsl	w0, w0, 1
	str	w0, [sp,8]
	b	.L35
.L34:
	ldr	w0, [sp,8]
	add	w0, w0, 2
	str	w0, [sp,8]
.L35:
	ldr	w0, [sp,4]
	cmp	w0, 9
	bgt	.L36
	ldr	w0, [sp,4]
	lsl	w0, w0, 1
	str	w0, [sp,4]
	b	.L37
.L36:
	ldr	w0, [sp,4]
	add	w0, w0, 2
	str	w0, [sp,4]
.L37:
	ldr	w1, [sp,12]
	ldr	w0, [sp,8]
	add	w1, w1, w0
	ldr	w0, [sp,4]
	add	w0, w1, w0
	add	sp, sp, 32
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit, .-Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
