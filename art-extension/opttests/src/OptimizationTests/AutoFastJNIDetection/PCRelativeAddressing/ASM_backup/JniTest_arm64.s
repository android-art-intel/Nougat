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
	.local	_ZL10global_var
	.comm	_ZL10global_var,4,4
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal
	.type	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal, %function
Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal:
.LFB237:
	.cfi_startproc
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	str	x0, [sp,24]
	str	x1, [sp,16]
	str	w2, [sp,12]
	adrp	x0, _ZL10global_var
	add	x0, x0, :lo12:_ZL10global_var
	ldr	w1, [sp,12]
	str	w1, [x0]
	add	sp, sp, 32
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal, .-Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal
	.type	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal, %function
Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal:
.LFB238:
	.cfi_startproc
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	x0, [sp,8]
	str	x1, [sp]
	adrp	x0, _ZL10global_var
	add	x0, x0, :lo12:_ZL10global_var
	ldr	w0, [x0]
	add	sp, sp, 16
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE238:
	.size	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal, .-Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
