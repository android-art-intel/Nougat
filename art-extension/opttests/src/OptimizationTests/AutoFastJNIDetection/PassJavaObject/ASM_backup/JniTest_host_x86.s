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
.LFB31:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	124(%eax), %edx
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	*%edx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE31:
	.size	_ZN7JNIEnv_14GetObjectClassEP8_jobject, .-_ZN7JNIEnv_14GetObjectClassEP8_jobject
	.section	.text._ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_,"axG",@progbits,_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_,comdat
	.align 2
	.weak	_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_
	.hidden	_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_
	.type	_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_, @function
_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_:
.LFB94:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	376(%eax), %edx
	movl	20(%ebp), %eax
	movl	%eax, 12(%esp)
	movl	16(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	*%edx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE94:
	.size	_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_, .-_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_
	.section	.text._ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID,"axG",@progbits,_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID,comdat
	.align 2
	.weak	_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID
	.type	_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID, @function
_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID:
.LFB100:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	400(%eax), %edx
	movl	16(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	*%edx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE100:
	.size	_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID, .-_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID
	.section	.text._ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID,"axG",@progbits,_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID,comdat
	.align 2
	.weak	_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID
	.type	_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID, @function
_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID:
.LFB102:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	408(%eax), %edx
	movl	16(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	*%edx
	fstps	-12(%ebp)
	movl	-12(%ebp), %eax
	movl	%eax, -12(%ebp)
	flds	-12(%ebp)
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE102:
	.size	_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID, .-_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID
	.section	.text._ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID,"axG",@progbits,_ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID,comdat
	.align 2
	.weak	_ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID
	.hidden	_ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID
	.type	_ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID, @function
_ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID:
.LFB103:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	412(%eax), %edx
	movl	16(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	*%edx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE103:
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
.LFB238:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$52, %esp
	.cfi_offset 3, -12
	call	__i686.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	16(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN7JNIEnv_14GetObjectClassEP8_jobject
	movl	%eax, -32(%ebp)
	leal	.LC0@GOTOFF(%ebx), %eax
	movl	%eax, 12(%esp)
	leal	.LC1@GOTOFF(%ebx), %eax
	movl	%eax, 8(%esp)
	movl	-32(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_
	movl	%eax, -28(%ebp)
	movl	-28(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	16(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN7JNIEnv_11GetIntFieldEP8_jobjectP9_jfieldID
	movl	%eax, -24(%ebp)
	leal	.LC2@GOTOFF(%ebx), %eax
	movl	%eax, 12(%esp)
	leal	.LC3@GOTOFF(%ebx), %eax
	movl	%eax, 8(%esp)
	movl	-32(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_
	movl	%eax, -20(%ebp)
	movl	-20(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	16(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN7JNIEnv_13GetFloatFieldEP8_jobjectP9_jfieldID
	fstps	-16(%ebp)
	leal	.LC4@GOTOFF(%ebx), %eax
	movl	%eax, 12(%esp)
	leal	.LC5@GOTOFF(%ebx), %eax
	movl	%eax, 8(%esp)
	movl	-32(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN7JNIEnv_10GetFieldIDEP7_jclassPKcS3_
	movl	%eax, -12(%ebp)
	movl	-12(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	16(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN7JNIEnv_14GetDoubleFieldEP8_jobjectP9_jfieldID
	fstpl	-40(%ebp)
	fildl	-24(%ebp)
	fadds	-16(%ebp)
	faddl	-40(%ebp)
	addl	$52, %esp
	popl	%ebx
	.cfi_restore 3
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE238:
	.size	Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj, .-Java_OptimizationTests_AutoFastJNIDetection_PassJavaObject_Main_nativePassObj
	.section	.text.__i686.get_pc_thunk.bx,"axG",@progbits,__i686.get_pc_thunk.bx,comdat
	.globl	__i686.get_pc_thunk.bx
	.hidden	__i686.get_pc_thunk.bx
	.type	__i686.get_pc_thunk.bx, @function
__i686.get_pc_thunk.bx:
.LFB239:
	.cfi_startproc
	movl	(%esp), %ebx
	ret
	.cfi_endproc
.LFE239:
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
