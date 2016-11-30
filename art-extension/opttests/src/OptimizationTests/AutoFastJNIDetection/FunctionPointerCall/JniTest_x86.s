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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall
	.type	Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall, @function
Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall:
.LFB234:
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
	leal	addInt@GOTOFF(%ebx), %eax
	movl	%eax, -12(%ebp)
	movl	$4, 8(%esp)
	movl	$3, 4(%esp)
	movl	$2, (%esp)
	movl	-12(%ebp), %eax
	call	*%eax
	movl	%eax, -16(%ebp)
	movl	-16(%ebp), %eax
	leal	36(%esp), %esp
	popl	%ebx
	.cfi_restore 3
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE234:
	.size	Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall, .-Java_OptimizationTests_AutoFastJNIDetection_FunctionPointerCall_Main_nativeFunctionPointerCall
	.globl	addInt
	.hidden	addInt
	.type	addInt, @function
addInt:
.LFB235:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	leal	(%esp), %ebp
	.cfi_def_cfa_register 5
	movl	12(%ebp), %eax
	movl	8(%ebp), %edx
	addl	%eax, %edx
	movl	16(%ebp), %eax
	addl	%edx, %eax
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE235:
	.size	addInt, .-addInt
	.section	.text.__x86.get_pc_thunk.bx,"axG",@progbits,__x86.get_pc_thunk.bx,comdat
	.globl	__x86.get_pc_thunk.bx
	.hidden	__x86.get_pc_thunk.bx
	.type	__x86.get_pc_thunk.bx, @function
__x86.get_pc_thunk.bx:
.LFB236:
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
.LFE236:
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",@progbits
