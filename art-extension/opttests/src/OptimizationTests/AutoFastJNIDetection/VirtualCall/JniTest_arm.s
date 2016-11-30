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
	.section	.text._ZN1A8getValueEv,"axG",%progbits,_ZN1A8getValueEv,comdat
	.align	2
	.weak	_ZN1A8getValueEv
	.hidden	_ZN1A8getValueEv
	.type	_ZN1A8getValueEv, %function
_ZN1A8getValueEv:
	.fnstart
.LFB234:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	str	r0, [fp, #-8]
	mov	r3, #1
	mov	r0, r3
	sub	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.cantunwind
	.fnend
	.size	_ZN1A8getValueEv, .-_ZN1A8getValueEv
	.section	.text._ZN1B8getValueEv,"axG",%progbits,_ZN1B8getValueEv,comdat
	.align	2
	.weak	_ZN1B8getValueEv
	.hidden	_ZN1B8getValueEv
	.type	_ZN1B8getValueEv, %function
_ZN1B8getValueEv:
	.fnstart
.LFB235:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	str	r0, [fp, #-8]
	mov	r3, #100
	mov	r0, r3
	sub	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.cantunwind
	.fnend
	.size	_ZN1B8getValueEv, .-_ZN1B8getValueEv
	.section	.text._ZN1AC2Ev,"axG",%progbits,_ZN1AC5Ev,comdat
	.align	2
	.weak	_ZN1AC2Ev
	.hidden	_ZN1AC2Ev
	.type	_ZN1AC2Ev, %function
_ZN1AC2Ev:
	.fnstart
.LFB239:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	str	r0, [fp, #-8]
	ldr	r3, [fp, #-8]
	ldr	r2, .L8
.LPIC0:
	add	r2, pc, r2
	add	r2, r2, #8
	str	r2, [r3]
	ldr	r3, [fp, #-8]
	mov	r0, r3
	sub	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
.L9:
	.align	2
.L8:
	.word	_ZTV1A-(.LPIC0+8)
	.cantunwind
	.fnend
	.size	_ZN1AC2Ev, .-_ZN1AC2Ev
	.weak	_ZN1AC1Ev
	.hidden	_ZN1AC1Ev
_ZN1AC1Ev = _ZN1AC2Ev
	.section	.text._ZN1BC2Ev,"axG",%progbits,_ZN1BC5Ev,comdat
	.align	2
	.weak	_ZN1BC2Ev
	.hidden	_ZN1BC2Ev
	.type	_ZN1BC2Ev, %function
_ZN1BC2Ev:
	.fnstart
.LFB241:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #8
	str	r0, [fp, #-8]
	ldr	r3, [fp, #-8]
	mov	r0, r3
	bl	_ZN1AC2Ev(PLT)
	ldr	r3, [fp, #-8]
	ldr	r2, .L13
.LPIC1:
	add	r2, pc, r2
	add	r2, r2, #8
	str	r2, [r3]
	ldr	r3, [fp, #-8]
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
.L14:
	.align	2
.L13:
	.word	_ZTV1B-(.LPIC1+8)
	.cantunwind
	.fnend
	.size	_ZN1BC2Ev, .-_ZN1BC2Ev
	.weak	_ZN1BC1Ev
	.hidden	_ZN1BC1Ev
_ZN1BC1Ev = _ZN1BC2Ev
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall
	.type	Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall, %function
Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall:
	.fnstart
.LFB236:
	@ args = 4, pretend = 0, frame = 24
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	.save {fp, lr}
	.setfp fp, sp, #4
	add	fp, sp, #4
	.pad #24
	sub	sp, sp, #24
	str	r0, [fp, #-16]
	str	r1, [fp, #-20]
	str	r2, [fp, #-24]
	str	r3, [fp, #-28]
	sub	r3, fp, #12
	mov	r0, r3
	bl	_ZN1BC1Ev(PLT)
	sub	r3, fp, #12
	str	r3, [fp, #-8]
	ldr	r3, [fp, #-8]
	ldr	r3, [r3]
	ldr	r3, [r3]
	ldr	r0, [fp, #-8]
.LEHB0:
	blx	r3
.LEHE0:
	mov	r3, r0
	b	.L19
.L18:
.LEHB1:
	bl	__cxa_end_cleanup(PLT)
.LEHE1:
.L19:
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
	.global	__gxx_personality_v0
	.personality	__gxx_personality_v0
	.handlerdata
.LLSDA236:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE236-.LLSDACSB236
.LLSDACSB236:
	.uleb128 .LEHB0-.LFB236
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L18-.LFB236
	.uleb128 0
	.uleb128 .LEHB1-.LFB236
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0
	.uleb128 0
.LLSDACSE236:
	.text
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall, .-Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall
	.hidden	_ZTV1B
	.weak	_ZTV1B
	.section	.data.rel.ro._ZTV1B,"awG",%progbits,_ZTV1B,comdat
	.align	3
	.type	_ZTV1B, %object
	.size	_ZTV1B, 12
_ZTV1B:
	.word	0
	.word	_ZTI1B
	.word	_ZN1B8getValueEv
	.hidden	_ZTV1A
	.weak	_ZTV1A
	.section	.data.rel.ro._ZTV1A,"awG",%progbits,_ZTV1A,comdat
	.align	3
	.type	_ZTV1A, %object
	.size	_ZTV1A, 12
_ZTV1A:
	.word	0
	.word	_ZTI1A
	.word	_ZN1A8getValueEv
	.hidden	_ZTS1B
	.weak	_ZTS1B
	.section	.rodata._ZTS1B,"aG",%progbits,_ZTS1B,comdat
	.align	2
	.type	_ZTS1B, %object
	.size	_ZTS1B, 3
_ZTS1B:
	.ascii	"1B\000"
	.hidden	_ZTI1B
	.weak	_ZTI1B
	.section	.data.rel.ro._ZTI1B,"awG",%progbits,_ZTI1B,comdat
	.align	2
	.type	_ZTI1B, %object
	.size	_ZTI1B, 12
_ZTI1B:
	.word	_ZTVN10__cxxabiv120__si_class_type_infoE+8
	.word	_ZTS1B
	.word	_ZTI1A
	.hidden	_ZTS1A
	.weak	_ZTS1A
	.section	.rodata._ZTS1A,"aG",%progbits,_ZTS1A,comdat
	.align	2
	.type	_ZTS1A, %object
	.size	_ZTS1A, 3
_ZTS1A:
	.ascii	"1A\000"
	.hidden	_ZTI1A
	.weak	_ZTI1A
	.section	.data.rel.ro._ZTI1A,"awG",%progbits,_ZTI1A,comdat
	.align	2
	.type	_ZTI1A, %object
	.size	_ZTI1A, 8
_ZTI1A:
	.word	_ZTVN10__cxxabiv117__class_type_infoE+8
	.word	_ZTS1A
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
