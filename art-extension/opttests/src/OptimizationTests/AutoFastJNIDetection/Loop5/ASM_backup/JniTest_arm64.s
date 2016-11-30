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
	.global	Java_OptimizationTests_AutoFastJNIDetection_Loop5_Main_nativeLoop5
	.type	Java_OptimizationTests_AutoFastJNIDetection_Loop5_Main_nativeLoop5, %function
Java_OptimizationTests_AutoFastJNIDetection_Loop5_Main_nativeLoop5:
.LFB237:
	.cfi_startproc
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	str	x0, [sp,24]
	str	x1, [sp,16]
	str	w2, [sp,12]
	str	w3, [sp,8]
	str	w4, [sp,4]
.L2:
	ldr	w0, [sp,8]
	add	w0, w0, 1
	str	w0, [sp,8]
	ldr	w0, [sp,8]
	cmp	w0, 24
	ble	.L2
	ldr	w0, [sp,12]
	cmn	w0, #2
	bne	.L3
	b	.L4
.L3:
	ldr	w0, [sp,12]
	add	w0, w0, 1
	str	w0, [sp,12]
	ldr	w1, [sp,8]
	ldr	w0, [sp,12]
	add	w0, w1, w0
	str	w0, [sp,8]
.L4:
	ldr	w0, [sp,12]
	cmp	w0, 9
	ble	.L2
	ldr	w1, [sp,8]
	ldr	w0, [sp,4]
	sub	w0, w1, w0
	ldr	w1, [sp,12]
	add	w0, w1, w0
	str	w0, [sp,12]
	ldr	w1, [sp,8]
	ldr	w0, [sp,12]
	mul	w0, w1, w0
	ldr	w1, [sp,4]
	sub	w0, w1, w0
	add	sp, sp, 32
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Loop5_Main_nativeLoop5, .-Java_OptimizationTests_AutoFastJNIDetection_Loop5_Main_nativeLoop5
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
