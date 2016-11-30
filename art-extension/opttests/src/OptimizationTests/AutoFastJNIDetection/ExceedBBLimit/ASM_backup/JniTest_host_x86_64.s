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
.LFB237:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movl	%edx, -20(%rbp)
	movl	%ecx, -24(%rbp)
	movl	%r8d, -28(%rbp)
	movl	-20(%rbp), %eax
	cmpl	-24(%rbp), %eax
	jle	.L2
	movl	-24(%rbp), %eax
	movl	-28(%rbp), %edx
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	cmpl	-20(%rbp), %eax
	jle	.L3
	movl	-24(%rbp), %eax
	addl	%eax, -20(%rbp)
	jmp	.L4
.L3:
	movl	-20(%rbp), %eax
	imull	-24(%rbp), %eax
	cmpl	-28(%rbp), %eax
	jne	.L4
	movl	-20(%rbp), %eax
	movl	%eax, -28(%rbp)
	jmp	.L4
.L2:
	movl	-20(%rbp), %eax
	cmpl	-24(%rbp), %eax
	je	.L5
	movl	-20(%rbp), %eax
	imull	-24(%rbp), %eax
	movl	%eax, -20(%rbp)
	jmp	.L4
.L5:
	movl	-24(%rbp), %eax
	imull	-20(%rbp), %eax
	movl	%eax, -24(%rbp)
.L4:
	movl	-20(%rbp), %eax
	cmpl	-24(%rbp), %eax
	je	.L6
	movl	-24(%rbp), %eax
	movl	-28(%rbp), %edx
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	cmpl	-20(%rbp), %eax
	jl	.L7
	movl	-24(%rbp), %eax
	movl	%eax, -20(%rbp)
	jmp	.L7
.L6:
	movl	-20(%rbp), %eax
	cmpl	-24(%rbp), %eax
	je	.L8
	movl	-20(%rbp), %eax
	imull	-24(%rbp), %eax
	movl	%eax, -20(%rbp)
	jmp	.L7
.L8:
	movl	-24(%rbp), %eax
	imull	-20(%rbp), %eax
	movl	%eax, -24(%rbp)
.L7:
	movl	-20(%rbp), %eax
	cmpl	-28(%rbp), %eax
	je	.L9
	movl	-24(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -20(%rbp)
	jmp	.L10
.L9:
	movl	$0, -24(%rbp)
.L10:
	movl	-20(%rbp), %eax
	imull	-24(%rbp), %eax
	cmpl	-28(%rbp), %eax
	jne	.L11
	movl	-20(%rbp), %eax
	movl	%eax, -28(%rbp)
	jmp	.L12
.L11:
	movl	-20(%rbp), %eax
	cmpl	-28(%rbp), %eax
	je	.L13
	movl	-24(%rbp), %eax
	addl	$5, %eax
	movl	%eax, -20(%rbp)
	jmp	.L12
.L13:
	movl	$0, -24(%rbp)
.L12:
	cmpl	$0, -20(%rbp)
	jg	.L14
	addl	$1, -20(%rbp)
	jmp	.L15
.L14:
	addl	$2, -24(%rbp)
.L15:
	cmpl	$13, -20(%rbp)
	jle	.L16
	addl	$13, -20(%rbp)
	jmp	.L17
.L16:
	addl	$5, -24(%rbp)
.L17:
	cmpl	$13, -28(%rbp)
	jle	.L18
	addl	$13, -28(%rbp)
	jmp	.L19
.L18:
	addl	$5, -28(%rbp)
.L19:
	cmpl	$0, -24(%rbp)
	jg	.L20
	addl	$1, -24(%rbp)
	jmp	.L21
.L20:
	addl	$2, -24(%rbp)
.L21:
	cmpl	$13, -24(%rbp)
	jle	.L22
	addl	$13, -24(%rbp)
	jmp	.L23
.L22:
	addl	$5, -24(%rbp)
.L23:
	cmpl	$13, -28(%rbp)
	jle	.L24
	addl	$13, -28(%rbp)
	jmp	.L25
.L24:
	addl	$5, -28(%rbp)
.L25:
	cmpl	$0, -20(%rbp)
	je	.L26
	movl	-20(%rbp), %eax
	subl	%eax, -28(%rbp)
	jmp	.L27
.L26:
	movl	$0, -28(%rbp)
.L27:
	movl	-24(%rbp), %eax
	cmpl	-28(%rbp), %eax
	jle	.L28
	cmpl	$0, -24(%rbp)
	jns	.L29
	movl	-28(%rbp), %eax
	addl	%eax, -24(%rbp)
	jmp	.L30
.L29:
	movl	-24(%rbp), %eax
	imull	-28(%rbp), %eax
	cmpl	-28(%rbp), %eax
	jne	.L30
	movl	-24(%rbp), %eax
	movl	%eax, -28(%rbp)
	jmp	.L30
.L28:
	movl	-24(%rbp), %eax
	cmpl	-28(%rbp), %eax
	je	.L31
	movl	-24(%rbp), %eax
	imull	-28(%rbp), %eax
	movl	%eax, -24(%rbp)
	jmp	.L30
.L31:
	movl	-28(%rbp), %eax
	imull	-24(%rbp), %eax
	movl	%eax, -28(%rbp)
.L30:
	cmpl	$99, -20(%rbp)
	jg	.L32
	sall	-20(%rbp)
	jmp	.L33
.L32:
	addl	$2, -20(%rbp)
.L33:
	cmpl	$1000, -24(%rbp)
	jg	.L34
	sall	-24(%rbp)
	jmp	.L35
.L34:
	addl	$2, -24(%rbp)
.L35:
	cmpl	$9, -28(%rbp)
	jg	.L36
	sall	-28(%rbp)
	jmp	.L37
.L36:
	addl	$2, -28(%rbp)
.L37:
	movl	-24(%rbp), %eax
	movl	-20(%rbp), %edx
	addl	%edx, %eax
	addl	-28(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit, .-Java_OptimizationTests_AutoFastJNIDetection_ExceedBBLimit_Main_nativeExceedBBLimit
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
