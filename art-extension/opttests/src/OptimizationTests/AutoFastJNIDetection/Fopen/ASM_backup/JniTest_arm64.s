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
	.section	.rodata
	.align	3
.LC0:
	.string	"file.txt"
	.align	3
.LC1:
	.string	"w+"
	.align	3
.LC2:
	.string	"%s %s %s %d"
	.align	3
.LC3:
	.string	"We"
	.align	3
.LC4:
	.string	"are"
	.align	3
.LC5:
	.string	"in"
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen
	.type	Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen, %function
Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen:
.LFB242:
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
	adrp	x0, .LC0
	add	x2, x0, :lo12:.LC0
	adrp	x0, .LC1
	add	x1, x0, :lo12:.LC1
	mov	x0, x2
	bl	fopen
	str	x0, [x29,56]
	adrp	x0, .LC2
	add	x1, x0, :lo12:.LC2
	adrp	x0, .LC3
	add	x2, x0, :lo12:.LC3
	adrp	x0, .LC4
	add	x3, x0, :lo12:.LC4
	adrp	x0, .LC5
	add	x4, x0, :lo12:.LC5
	ldr	x0, [x29,56]
	mov	w5, 2016
	bl	fprintf
	ldr	x0, [x29,56]
	bl	fclose
	ldr	w0, [x29,28]
	ldp	x29, x30, [sp], 64
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE242:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen, .-Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
