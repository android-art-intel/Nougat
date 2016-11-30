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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit
	.type	Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit, @function
Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit:
.LFB234:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	leal	(%esp), %ebp
	.cfi_def_cfa_register 5
	movl	16(%ebp), %eax
	cmpl	20(%ebp), %eax
	jle	.L2
	movl	20(%ebp), %eax
	movl	24(%ebp), %edx
	subl	%eax, %edx
	movl	%edx, %eax
	cmpl	16(%ebp), %eax
	jle	.L3
	movl	20(%ebp), %eax
	addl	%eax, 16(%ebp)
	jmp	.L4
.L3:
	movl	16(%ebp), %eax
	imull	20(%ebp), %eax
	cmpl	24(%ebp), %eax
	jne	.L4
	movl	16(%ebp), %eax
	movl	%eax, 24(%ebp)
	jmp	.L4
.L2:
	movl	16(%ebp), %eax
	cmpl	20(%ebp), %eax
	je	.L5
	movl	16(%ebp), %eax
	imull	20(%ebp), %eax
	movl	%eax, 16(%ebp)
	jmp	.L4
.L5:
	movl	20(%ebp), %eax
	imull	16(%ebp), %eax
	movl	%eax, 20(%ebp)
.L4:
	movl	16(%ebp), %eax
	cmpl	20(%ebp), %eax
	je	.L6
	movl	20(%ebp), %eax
	movl	24(%ebp), %edx
	subl	%eax, %edx
	movl	%edx, %eax
	cmpl	16(%ebp), %eax
	jl	.L7
	movl	20(%ebp), %eax
	movl	%eax, 16(%ebp)
	jmp	.L7
.L6:
	movl	16(%ebp), %eax
	cmpl	20(%ebp), %eax
	je	.L8
	movl	16(%ebp), %eax
	imull	20(%ebp), %eax
	movl	%eax, 16(%ebp)
	jmp	.L7
.L8:
	movl	20(%ebp), %eax
	imull	16(%ebp), %eax
	movl	%eax, 20(%ebp)
.L7:
	movl	16(%ebp), %eax
	cmpl	24(%ebp), %eax
	je	.L9
	movl	20(%ebp), %eax
	addl	$1, %eax
	movl	%eax, 16(%ebp)
	jmp	.L10
.L9:
	movl	$0, 20(%ebp)
.L10:
	movl	16(%ebp), %eax
	imull	20(%ebp), %eax
	cmpl	24(%ebp), %eax
	jne	.L11
	movl	16(%ebp), %eax
	movl	%eax, 24(%ebp)
	jmp	.L12
.L11:
	movl	16(%ebp), %eax
	cmpl	24(%ebp), %eax
	je	.L13
	movl	20(%ebp), %eax
	addl	$5, %eax
	movl	%eax, 16(%ebp)
	jmp	.L12
.L13:
	movl	$0, 20(%ebp)
.L12:
	cmpl	$0, 16(%ebp)
	jg	.L14
	addl	$1, 16(%ebp)
	jmp	.L15
.L14:
	addl	$2, 20(%ebp)
.L15:
	cmpl	$13, 16(%ebp)
	jle	.L16
	addl	$13, 16(%ebp)
	jmp	.L17
.L16:
	addl	$5, 20(%ebp)
.L17:
	cmpl	$13, 24(%ebp)
	jle	.L18
	addl	$13, 24(%ebp)
	jmp	.L19
.L18:
	addl	$5, 24(%ebp)
.L19:
	cmpl	$0, 20(%ebp)
	jg	.L20
	addl	$1, 20(%ebp)
	jmp	.L21
.L20:
	addl	$2, 20(%ebp)
.L21:
	cmpl	$13, 20(%ebp)
	jle	.L22
	addl	$13, 20(%ebp)
	jmp	.L23
.L22:
	addl	$5, 20(%ebp)
.L23:
	cmpl	$13, 24(%ebp)
	jle	.L24
	addl	$13, 24(%ebp)
	jmp	.L25
.L24:
	addl	$5, 24(%ebp)
.L25:
	cmpl	$0, 16(%ebp)
	je	.L26
	movl	16(%ebp), %eax
	subl	%eax, 24(%ebp)
	jmp	.L27
.L26:
	movl	$0, 24(%ebp)
.L27:
	movl	20(%ebp), %eax
	cmpl	24(%ebp), %eax
	jle	.L28
	cmpl	$0, 20(%ebp)
	jns	.L29
	movl	24(%ebp), %eax
	addl	%eax, 20(%ebp)
	jmp	.L30
.L29:
	movl	20(%ebp), %eax
	imull	24(%ebp), %eax
	cmpl	24(%ebp), %eax
	jne	.L30
	movl	20(%ebp), %eax
	movl	%eax, 24(%ebp)
	jmp	.L30
.L28:
	movl	20(%ebp), %eax
	cmpl	24(%ebp), %eax
	je	.L31
	movl	20(%ebp), %eax
	imull	24(%ebp), %eax
	movl	%eax, 20(%ebp)
	jmp	.L30
.L31:
	movl	24(%ebp), %eax
	imull	20(%ebp), %eax
	movl	%eax, 24(%ebp)
.L30:
	cmpl	$99, 16(%ebp)
	jg	.L32
	sall	16(%ebp)
	jmp	.L33
.L32:
	addl	$2, 16(%ebp)
.L33:
	cmpl	$1000, 20(%ebp)
	jg	.L34
	sall	20(%ebp)
	jmp	.L35
.L34:
	addl	$2, 20(%ebp)
.L35:
	cmpl	$9, 24(%ebp)
	jg	.L36
	sall	24(%ebp)
	jmp	.L37
.L36:
	addl	$2, 24(%ebp)
.L37:
	movl	20(%ebp), %eax
	movl	16(%ebp), %edx
	addl	%eax, %edx
	movl	24(%ebp), %eax
	addl	%edx, %eax
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE234:
	.size	Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit, .-Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",@progbits
