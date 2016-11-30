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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching
	.type	Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching, @function
Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching:
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
	jne	.L5
	movl	-20(%rbp), %eax
	movl	%eax, -28(%rbp)
	jmp	.L4
.L5:
	movl	-20(%rbp), %eax
	cmpl	-28(%rbp), %eax
	je	.L6
	movl	-24(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -20(%rbp)
	jmp	.L4
.L6:
	movl	$0, -24(%rbp)
	jmp	.L4
.L2:
	movl	-20(%rbp), %eax
	cmpl	-24(%rbp), %eax
	je	.L7
	movl	-20(%rbp), %eax
	imull	-24(%rbp), %eax
	movl	%eax, -20(%rbp)
	jmp	.L4
.L7:
	movl	-24(%rbp), %eax
	imull	-20(%rbp), %eax
	movl	%eax, -24(%rbp)
.L4:
	movl	-24(%rbp), %eax
	movl	-20(%rbp), %edx
	addl	%edx, %eax
	addl	-28(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching, .-Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
