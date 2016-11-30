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
	.global	Java_OptimizationTests_AutoFastJNIDetection_Malloc_Main_nativeMalloc
	.type	Java_OptimizationTests_AutoFastJNIDetection_Malloc_Main_nativeMalloc, %function
Java_OptimizationTests_AutoFastJNIDetection_Malloc_Main_nativeMalloc:
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
	mov	x0, 4
	bl	malloc
	str	x0, [x29,56]
	ldr	x0, [x29,56]
	cmp	x0, xzr
	bne	.L2
	mov	w0, -1
	b	.L3
.L2:
	ldr	x0, [x29,56]
	ldr	w1, [x29,28]
	str	w1, [x0]
	ldr	x0, [x29,56]
	ldr	w0, [x0]
	str	w0, [x29,52]
	ldr	x0, [x29,56]
	bl	free
	ldr	w0, [x29,52]
.L3:
	ldp	x29, x30, [sp], 64
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE242:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Malloc_Main_nativeMalloc, .-Java_OptimizationTests_AutoFastJNIDetection_Malloc_Main_nativeMalloc
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
