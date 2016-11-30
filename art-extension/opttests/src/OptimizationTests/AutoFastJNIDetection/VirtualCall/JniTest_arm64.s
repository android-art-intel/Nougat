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
	.section	.text._ZN1A8getValueEv,"axG",%progbits,_ZN1A8getValueEv,comdat
	.align	2
	.weak	_ZN1A8getValueEv
	.hidden	_ZN1A8getValueEv
	.type	_ZN1A8getValueEv, %function
_ZN1A8getValueEv:
.LFB237:
	.cfi_startproc
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	x0, [sp,8]
	mov	w0, 1
	add	sp, sp, 16
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE237:
	.size	_ZN1A8getValueEv, .-_ZN1A8getValueEv
	.section	.text._ZN1B8getValueEv,"axG",%progbits,_ZN1B8getValueEv,comdat
	.align	2
	.weak	_ZN1B8getValueEv
	.hidden	_ZN1B8getValueEv
	.type	_ZN1B8getValueEv, %function
_ZN1B8getValueEv:
.LFB238:
	.cfi_startproc
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	x0, [sp,8]
	mov	w0, 100
	add	sp, sp, 16
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE238:
	.size	_ZN1B8getValueEv, .-_ZN1B8getValueEv
	.section	.text._ZN1AC2Ev,"axG",%progbits,_ZN1AC5Ev,comdat
	.align	2
	.weak	_ZN1AC2Ev
	.hidden	_ZN1AC2Ev
	.type	_ZN1AC2Ev, %function
_ZN1AC2Ev:
.LFB242:
	.cfi_startproc
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	x0, [sp,8]
	ldr	x0, [sp,8]
	adrp	x1, _ZTV1A+16
	add	x1, x1, :lo12:_ZTV1A+16
	str	x1, [x0]
	add	sp, sp, 16
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE242:
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
.LFB244:
	.cfi_startproc
	stp	x29, x30, [sp, -32]!
	.cfi_def_cfa_offset 32
	.cfi_offset 29, -32
	.cfi_offset 30, -24
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	x0, [x29,24]
	ldr	x0, [x29,24]
	bl	_ZN1AC2Ev
	ldr	x0, [x29,24]
	adrp	x1, _ZTV1B+16
	add	x1, x1, :lo12:_ZTV1B+16
	str	x1, [x0]
	ldp	x29, x30, [sp], 32
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE244:
	.size	_ZN1BC2Ev, .-_ZN1BC2Ev
	.weak	_ZN1BC1Ev
	.hidden	_ZN1BC1Ev
_ZN1BC1Ev = _ZN1BC2Ev
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall
	.type	Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall, %function
Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall:
.LFB239:
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
	add	x0, x29, 48
	bl	_ZN1BC1Ev
	add	x0, x29, 48
	str	x0, [x29,56]
	ldr	x0, [x29,56]
	ldr	x0, [x0]
	ldr	x1, [x0]
	ldr	x0, [x29,56]
	blr	x1
	nop
	ldp	x29, x30, [sp], 64
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE239:
	.size	Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall, .-Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall
	.hidden	_ZTV1B
	.weak	_ZTV1B
	.section	.data.rel.ro.local._ZTV1B,"awG",%progbits,_ZTV1B,comdat
	.align	3
	.type	_ZTV1B, %object
	.size	_ZTV1B, 24
_ZTV1B:
	.xword	0
	.xword	_ZTI1B
	.xword	_ZN1B8getValueEv
	.hidden	_ZTV1A
	.weak	_ZTV1A
	.section	.data.rel.ro.local._ZTV1A,"awG",%progbits,_ZTV1A,comdat
	.align	3
	.type	_ZTV1A, %object
	.size	_ZTV1A, 24
_ZTV1A:
	.xword	0
	.xword	_ZTI1A
	.xword	_ZN1A8getValueEv
	.hidden	_ZTS1B
	.weak	_ZTS1B
	.section	.rodata._ZTS1B,"aG",%progbits,_ZTS1B,comdat
	.align	3
	.type	_ZTS1B, %object
	.size	_ZTS1B, 3
_ZTS1B:
	.string	"1B"
	.hidden	_ZTI1B
	.weak	_ZTI1B
	.section	.data.rel.ro._ZTI1B,"awG",%progbits,_ZTI1B,comdat
	.align	3
	.type	_ZTI1B, %object
	.size	_ZTI1B, 24
_ZTI1B:
	.xword	_ZTVN10__cxxabiv120__si_class_type_infoE+16
	.xword	_ZTS1B
	.xword	_ZTI1A
	.hidden	_ZTS1A
	.weak	_ZTS1A
	.section	.rodata._ZTS1A,"aG",%progbits,_ZTS1A,comdat
	.align	3
	.type	_ZTS1A, %object
	.size	_ZTS1A, 3
_ZTS1A:
	.string	"1A"
	.hidden	_ZTI1A
	.weak	_ZTI1A
	.section	.data.rel.ro._ZTI1A,"awG",%progbits,_ZTI1A,comdat
	.align	3
	.type	_ZTI1A, %object
	.size	_ZTI1A, 16
_ZTI1A:
	.xword	_ZTVN10__cxxabiv117__class_type_infoE+16
	.xword	_ZTS1A
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
