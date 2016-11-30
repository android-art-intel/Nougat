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
	.section	.text._ZN7_JNIEnv14GetObjectClassEP8_jobject,"axG",%progbits,_ZN7_JNIEnv14GetObjectClassEP8_jobject,comdat
	.align	2
	.weak	_ZN7_JNIEnv14GetObjectClassEP8_jobject
	.hidden	_ZN7_JNIEnv14GetObjectClassEP8_jobject
	.type	_ZN7_JNIEnv14GetObjectClassEP8_jobject, %function
_ZN7_JNIEnv14GetObjectClassEP8_jobject:
	.fnstart
.LFB27:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	.save {fp, lr}
	.setfp fp, sp, #4
	add	fp, sp, #4
	.pad #8
	sub	sp, sp, #8
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	ldr	r3, [fp, #-8]
	ldr	r3, [r3]
	ldr	r3, [r3, #124]
	ldr	r0, [fp, #-8]
	ldr	r1, [fp, #-12]
	blx	r3
	mov	r3, r0
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
	.fnend
	.size	_ZN7_JNIEnv14GetObjectClassEP8_jobject, .-_ZN7_JNIEnv14GetObjectClassEP8_jobject
	.section	.text._ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_,"axG",%progbits,_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_,comdat
	.align	2
	.weak	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	.hidden	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	.type	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_, %function
_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_:
	.fnstart
.LFB90:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	.save {fp, lr}
	.setfp fp, sp, #4
	add	fp, sp, #4
	.pad #16
	sub	sp, sp, #16
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	str	r2, [fp, #-16]
	str	r3, [fp, #-20]
	ldr	r3, [fp, #-8]
	ldr	r3, [r3]
	ldr	ip, [r3, #376]
	ldr	r0, [fp, #-8]
	ldr	r1, [fp, #-12]
	ldr	r2, [fp, #-16]
	ldr	r3, [fp, #-20]
	blx	ip
	mov	r3, r0
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
	.fnend
	.size	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_, .-_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	.section	.text._ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID,"axG",%progbits,_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID,comdat
	.align	2
	.weak	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	.type	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID, %function
_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID:
	.fnstart
.LFB96:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	.save {fp, lr}
	.setfp fp, sp, #4
	add	fp, sp, #4
	.pad #16
	sub	sp, sp, #16
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	str	r2, [fp, #-16]
	ldr	r3, [fp, #-8]
	ldr	r3, [r3]
	ldr	r3, [r3, #400]
	ldr	r0, [fp, #-8]
	ldr	r1, [fp, #-12]
	ldr	r2, [fp, #-16]
	blx	r3
	mov	r3, r0
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
	.fnend
	.size	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID, .-_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	.section	.text._ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID,"axG",%progbits,_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID,comdat
	.align	2
	.weak	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	.type	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID, %function
_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID:
	.fnstart
.LFB98:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	.save {fp, lr}
	.setfp fp, sp, #4
	add	fp, sp, #4
	.pad #16
	sub	sp, sp, #16
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	str	r2, [fp, #-16]
	ldr	r3, [fp, #-8]
	ldr	r3, [r3]
	ldr	r3, [r3, #408]
	ldr	r0, [fp, #-8]
	ldr	r1, [fp, #-12]
	ldr	r2, [fp, #-16]
	blx	r3
	mov	r3, r0
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
	.fnend
	.size	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID, .-_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	.section	.text._ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID,"axG",%progbits,_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID,comdat
	.align	2
	.weak	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID
	.type	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID, %function
_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID:
	.fnstart
.LFB99:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	.save {fp, lr}
	.setfp fp, sp, #4
	add	fp, sp, #4
	.pad #16
	sub	sp, sp, #16
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	str	r2, [fp, #-16]
	ldr	r3, [fp, #-8]
	ldr	r3, [r3]
	ldr	r3, [r3, #412]
	ldr	r0, [fp, #-8]
	ldr	r1, [fp, #-12]
	ldr	r2, [fp, #-16]
	blx	r3
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
	.fnend
	.size	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID, .-_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID
	.section	.rodata
	.align	2
.LC0:
	.ascii	"iValue\000"
	.align	2
.LC1:
	.ascii	"I\000"
	.align	2
.LC2:
	.ascii	"fValue\000"
	.align	2
.LC3:
	.ascii	"F\000"
	.align	2
.LC4:
	.ascii	"dValue\000"
	.align	2
.LC5:
	.ascii	"D\000"
	.global	__aeabi_i2f
	.global	__aeabi_fadd
	.global	__aeabi_f2d
	.global	__aeabi_dadd
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj
	.type	Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj, %function
Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj:
	.fnstart
.LFB234:
	@ args = 0, pretend = 0, frame = 48
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	.save {fp, lr}
	.setfp fp, sp, #4
	add	fp, sp, #4
	.pad #48
	sub	sp, sp, #48
	str	r0, [fp, #-40]
	str	r1, [fp, #-44]
	str	r2, [fp, #-48]
	ldr	r0, [fp, #-40]
	ldr	r1, [fp, #-48]
	bl	_ZN7_JNIEnv14GetObjectClassEP8_jobject(PLT)
	mov	r3, r0
	str	r3, [fp, #-8]
	ldr	r0, [fp, #-40]
	ldr	r1, [fp, #-8]
	ldr	r3, .L13
.LPIC0:
	add	r3, pc, r3
	mov	r2, r3
	ldr	r3, .L13+4
.LPIC1:
	add	r3, pc, r3
	bl	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_(PLT)
	mov	r3, r0
	str	r3, [fp, #-12]
	ldr	r0, [fp, #-40]
	ldr	r1, [fp, #-48]
	ldr	r2, [fp, #-12]
	bl	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID(PLT)
	mov	r3, r0
	str	r3, [fp, #-16]
	ldr	r0, [fp, #-40]
	ldr	r1, [fp, #-8]
	ldr	r3, .L13+8
.LPIC2:
	add	r3, pc, r3
	mov	r2, r3
	ldr	r3, .L13+12
.LPIC3:
	add	r3, pc, r3
	bl	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_(PLT)
	mov	r3, r0
	str	r3, [fp, #-20]
	ldr	r0, [fp, #-40]
	ldr	r1, [fp, #-48]
	ldr	r2, [fp, #-20]
	bl	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID(PLT)
	mov	r3, r0
	str	r3, [fp, #-24]	@ float
	ldr	r0, [fp, #-40]
	ldr	r1, [fp, #-8]
	ldr	r3, .L13+16
.LPIC4:
	add	r3, pc, r3
	mov	r2, r3
	ldr	r3, .L13+20
.LPIC5:
	add	r3, pc, r3
	bl	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_(PLT)
	mov	r3, r0
	str	r3, [fp, #-28]
	ldr	r0, [fp, #-40]
	ldr	r1, [fp, #-48]
	ldr	r2, [fp, #-28]
	bl	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID(PLT)
	mov	r2, r0
	mov	r3, r1
	strd	r2, [fp, #-36]
	ldr	r0, [fp, #-16]
	bl	__aeabi_i2f(PLT)
	mov	r3, r0
	mov	r0, r3
	ldr	r1, [fp, #-24]	@ float
	bl	__aeabi_fadd(PLT)
	mov	r3, r0
	mov	r0, r3
	bl	__aeabi_f2d(PLT)
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	ldrd	r2, [fp, #-36]
	bl	__aeabi_dadd(PLT)
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
.L14:
	.align	2
.L13:
	.word	.LC0-(.LPIC0+8)
	.word	.LC1-(.LPIC1+8)
	.word	.LC2-(.LPIC2+8)
	.word	.LC3-(.LPIC3+8)
	.word	.LC4-(.LPIC4+8)
	.word	.LC5-(.LPIC5+8)
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj, .-Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
