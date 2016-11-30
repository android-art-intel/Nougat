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
	.text
	.globl	Java_OptimizationTests_AutoFastJNIDetection_Malloc_Main_nativeMalloc
	.type	Java_OptimizationTests_AutoFastJNIDetection_Malloc_Main_nativeMalloc, @function
Java_OptimizationTests_AutoFastJNIDetection_Malloc_Main_nativeMalloc:
.LFB248:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	leal	-36(%esp), %esp
	.cfi_offset 3, -12
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	$4, (%esp)
	call	malloc@PLT
	movl	%eax, -12(%ebp)
	cmpl	$0, -12(%ebp)
	jne	.L2
	movl	$-1, %eax
	jmp	.L3
.L2:
	movl	-12(%ebp), %eax
	movl	16(%ebp), %edx
	movl	%edx, (%eax)
	movl	-12(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, -16(%ebp)
	movl	-12(%ebp), %eax
	movl	%eax, (%esp)
	call	free@PLT
	movl	-16(%ebp), %eax
.L3:
	leal	36(%esp), %esp
	popl	%ebx
	.cfi_restore 3
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE248:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Malloc_Main_nativeMalloc, .-Java_OptimizationTests_AutoFastJNIDetection_Malloc_Main_nativeMalloc
	.section	.text.__x86.get_pc_thunk.bx,"axG",@progbits,__x86.get_pc_thunk.bx,comdat
	.globl	__x86.get_pc_thunk.bx
	.hidden	__x86.get_pc_thunk.bx
	.type	__x86.get_pc_thunk.bx, @function
__x86.get_pc_thunk.bx:
.LFB249:
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
.LFE249:
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",@progbits
