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
	.global	Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall
	.type	Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall, %function
Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall:
.LFB237:
	.cfi_startproc
	stp	x29, x30, [sp, -64]!
	.cfi_def_cfa_offset 64
	.cfi_offset 29, -64
	.cfi_offset 30, -56
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	x0, [x29,40]
	str	x1, [x29,32]
	str	w2, [x29,28]
	str	w3, [x29,24]
	str	w4, [x29,20]
	adrp	x0, addInt
	add	x0, x0, :lo12:addInt
	str	x0, [x29,56]
	ldr	x3, [x29,56]
	mov	w0, 2
	mov	w1, 3
	mov	w2, 4
	blr	x3
	str	w0, [x29,52]
	ldr	w0, [x29,52]
	ldp	x29, x30, [sp], 64
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall, .-Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall
	.align	2
	.global	addInt
	.hidden	addInt
	.type	addInt, %function
addInt:
.LFB238:
	.cfi_startproc
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	w0, [sp,12]
	str	w1, [sp,8]
	str	w2, [sp,4]
	ldr	w1, [sp,12]
	ldr	w0, [sp,8]
	add	w1, w1, w0
	ldr	w0, [sp,4]
	add	w0, w1, w0
	add	sp, sp, 16
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE238:
	.size	addInt, .-addInt
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
