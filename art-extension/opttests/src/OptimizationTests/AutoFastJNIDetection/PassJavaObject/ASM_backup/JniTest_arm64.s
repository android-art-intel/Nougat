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
	.section	.text._ZN7_JNIEnv14GetObjectClassEP8_jobject,"axG",%progbits,_ZN7_JNIEnv14GetObjectClassEP8_jobject,comdat
	.align	2
	.weak	_ZN7_JNIEnv14GetObjectClassEP8_jobject
	.hidden	_ZN7_JNIEnv14GetObjectClassEP8_jobject
	.type	_ZN7_JNIEnv14GetObjectClassEP8_jobject, %function
_ZN7_JNIEnv14GetObjectClassEP8_jobject:
.LFB30:
	.cfi_startproc
	stp	x29, x30, [sp, -32]!
	.cfi_def_cfa_offset 32
	.cfi_offset 29, -32
	.cfi_offset 30, -24
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	x0, [x29,24]
	str	x1, [x29,16]
	ldr	x0, [x29,24]
	ldr	x0, [x0]
	ldr	x2, [x0,248]
	ldr	x0, [x29,24]
	ldr	x1, [x29,16]
	blr	x2
	nop
	ldp	x29, x30, [sp], 32
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE30:
	.size	_ZN7_JNIEnv14GetObjectClassEP8_jobject, .-_ZN7_JNIEnv14GetObjectClassEP8_jobject
	.section	.text._ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_,"axG",%progbits,_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_,comdat
	.align	2
	.weak	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	.hidden	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	.type	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_, %function
_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_:
.LFB93:
	.cfi_startproc
	stp	x29, x30, [sp, -48]!
	.cfi_def_cfa_offset 48
	.cfi_offset 29, -48
	.cfi_offset 30, -40
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	x0, [x29,40]
	str	x1, [x29,32]
	str	x2, [x29,24]
	str	x3, [x29,16]
	ldr	x0, [x29,40]
	ldr	x0, [x0]
	ldr	x4, [x0,752]
	ldr	x0, [x29,40]
	ldr	x1, [x29,32]
	ldr	x2, [x29,24]
	ldr	x3, [x29,16]
	blr	x4
	nop
	ldp	x29, x30, [sp], 48
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE93:
	.size	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_, .-_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	.section	.text._ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID,"axG",%progbits,_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID,comdat
	.align	2
	.weak	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	.type	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID, %function
_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID:
.LFB99:
	.cfi_startproc
	stp	x29, x30, [sp, -48]!
	.cfi_def_cfa_offset 48
	.cfi_offset 29, -48
	.cfi_offset 30, -40
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	x0, [x29,40]
	str	x1, [x29,32]
	str	x2, [x29,24]
	ldr	x0, [x29,40]
	ldr	x0, [x0]
	ldr	x3, [x0,800]
	ldr	x0, [x29,40]
	ldr	x1, [x29,32]
	ldr	x2, [x29,24]
	blr	x3
	nop
	ldp	x29, x30, [sp], 48
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE99:
	.size	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID, .-_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	.section	.text._ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID,"axG",%progbits,_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID,comdat
	.align	2
	.weak	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	.type	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID, %function
_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID:
.LFB101:
	.cfi_startproc
	stp	x29, x30, [sp, -48]!
	.cfi_def_cfa_offset 48
	.cfi_offset 29, -48
	.cfi_offset 30, -40
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	x0, [x29,40]
	str	x1, [x29,32]
	str	x2, [x29,24]
	ldr	x0, [x29,40]
	ldr	x0, [x0]
	ldr	x3, [x0,816]
	ldr	x0, [x29,40]
	ldr	x1, [x29,32]
	ldr	x2, [x29,24]
	blr	x3
	fmov	w0, s0
	nop
	fmov	s0, w0
	ldp	x29, x30, [sp], 48
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE101:
	.size	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID, .-_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	.section	.text._ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID,"axG",%progbits,_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID,comdat
	.align	2
	.weak	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID
	.type	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID, %function
_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID:
.LFB102:
	.cfi_startproc
	stp	x29, x30, [sp, -48]!
	.cfi_def_cfa_offset 48
	.cfi_offset 29, -48
	.cfi_offset 30, -40
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	x0, [x29,40]
	str	x1, [x29,32]
	str	x2, [x29,24]
	ldr	x0, [x29,40]
	ldr	x0, [x0]
	ldr	x3, [x0,824]
	ldr	x0, [x29,40]
	ldr	x1, [x29,32]
	ldr	x2, [x29,24]
	blr	x3
	fmov	x0, d0
	nop
	fmov	d0, x0
	ldp	x29, x30, [sp], 48
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE102:
	.size	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID, .-_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID
	.section	.rodata
	.align	3
.LC0:
	.string	"iValue"
	.align	3
.LC1:
	.string	"I"
	.align	3
.LC2:
	.string	"fValue"
	.align	3
.LC3:
	.string	"F"
	.align	3
.LC4:
	.string	"dValue"
	.align	3
.LC5:
	.string	"D"
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj
	.type	Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj, %function
Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj:
.LFB237:
	.cfi_startproc
	stp	x29, x30, [sp, -112]!
	.cfi_def_cfa_offset 112
	.cfi_offset 29, -112
	.cfi_offset 30, -104
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	x0, [x29,40]
	str	x1, [x29,32]
	str	x2, [x29,24]
	ldr	x0, [x29,40]
	ldr	x1, [x29,24]
	bl	_ZN7_JNIEnv14GetObjectClassEP8_jobject
	str	x0, [x29,104]
	adrp	x0, .LC0
	add	x2, x0, :lo12:.LC0
	adrp	x0, .LC1
	add	x3, x0, :lo12:.LC1
	ldr	x0, [x29,40]
	ldr	x1, [x29,104]
	bl	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	str	x0, [x29,96]
	ldr	x0, [x29,40]
	ldr	x1, [x29,24]
	ldr	x2, [x29,96]
	bl	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	str	w0, [x29,92]
	adrp	x0, .LC2
	add	x2, x0, :lo12:.LC2
	adrp	x0, .LC3
	add	x3, x0, :lo12:.LC3
	ldr	x0, [x29,40]
	ldr	x1, [x29,104]
	bl	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	str	x0, [x29,80]
	ldr	x0, [x29,40]
	ldr	x1, [x29,24]
	ldr	x2, [x29,80]
	bl	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	fmov	w0, s0
	str	w0, [x29,76]
	adrp	x0, .LC4
	add	x2, x0, :lo12:.LC4
	adrp	x0, .LC5
	add	x3, x0, :lo12:.LC5
	ldr	x0, [x29,40]
	ldr	x1, [x29,104]
	bl	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	str	x0, [x29,64]
	ldr	x0, [x29,40]
	ldr	x1, [x29,24]
	ldr	x2, [x29,64]
	bl	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID
	fmov	x0, d0
	str	x0, [x29,56]
	ldr	w0, [x29,92]
	scvtf	s0, w0
	fmov	w1, s0
	ldr	w0, [x29,76]
	fmov	s0, w1
	fmov	s1, w0
	fadd	s0, s0, s1
	fmov	w0, s0
	fmov	s0, w0
	fcvt	d0, s0
	fmov	x1, d0
	ldr	x0, [x29,56]
	fmov	d0, x1
	fmov	d1, x0
	fadd	d0, d0, d1
	fmov	x0, d0
	fmov	d0, x0
	ldp	x29, x30, [sp], 112
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj, .-Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
