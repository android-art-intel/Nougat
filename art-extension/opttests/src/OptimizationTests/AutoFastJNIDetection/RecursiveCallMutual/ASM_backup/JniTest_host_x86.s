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
.LFB238:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	cmpl	$0, 8(%ebp)
	jle	.L2
	movl	12(%ebp), %eax
	leal	5(%eax), %edx
	movl	8(%ebp), %eax
	subl	$100, %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	_Z3barii
	jmp	.L3
.L2:
	movl	12(%ebp), %eax
.L3:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE238:
	.size	_Z3fooii, .-_Z3fooii
	.globl	_Z3barii
	.hidden	_Z3barii
	.type	_Z3barii, @function
_Z3barii:
.LFB239:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	cmpl	$1, 8(%ebp)
	jle	.L5
	movl	12(%ebp), %eax
	leal	-2(%eax), %edx
	movl	8(%ebp), %eax
	subl	$200, %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	_Z3fooii
	jmp	.L6
.L5:
	movl	12(%ebp), %eax
	subl	$1, %eax
.L6:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE239:
	.size	_Z3barii, .-_Z3barii
	.globl	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallMutual_Main_nativeRecursiveCallMutual
	.type	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallMutual_Main_nativeRecursiveCallMutual, @function
Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallMutual_Main_nativeRecursiveCallMutual:
.LFB240:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	movl	20(%ebp), %eax
	movl	16(%ebp), %edx
	addl	%edx, %eax
	movl	%eax, 4(%esp)
	movl	24(%ebp), %eax
	movl	%eax, (%esp)
	call	_Z3fooii
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE240:
	.size	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallMutual_Main_nativeRecursiveCallMutual, .-Java_OptimizationTests_AutoFastJNIDetection_RecursiveCallMutual_Main_nativeRecursiveCallMutual
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
