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
	.hidden	count
	.globl	count
	.data
	.align 4
	.type	count, @object
	.size	count, 4
count:
	.long	5
	.text
	.globl	Java_OptimizationTests_AutoFastJNIDetection_LockPrefix_Main_nativeLockPrefix
	.type	Java_OptimizationTests_AutoFastJNIDetection_LockPrefix_Main_nativeLockPrefix, @function
Java_OptimizationTests_AutoFastJNIDetection_LockPrefix_Main_nativeLockPrefix:
.LFB238:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%edi
	pushl	%esi
	subl	$8, %esp
	.cfi_offset 6, -16
	.cfi_offset 7, -12
	call	__i686.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
	movl	16(%ebp), %eax
	movl	%eax, -16(%ebp)
	movl	20(%ebp), %eax
	movl	%eax, -12(%ebp)
	movl	count@GOTOFF(%ecx), %eax
	movl	%eax, %edx
	sarl	$31, %edx
	movl	%eax, %esi
	movl	%edx, %edi
	addl	-16(%ebp), %esi
	adcl	-12(%ebp), %edi
	movl	$1, %eax
	lock xaddl	%eax, count@GOTOFF(%ecx)
	movl	%eax, %edx
	sarl	$31, %edx
	addl	%esi, %eax
	adcl	%edi, %edx
	addl	$8, %esp
	popl	%esi
	.cfi_restore 6
	popl	%edi
	.cfi_restore 7
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE238:
	.size	Java_OptimizationTests_AutoFastJNIDetection_LockPrefix_Main_nativeLockPrefix, .-Java_OptimizationTests_AutoFastJNIDetection_LockPrefix_Main_nativeLockPrefix
	.section	.text.__i686.get_pc_thunk.cx,"axG",@progbits,__i686.get_pc_thunk.cx,comdat
	.globl	__i686.get_pc_thunk.cx
	.hidden	__i686.get_pc_thunk.cx
	.type	__i686.get_pc_thunk.cx, @function
__i686.get_pc_thunk.cx:
.LFB239:
	.cfi_startproc
	movl	(%esp), %ecx
	ret
	.cfi_endproc
.LFE239:
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
