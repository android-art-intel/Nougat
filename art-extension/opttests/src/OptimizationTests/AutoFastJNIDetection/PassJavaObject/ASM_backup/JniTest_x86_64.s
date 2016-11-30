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
	.section	.text._ZN7_JNIEnv14GetObjectClassEP8_jobject,"axG",@progbits,_ZN7_JNIEnv14GetObjectClassEP8_jobject,comdat
	.align 2
	.weak	_ZN7_JNIEnv14GetObjectClassEP8_jobject
	.hidden	_ZN7_JNIEnv14GetObjectClassEP8_jobject
	.type	_ZN7_JNIEnv14GetObjectClassEP8_jobject, @function
_ZN7_JNIEnv14GetObjectClassEP8_jobject:
.LFB30:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	-16(%rsp), %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	248(%rax), %rax
	movq	-16(%rbp), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, %rsi
	movq	%rdx, %rdi
	call	*%rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE30:
	.size	_ZN7_JNIEnv14GetObjectClassEP8_jobject, .-_ZN7_JNIEnv14GetObjectClassEP8_jobject
	.section	.text._ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_,"axG",@progbits,_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_,comdat
	.align 2
	.weak	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	.hidden	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	.type	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_, @function
_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_:
.LFB93:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	-32(%rsp), %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movq	%rcx, -32(%rbp)
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	752(%rax), %rax
	movq	-32(%rbp), %rcx
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rsi
	movq	-8(%rbp), %rdi
	call	*%rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE93:
	.size	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_, .-_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	.section	.text._ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID,"axG",@progbits,_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID,comdat
	.align 2
	.weak	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	.type	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID, @function
_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID:
.LFB99:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	-32(%rsp), %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	800(%rax), %rax
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rsi
	movq	-8(%rbp), %rcx
	movq	%rcx, %rdi
	call	*%rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE99:
	.size	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID, .-_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	.section	.text._ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID,"axG",@progbits,_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID,comdat
	.align 2
	.weak	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	.type	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID, @function
_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID:
.LFB101:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	-32(%rsp), %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	816(%rax), %rax
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rsi
	movq	-8(%rbp), %rcx
	movq	%rcx, %rdi
	call	*%rax
	movd	%xmm0, %eax
	movd	%eax, %xmm0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE101:
	.size	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID, .-_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	.section	.text._ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID,"axG",@progbits,_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID,comdat
	.align 2
	.weak	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID
	.type	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID, @function
_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID:
.LFB102:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	-32(%rsp), %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	824(%rax), %rax
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rsi
	movq	-8(%rbp), %rcx
	movq	%rcx, %rdi
	call	*%rax
	movq	%xmm0, %rax
	movq	%rax, %xmm0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE102:
	.size	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID, .-_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID
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
	leaq	-96(%rsp), %rsp
	movq	%rdi, -72(%rbp)
	movq	%rsi, -80(%rbp)
	movq	%rdx, -88(%rbp)
	movq	-88(%rbp), %rdx
	movq	-72(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	_ZN7_JNIEnv14GetObjectClassEP8_jobject
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rsi
	movq	-72(%rbp), %rax
	leaq	.LC0(%rip), %rcx
	leaq	.LC1(%rip), %rdx
	movq	%rax, %rdi
	call	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rdx
	movq	-88(%rbp), %rcx
	movq	-72(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	movl	%eax, -20(%rbp)
	movq	-8(%rbp), %rsi
	movq	-72(%rbp), %rax
	leaq	.LC2(%rip), %rcx
	leaq	.LC3(%rip), %rdx
	movq	%rax, %rdi
	call	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	movq	%rax, -32(%rbp)
	movq	-32(%rbp), %rdx
	movq	-88(%rbp), %rcx
	movq	-72(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	movd	%xmm0, %eax
	movl	%eax, -36(%rbp)
	movq	-8(%rbp), %rsi
	movq	-72(%rbp), %rax
	leaq	.LC4(%rip), %rcx
	leaq	.LC5(%rip), %rdx
	movq	%rax, %rdi
	call	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	movq	%rax, -48(%rbp)
	movq	-48(%rbp), %rdx
	movq	-88(%rbp), %rcx
	movq	-72(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID
	movq	%xmm0, %rax
	movq	%rax, -56(%rbp)
	xorps	%xmm2, %xmm2
	cvtsi2ss	-20(%rbp), %xmm2
	movd	%xmm2, %eax
	movd	%eax, %xmm0
	addss	-36(%rbp), %xmm0
	cvtss2sd	%xmm0, %xmm3
	movq	%xmm3, %rax
	movq	%rax, %xmm1
	addsd	-56(%rbp), %xmm1
	movq	%xmm1, %rax
	movq	%rax, %xmm0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj, .-Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj
	.ident	"GCC: (GNU) 4.9 20140514 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
