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
	.global	Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching
	.type	Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching, %function
Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching:
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
	b	.L7
.L3:
	ldr	w1, [sp,12]
	ldr	w0, [sp,8]
	mul	w1, w1, w0
	ldr	w0, [sp,4]
	cmp	w1, w0
	bne	.L5
	ldr	w0, [sp,12]
	str	w0, [sp,4]
	b	.L7
.L5:
	ldr	w1, [sp,12]
	ldr	w0, [sp,4]
	cmp	w1, w0
	beq	.L6
	ldr	w0, [sp,8]
	add	w0, w0, 1
	str	w0, [sp,12]
	b	.L7
.L6:
	str	wzr, [sp,8]
	b	.L7
.L2:
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
	ldr	w0, [sp,8]
	add	w1, w1, w0
	ldr	w0, [sp,4]
	add	w0, w1, w0
	add	sp, sp, 32
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching, .-Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
