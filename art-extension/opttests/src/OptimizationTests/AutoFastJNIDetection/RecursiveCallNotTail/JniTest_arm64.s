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
	.global	_Z3fooii
	.hidden	_Z3fooii
	.type	_Z3fooii, %function
_Z3fooii:
.LFB237:
	.cfi_startproc
	stp	x29, x30, [sp, -32]!
	.cfi_def_cfa_offset 32
	.cfi_offset 29, -32
	.cfi_offset 30, -24
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	w0, [x29,28]
	str	w1, [x29,24]
	ldr	w0, [x29,28]
	cmp	w0, wzr
	ble	.L2
	ldr	w0, [x29,28]
	sub	w2, w0, #1
	ldr	w0, [x29,24]
	add	w1, w0, 5
	mov	w0, w2
	bl	_Z3fooii
	add	w0, w0, 5
	b	.L3
.L2:
	ldr	w0, [x29,24]
.L3:
	ldp	x29, x30, [sp], 32
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE237:
	.size	_Z3fooii, .-_Z3fooii
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallNotTail_Main_nativeRecursiveCallNotTail
	.type	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallNotTail_Main_nativeRecursiveCallNotTail, %function
Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallNotTail_Main_nativeRecursiveCallNotTail:
.LFB238:
	.cfi_startproc
	stp	x29, x30, [sp, -48]!
	.cfi_def_cfa_offset 48
	.cfi_offset 29, -48
	.cfi_offset 30, -40
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	x0, [x29,40]
	str	x1, [x29,32]
	str	w2, [x29,28]
	str	w3, [x29,24]
	str	w4, [x29,20]
	ldr	w1, [x29,28]
	ldr	w0, [x29,24]
	add	w1, w1, w0
	ldr	w0, [x29,20]
	bl	_Z3fooii
	add	w0, w0, 1
	ldp	x29, x30, [sp], 48
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE238:
	.size	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallNotTail_Main_nativeRecursiveCallNotTail, .-Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallNotTail_Main_nativeRecursiveCallNotTail
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
