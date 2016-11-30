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
.LFB27:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	leal	(%esp), %ebp
	.cfi_def_cfa_register 5
	leal	-24(%esp), %esp
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	124(%eax), %eax
	movl	12(%ebp), %edx
	movl	%edx, 4(%esp)
	movl	8(%ebp), %edx
	movl	%edx, (%esp)
	call	*%eax
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE27:
	.size	_ZN7_JNIEnv14GetObjectClassEP8_jobject, .-_ZN7_JNIEnv14GetObjectClassEP8_jobject
	.section	.text._ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_,"axG",@progbits,_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_,comdat
	.align 2
	.weak	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	.hidden	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	.type	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_, @function
_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_:
.LFB90:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	leal	(%esp), %ebp
	.cfi_def_cfa_register 5
	leal	-24(%esp), %esp
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	376(%eax), %eax
	movl	20(%ebp), %edx
	movl	%edx, 12(%esp)
	movl	16(%ebp), %edx
	movl	%edx, 8(%esp)
	movl	12(%ebp), %edx
	movl	%edx, 4(%esp)
	movl	8(%ebp), %edx
	movl	%edx, (%esp)
	call	*%eax
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE90:
	.size	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_, .-_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	.section	.text._ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID,"axG",@progbits,_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID,comdat
	.align 2
	.weak	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	.type	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID, @function
_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID:
.LFB96:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	leal	(%esp), %ebp
	.cfi_def_cfa_register 5
	leal	-24(%esp), %esp
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	400(%eax), %eax
	movl	16(%ebp), %edx
	movl	%edx, 8(%esp)
	movl	12(%ebp), %edx
	movl	%edx, 4(%esp)
	movl	8(%ebp), %edx
	movl	%edx, (%esp)
	call	*%eax
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE96:
	.size	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID, .-_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	.section	.text._ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID,"axG",@progbits,_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID,comdat
	.align 2
	.weak	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	.type	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID, @function
_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID:
.LFB98:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	leal	(%esp), %ebp
	.cfi_def_cfa_register 5
	leal	-24(%esp), %esp
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	408(%eax), %eax
	movl	16(%ebp), %edx
	movl	%edx, 8(%esp)
	movl	12(%ebp), %edx
	movl	%edx, 4(%esp)
	movl	8(%ebp), %edx
	movl	%edx, (%esp)
	call	*%eax
	fstps	-12(%ebp)
	movl	-12(%ebp), %eax
	movl	%eax, -12(%ebp)
	flds	-12(%ebp)
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE98:
	.size	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID, .-_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	.section	.text._ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID,"axG",@progbits,_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID,comdat
	.align 2
	.weak	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID
	.type	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID, @function
_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID:
.LFB99:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	leal	(%esp), %ebp
	.cfi_def_cfa_register 5
	leal	-40(%esp), %esp
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	412(%eax), %eax
	movl	16(%ebp), %edx
	movl	%edx, 8(%esp)
	movl	12(%ebp), %edx
	movl	%edx, 4(%esp)
	movl	8(%ebp), %edx
	movl	%edx, (%esp)
	call	*%eax
	fstpl	-16(%ebp)
	movsd	-16(%ebp), %xmm0
	movsd	%xmm0, -16(%ebp)
	fldl	-16(%ebp)
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE99:
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
.LFB234:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	leal	-68(%esp), %esp
	.cfi_offset 3, -12
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	16(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN7_JNIEnv14GetObjectClassEP8_jobject
	movl	%eax, -12(%ebp)
	leal	.LC0@GOTOFF(%ebx), %eax
	movl	%eax, 12(%esp)
	leal	.LC1@GOTOFF(%ebx), %eax
	movl	%eax, 8(%esp)
	movl	-12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	movl	%eax, -16(%ebp)
	movl	-16(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	16(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN7_JNIEnv11GetIntFieldEP8_jobjectP9_jfieldID
	movl	%eax, -20(%ebp)
	leal	.LC2@GOTOFF(%ebx), %eax
	movl	%eax, 12(%esp)
	leal	.LC3@GOTOFF(%ebx), %eax
	movl	%eax, 8(%esp)
	movl	-12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	movl	%eax, -24(%ebp)
	movl	-24(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	16(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN7_JNIEnv13GetFloatFieldEP8_jobjectP9_jfieldID
	fstps	-48(%ebp)
	movl	-48(%ebp), %eax
	movl	%eax, -28(%ebp)
	leal	.LC4@GOTOFF(%ebx), %eax
	movl	%eax, 12(%esp)
	leal	.LC5@GOTOFF(%ebx), %eax
	movl	%eax, 8(%esp)
	movl	-12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN7_JNIEnv10GetFieldIDEP7_jclassPKcS3_
	movl	%eax, -32(%ebp)
	movl	-32(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	16(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN7_JNIEnv14GetDoubleFieldEP8_jobjectP9_jfieldID
	fstpl	-48(%ebp)
	movsd	-48(%ebp), %xmm0
	movsd	%xmm0, -40(%ebp)
	cvtsi2ss	-20(%ebp), %xmm1
	movd	%xmm1, %eax
	movd	%eax, %xmm0
	addss	-28(%ebp), %xmm0
	cvtss2sd	%xmm0, %xmm0
	addsd	-40(%ebp), %xmm0
	movsd	%xmm0, -48(%ebp)
	fldl	-48(%ebp)
	leal	68(%esp), %esp
	popl	%ebx
	.cfi_restore 3
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE234:
	.size	Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj, .-Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj
	.section	.text.__x86.get_pc_thunk.bx,"axG",@progbits,__x86.get_pc_thunk.bx,comdat
	.globl	__x86.get_pc_thunk.bx
	.hidden	__x86.get_pc_thunk.bx
	.type	__x86.get_pc_thunk.bx, @function
__x86.get_pc_thunk.bx:
.LFB235:
	.cfi_startproc
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	movl	(%esp), %ebx
	ret
	.cfi_endproc
.LFE235:
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",@progbits
