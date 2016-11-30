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
.LFB234:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	leal	(%esp), %ebp
	.cfi_def_cfa_register 5
	leal	-24(%esp), %esp
	cmpl	$0, 8(%ebp)
	jle	.L2
	movl	12(%ebp), %eax
	movl	%eax, %edx
	addl	$5, %edx
	movl	8(%ebp), %eax
	subl	$1, %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	_Z3fooii
	jmp	.L3
.L2:
	movl	12(%ebp), %eax
.L3:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE234:
	.size	_Z3fooii, .-_Z3fooii
	.globl	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCall_Main_nativeRecursiveCall
	.type	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCall_Main_nativeRecursiveCall, @function
Java_OptimizationTests_AutoFastJNIDetection_RecursiveCall_Main_nativeRecursiveCall:
.LFB235:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	leal	(%esp), %ebp
	.cfi_def_cfa_register 5
	leal	-24(%esp), %esp
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
.LFE235:
	.size	Java_OptimizationTests_AutoFastJNIDetection_RecursiveCall_Main_nativeRecursiveCall, .-Java_OptimizationTests_AutoFastJNIDetection_RecursiveCall_Main_nativeRecursiveCall
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",@progbits
