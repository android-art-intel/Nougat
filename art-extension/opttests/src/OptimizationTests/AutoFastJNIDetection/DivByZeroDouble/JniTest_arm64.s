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
	.global	Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble
	.type	Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble, %function
Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble:
.LFB237:
	.cfi_startproc
	sub	sp, sp, #48
	.cfi_def_cfa_offset 48
	str	x0, [sp,24]
	str	x1, [sp,16]
	str	d0, [sp,8]
	str	s1, [sp,4]
	str	w2, [sp]
	ldr	w0, [sp,4]
	fmov	s0, w0
	fcvt	d0, s0
	fmov	x1, d0
	ldr	x0, [sp,8]
	fmov	d0, x1
	fmov	d1, x0
	fadd	d0, d0, d1
	fmov	x1, d0
	ldr	w0, [sp]
	sub	w0, w0, #5
	scvtf	d0, w0
	fmov	x0, d0
	fmov	d0, x1
	fmov	d1, x0
	fdiv	d0, d0, d1
	fmov	x0, d0
	str	x0, [sp,40]
	ldr	x0, [sp,40]
	fmov	d0, x0
	add	sp, sp, 48
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble, .-Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
