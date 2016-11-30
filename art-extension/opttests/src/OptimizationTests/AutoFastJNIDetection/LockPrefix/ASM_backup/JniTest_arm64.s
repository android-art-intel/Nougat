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
	.hidden	count
	.global	count
	.data
	.align	2
	.type	count, %object
	.size	count, 4
count:
	.word	5
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_LockPrefix_Main_nativeLockPrefix
	.type	Java_OptimizationTests_AutoFastJNIDetection_LockPrefix_Main_nativeLockPrefix, %function
Java_OptimizationTests_AutoFastJNIDetection_LockPrefix_Main_nativeLockPrefix:
.LFB237:
	.cfi_startproc
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	str	x0, [sp,24]
	str	x1, [sp,16]
	str	x2, [sp,8]
	adrp	x0, count
	add	x0, x0, :lo12:count
	ldr	w0, [x0]
	sxtw	x1, w0
	ldr	x0, [sp,8]
	add	x1, x1, x0
	adrp	x0, count
	add	x0, x0, :lo12:count
.L3:
	ldaxr	w2, [x0]
	add	w3, w2, 1
	stlxr	w4, w3, [x0]
	cbnz	w4, .L3
	mov	w0, w2
	sxtw	x0, w0
	add	x0, x1, x0
	add	sp, sp, 32
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_LockPrefix_Main_nativeLockPrefix, .-Java_OptimizationTests_AutoFastJNIDetection_LockPrefix_Main_nativeLockPrefix
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
