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
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch
	.type	Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch, %function
Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch:
	.fnstart
.LFB234:
	@ args = 4, pretend = 0, frame = 24
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {r4, fp, lr}
	.save {r4, fp, lr}
	.setfp fp, sp, #8
	add	fp, sp, #8
	.pad #28
	sub	sp, sp, #28
	str	r0, [fp, #-24]
	str	r1, [fp, #-28]
	str	r2, [fp, #-32]
	str	r3, [fp, #-36]
	ldr	r4, .L7
.LPIC0:
	add	r4, pc, r4
	mvn	r3, #110
	str	r3, [fp, #-16]
	mov	r0, #4
	bl	__cxa_allocate_exception(PLT)
	mov	r3, r0
	ldr	r1, [fp, #-32]
	ldr	r2, [fp, #-36]
	add	r1, r1, r2
	ldr	r2, [fp, #4]
	add	r2, r1, r2
	str	r2, [r3]
	mov	r0, r3
	ldr	r3, .L7+4
	ldr	r3, [r4, r3]
	mov	r1, r3
	mov	r2, #0
.LEHB0:
	bl	__cxa_throw(PLT)
.LEHE0:
.L6:
	mov	r3, r0
	mov	r2, r1
	cmp	r2, #1
	beq	.L5
.LEHB1:
	bl	__cxa_end_cleanup(PLT)
.LEHE1:
.L5:
	mov	r0, r3
	bl	__cxa_begin_catch(PLT)
	mov	r3, r0
	ldr	r3, [r3]
	str	r3, [fp, #-20]
	ldr	r2, [fp, #-20]
	ldr	r3, [fp, #-16]
	add	r4, r2, r3
	bl	__cxa_end_catch(PLT)
	mov	r3, r4
	mov	r0, r3
	sub	sp, fp, #8
	@ sp needed
	ldmfd	sp!, {r4, fp, pc}
.L8:
	.align	2
.L7:
	.word	_GLOBAL_OFFSET_TABLE_-(.LPIC0+8)
	.word	_ZTIi(GOT)
	.global	__gxx_personality_v0
	.personality	__gxx_personality_v0
	.handlerdata
	.align	2
.LLSDA234:
	.byte	0xff
	.byte	0
	.uleb128 .LLSDATT234-.LLSDATTD234
.LLSDATTD234:
	.byte	0x1
	.uleb128 .LLSDACSE234-.LLSDACSB234
.LLSDACSB234:
	.uleb128 .LEHB0-.LFB234
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L6-.LFB234
	.uleb128 0x1
	.uleb128 .LEHB1-.LFB234
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0
	.uleb128 0
.LLSDACSE234:
	.byte	0x1
	.byte	0
	.align	2
	.word	_ZTIi(TARGET2)
.LLSDATT234:
	.text
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch, .-Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
