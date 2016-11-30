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

	.file	"JniTest.cpp"
	.section	.text._ZN7JNIEnv_14GetObjectClassEP8_jobject,"axG",@progbits,_ZN7JNIEnv_14GetObjectClassEP8_jobject,comdat
	.align 2
	.weak	_ZN7JNIEnv_14GetObjectClassEP8_jobject
	.hidden	_ZN7JNIEnv_14GetObjectClassEP8_jobject
	.type	_ZN7JNIEnv_14GetObjectClassEP8_jobject, @function
_ZN7JNIEnv_14GetObjectClassEP8_jobject:
.LFB30:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	248(%rax), %rcx
	movq	-16(%rbp), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	*%rcx
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE30:
	.size	_ZN7JNIEnv_14GetObjectClassEP8_jobject, .-_ZN7JNIEnv_14GetObjectClassEP8_jobject
	.section	.text._ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_,"axG",@progbits,_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_,comdat
	.align 2
	.weak	_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_
	.hidden	_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_
	.type	_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_, @function
_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_:
.LFB93:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movq	%rcx, -32(%rbp)
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	752(%rax), %r8
	movq	-32(%rbp), %rcx
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rsi
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	*%r8
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE93:
	.size	_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_, .-_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_
	.section	.text._ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID,"axG",@progbits,_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID,comdat
	.align 2
	.weak	_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID
	.type	_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID, @function
_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID:
.LFB99:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	800(%rax), %r8
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rcx
	movq	-8(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	*%r8
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE99:
	.size	_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID, .-_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID
	.section	.text._ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID,"axG",@progbits,_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID,comdat
	.align 2
	.weak	_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID
	.type	_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID, @function
_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID:
.LFB101:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	816(%rax), %r8
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rcx
	movq	-8(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	*%r8
	movss	%xmm0, -28(%rbp)
	movl	-28(%rbp), %eax
	movl	%eax, -28(%rbp)
	movss	-28(%rbp), %xmm0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE101:
	.size	_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID, .-_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID
	.section	.text._ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID,"axG",@progbits,_ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID,comdat
	.align 2
	.weak	_ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID
	.type	_ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID, @function
_ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID:
.LFB102:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	824(%rax), %r8
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rcx
	movq	-8(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	*%r8
	movsd	%xmm0, -32(%rbp)
	movq	-32(%rbp), %rax
	movq	%rax, -32(%rbp)
	movsd	-32(%rbp), %xmm0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE102:
	.size	_ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID, .-_ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID
	.section	.rodata
.LC0:
	.string	"I"
.LC1:
	.string	"iValue"
.LC2:
	.string	"F"
.LC3:
	.string	"fValue"
.LC4:
	.string	"D"
.LC5:
	.string	"dValue"
	.text
	.globl	Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj
	.type	Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj, @function
Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj:
.LFB237:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$80, %rsp
	movq	%rdi, -56(%rbp)
	movq	%rsi, -64(%rbp)
	movq	%rdx, -72(%rbp)
	movq	-72(%rbp), %rdx
	movq	-56(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	_ZN7JNIEnv_14GetObjectClassEP8_jobject
	movq	%rax, -48(%rbp)
	movq	-48(%rbp), %rsi
	movq	-56(%rbp), %rax
	leaq	.LC0(%rip), %rcx
	leaq	.LC1(%rip), %rdx
	movq	%rax, %rdi
	call	_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_
	movq	%rax, -40(%rbp)
	movq	-40(%rbp), %rdx
	movq	-72(%rbp), %rcx
	movq	-56(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID
	movl	%eax, -8(%rbp)
	movq	-48(%rbp), %rsi
	movq	-56(%rbp), %rax
	leaq	.LC2(%rip), %rcx
	leaq	.LC3(%rip), %rdx
	movq	%rax, %rdi
	call	_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_
	movq	%rax, -32(%rbp)
	movq	-32(%rbp), %rdx
	movq	-72(%rbp), %rcx
	movq	-56(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID
	movss	%xmm0, -4(%rbp)
	movq	-48(%rbp), %rsi
	movq	-56(%rbp), %rax
	leaq	.LC4(%rip), %rcx
	leaq	.LC5(%rip), %rdx
	movq	%rax, %rdi
	call	_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_
	movq	%rax, -24(%rbp)
	movq	-24(%rbp), %rdx
	movq	-72(%rbp), %rcx
	movq	-56(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	_ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID
	movsd	%xmm0, -16(%rbp)
	cvtsi2ss	-8(%rbp), %xmm0
	addss	-4(%rbp), %xmm0
	unpcklps	%xmm0, %xmm0
	cvtps2pd	%xmm0, %xmm0
	addsd	-16(%rbp), %xmm0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj, .-Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
