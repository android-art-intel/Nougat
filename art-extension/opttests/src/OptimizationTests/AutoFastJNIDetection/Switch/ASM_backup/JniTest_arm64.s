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
	.global	Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch
	.type	Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch, %function
Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch:
.LFB237:
	.cfi_startproc
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	str	x0, [sp,24]
	str	x1, [sp,16]
	str	w2, [sp,12]
	str	w3, [sp,8]
	str	w4, [sp,4]
	ldr	w0, [sp,12]
	add	w0, w0, 1
	cmp	w0, 5
	bhi	.L2
	adrp	x1, .L4
	add	x1, x1, :lo12:.L4
	ldr	w0, [x1,w0,uxtw #2]
	adr	x1, .Lrtx4
	add	x0, x1, w0, sxtw #2
	br	x0
.Lrtx4:
	.section	.rodata
	.align	0
	.align	2
.L4:
	.word	(.L3 - .Lrtx4) / 4
	.word	(.L5 - .Lrtx4) / 4
	.word	(.L6 - .Lrtx4) / 4
	.word	(.L7 - .Lrtx4) / 4
	.word	(.L8 - .Lrtx4) / 4
	.word	(.L9 - .Lrtx4) / 4
	.text
.L3:
	mov	w0, 10
	str	w0, [sp,8]
	b	.L10
.L5:
	ldr	w0, [sp,8]
	add	w0, w0, 1
	str	w0, [sp,8]
	b	.L10
.L6:
	ldr	w1, [sp,8]
	ldr	w0, [sp,4]
	add	w0, w1, w0
	str	w0, [sp,8]
	b	.L10
.L7:
	ldr	w1, [sp,8]
	ldr	w0, [sp,4]
	sub	w0, w1, w0
	str	w0, [sp,8]
	ldr	w0, [sp,4]
	add	w0, w0, 5
	str	w0, [sp,4]
	b	.L10
.L8:
	ldr	w1, [sp,8]
	ldr	w0, [sp,4]
	sub	w0, w1, w0
	str	w0, [sp,8]
	ldr	w0, [sp,4]
	add	w0, w0, 6
	str	w0, [sp,4]
	b	.L10
.L9:
	ldr	w1, [sp,8]
	ldr	w0, [sp,4]
	sub	w0, w1, w0
	str	w0, [sp,8]
	ldr	w0, [sp,4]
	add	w0, w0, 6
	str	w0, [sp,4]
	b	.L10
.L2:
	ldr	w1, [sp,8]
	ldr	w0, [sp,4]
	sub	w0, w1, w0
	str	w0, [sp,8]
	nop
.L10:
	ldr	w0, [sp,8]
	add	sp, sp, 32
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch, .-Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
