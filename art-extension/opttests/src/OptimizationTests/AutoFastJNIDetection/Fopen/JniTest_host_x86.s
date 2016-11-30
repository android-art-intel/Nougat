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
	.section	.rodata
.LC0:
	.string	"w+"
.LC1:
	.string	"file.txt"
.LC2:
	.string	"in"
.LC3:
	.string	"are"
.LC4:
	.string	"We"
.LC5:
	.string	"%s %s %s %d"
	.text
	.globl	Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen
	.type	Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen, @function
Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen:
.LFB248:
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
	leal	.LC0@GOTOFF(%ebx), %eax
	movl	%eax, 4(%esp)
	leal	.LC1@GOTOFF(%ebx), %eax
	movl	%eax, (%esp)
	call	fopen@PLT
	movl	%eax, -12(%ebp)
	movl	$2016, 20(%esp)
	leal	.LC2@GOTOFF(%ebx), %eax
	movl	%eax, 16(%esp)
	leal	.LC3@GOTOFF(%ebx), %eax
	movl	%eax, 12(%esp)
	leal	.LC4@GOTOFF(%ebx), %eax
	movl	%eax, 8(%esp)
	leal	.LC5@GOTOFF(%ebx), %eax
	movl	%eax, 4(%esp)
	movl	-12(%ebp), %eax
	movl	%eax, (%esp)
	call	fprintf@PLT
	movl	-12(%ebp), %eax
	movl	%eax, (%esp)
	call	fclose@PLT
	movl	16(%ebp), %eax
	addl	$52, %esp
	popl	%ebx
	.cfi_restore 3
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE248:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen, .-Java_OptimizationTests_AutoFastJNIDetection_Fopen_Main_nativeFopen
	.section	.text.__i686.get_pc_thunk.bx,"axG",@progbits,__i686.get_pc_thunk.bx,comdat
	.globl	__i686.get_pc_thunk.bx
	.hidden	__i686.get_pc_thunk.bx
	.type	__i686.get_pc_thunk.bx, @function
__i686.get_pc_thunk.bx:
.LFB249:
	.cfi_startproc
	movl	(%esp), %ebx
	ret
	.cfi_endproc
.LFE249:
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
