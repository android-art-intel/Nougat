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
	.hidden	count_mutex
	.globl	count_mutex
	.bss
	.align 4
	.type	count_mutex, @object
	.size	count_mutex, 4
count_mutex:
	.zero	4
	.hidden	count
	.globl	count
	.data
	.align 4
	.type	count, @object
	.size	count, 4
count:
	.long	5
	.text
	.globl	Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex
	.type	Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex, @function
Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex:
.LFB244:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$20, %esp
	.cfi_offset 3, -12
	call	__i686.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	leal	count_mutex@GOTOFF(%ebx), %eax
	movl	%eax, (%esp)
	call	pthread_mutex_lock@PLT
	movl	count@GOTOFF(%ebx), %eax
	addl	$1, %eax
	movl	%eax, count@GOTOFF(%ebx)
	leal	count_mutex@GOTOFF(%ebx), %eax
	movl	%eax, (%esp)
	call	pthread_mutex_unlock@PLT
	movl	count@GOTOFF(%ebx), %eax
	addl	16(%ebp), %eax
	addl	$20, %esp
	popl	%ebx
	.cfi_restore 3
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE244:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex, .-Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex
	.section	.text.__i686.get_pc_thunk.bx,"axG",@progbits,__i686.get_pc_thunk.bx,comdat
	.globl	__i686.get_pc_thunk.bx
	.hidden	__i686.get_pc_thunk.bx
	.type	__i686.get_pc_thunk.bx, @function
__i686.get_pc_thunk.bx:
.LFB245:
	.cfi_startproc
	movl	(%esp), %ebx
	ret
	.cfi_endproc
.LFE245:
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
