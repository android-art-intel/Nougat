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
.LFB238:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	movl	16(%ebp), %eax
	cmpl	20(%ebp), %eax
	jle	.L2
	movl	20(%ebp), %eax
	movl	24(%ebp), %edx
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	cmpl	16(%ebp), %eax
	jle	.L3
	movl	20(%ebp), %eax
	addl	%eax, 16(%ebp)
	jmp	.L4
.L3:
	movl	16(%ebp), %eax
	imull	20(%ebp), %eax
	cmpl	24(%ebp), %eax
	jne	.L5
	movl	16(%ebp), %eax
	movl	%eax, 24(%ebp)
	jmp	.L4
.L5:
	movl	16(%ebp), %eax
	cmpl	24(%ebp), %eax
	je	.L6
	movl	20(%ebp), %eax
	addl	$1, %eax
	movl	%eax, 16(%ebp)
	jmp	.L4
.L6:
	movl	$0, 20(%ebp)
	jmp	.L4
.L2:
	movl	16(%ebp), %eax
	cmpl	20(%ebp), %eax
	je	.L7
	movl	16(%ebp), %eax
	imull	20(%ebp), %eax
	movl	%eax, 16(%ebp)
	jmp	.L4
.L7:
	movl	20(%ebp), %eax
	imull	16(%ebp), %eax
	movl	%eax, 20(%ebp)
.L4:
	movl	20(%ebp), %eax
	movl	16(%ebp), %edx
	addl	%edx, %eax
	addl	24(%ebp), %eax
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE238:
	.size	Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching, .-Java_OptimizationTests_AutoFastJNIDetection_ComplexBranching_Main_nativeComplexBranching
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
