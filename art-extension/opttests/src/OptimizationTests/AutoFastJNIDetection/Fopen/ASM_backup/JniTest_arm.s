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

	.arch armv5te
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 6
	.eabi_attribute 34, 0
	.eabi_attribute 18, 4
	.file	"JniTest.cpp"
	.section	.rodata
	.align	2
.LC0:
	.ascii	"file.txt\000"
	.align	2
.LC1:
	.ascii	"w+\000"
	.align	2
.LC2:
	.ascii	"%s %s %s %d\000"
	.align	2
.LC3:
	.ascii	"We\000"
	.align	2
.LC4:
	.ascii	"are\000"
	.align	2
.LC5:
	.ascii	"in\000"
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen
	.type	Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen, %function
Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen:
	.fnstart
.LFB248:
	@ args = 0, pretend = 0, frame = 24
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	.save {fp, lr}
	.setfp fp, sp, #4
	add	fp, sp, #4
	.pad #32
	sub	sp, sp, #32
	str	r0, [fp, #-16]
	str	r1, [fp, #-20]
	str	r2, [fp, #-24]
	ldr	r3, .L3
.LPIC0:
	add	r3, pc, r3
	mov	r0, r3
	ldr	r3, .L3+4
.LPIC1:
	add	r3, pc, r3
	mov	r1, r3
	bl	fopen(PLT)
	mov	r3, r0
	str	r3, [fp, #-8]
	ldr	r3, .L3+8
.LPIC2:
	add	r3, pc, r3
	str	r3, [sp]
	mov	r3, #2016
	str	r3, [sp, #4]
	ldr	r0, [fp, #-8]
	ldr	r3, .L3+12
.LPIC3:
	add	r3, pc, r3
	mov	r1, r3
	ldr	r3, .L3+16
.LPIC4:
	add	r3, pc, r3
	mov	r2, r3
	ldr	r3, .L3+20
.LPIC5:
	add	r3, pc, r3
	bl	fprintf(PLT)
	ldr	r0, [fp, #-8]
	bl	fclose(PLT)
	ldr	r3, [fp, #-24]
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
.L4:
	.align	2
.L3:
	.word	.LC0-(.LPIC0+8)
	.word	.LC1-(.LPIC1+8)
	.word	.LC5-(.LPIC2+8)
	.word	.LC2-(.LPIC3+8)
	.word	.LC3-(.LPIC4+8)
	.word	.LC4-(.LPIC5+8)
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen, .-Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
