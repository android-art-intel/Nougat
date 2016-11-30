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
	.globl	_Z3fooii
	.hidden	_Z3fooii
	.type	_Z3fooii, @function
_Z3fooii:
.LFB237:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	-16(%rsp), %rsp
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	cmpl	$0, -4(%rbp)
	jle	.L2
	movl	-8(%rbp), %eax
	leal	5(%rax), %edx
	movl	-4(%rbp), %eax
	subl	$100, %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	_Z3barii
	jmp	.L3
.L2:
	movl	-8(%rbp), %eax
.L3:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE237:
	.size	_Z3fooii, .-_Z3fooii
	.globl	_Z3barii
	.hidden	_Z3barii
	.type	_Z3barii, @function
_Z3barii:
.LFB238:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	-16(%rsp), %rsp
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	cmpl	$1, -4(%rbp)
	jle	.L5
	movl	-8(%rbp), %eax
	leal	-2(%rax), %edx
	movl	-4(%rbp), %eax
	subl	$200, %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	_Z3fooii
	jmp	.L6
.L5:
	movl	-8(%rbp), %eax
	subl	$1, %eax
.L6:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE238:
	.size	_Z3barii, .-_Z3barii
	.globl	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallMutual_Main_nativeRecursiveCallMutual
	.type	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallMutual_Main_nativeRecursiveCallMutual, @function
Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallMutual_Main_nativeRecursiveCallMutual:
.LFB239:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	-32(%rsp), %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movl	%edx, -20(%rbp)
	movl	%ecx, -24(%rbp)
	movl	%r8d, -28(%rbp)
	movl	-20(%rbp), %edx
	movl	-24(%rbp), %eax
	addl	%eax, %edx
	movl	-28(%rbp), %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	_Z3fooii
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE239:
	.size	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallMutual_Main_nativeRecursiveCallMutual, .-Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallMutual_Main_nativeRecursiveCallMutual
	.ident	"GCC: (GNU) 4.9 20140514 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
