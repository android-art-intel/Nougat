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
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	cmpl	$0, -4(%rbp)
	jle	.L2
	movl	-8(%rbp), %eax
	leal	5(%rax), %edx
	movl	-4(%rbp), %eax
	subl	$1, %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	_Z3fooii
	addl	$5, %eax
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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallNotTail_Main_nativeRecursiveCallNotTail
	.type	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallNotTail_Main_nativeRecursiveCallNotTail, @function
Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallNotTail_Main_nativeRecursiveCallNotTail:
.LFB238:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movl	%edx, -20(%rbp)
	movl	%ecx, -24(%rbp)
	movl	%r8d, -28(%rbp)
	movl	-24(%rbp), %eax
	movl	-20(%rbp), %edx
	addl	%eax, %edx
	movl	-28(%rbp), %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	_Z3fooii
	addl	$1, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE238:
	.size	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallNotTail_Main_nativeRecursiveCallNotTail, .-Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallNotTail_Main_nativeRecursiveCallNotTail
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
