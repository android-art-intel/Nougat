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
	.global	Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset
	.type	Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset, %function
Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset:
.LFB237:
	.cfi_startproc
	stp	x29, x30, [sp, -448]!
	.cfi_def_cfa_offset 448
	.cfi_offset 29, -448
	.cfi_offset 30, -440
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	add	x3, x29, 40
	str	x0, [x3]
	add	x0, x29, 32
	str	x1, [x0]
	add	x0, x29, 28
	str	w2, [x0]
	add	x0, x29, 48
	add	x1, x29, 28
	ldr	w1, [x1]
	mov	x2, 400
	bl	memset
	add	x0, x29, 48
	ldr	w0, [x0,20]
	ldp	x29, x30, [sp], 448
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset, .-Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
