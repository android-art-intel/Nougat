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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset
	.type	Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset, @function
Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset:
.LFB237:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$304, %rsp
	movq	%rdi, -408(%rbp)
	movq	%rsi, -416(%rbp)
	movl	%edx, -420(%rbp)
	leaq	-400(%rbp), %rax
	movl	-420(%rbp), %esi
	movq	%rax, %rdx
	movzbl	%sil, %ecx
	movabsq	$72340172838076673, %rax
	imulq	%rcx, %rax
	movl	$50, %ecx
	movq	%rdx, %rdi
	rep stosq
	movq	%rdi, %rdx
	movl	$400, %eax
	andl	$7, %eax
	movl	%eax, %ecx
	andb	$255, %ch
	testl	%ecx, %ecx
	je	.L3
	movl	$0, %eax
.L2:
	movl	%eax, %edi
	movb	%sil, (%rdx,%rdi)
	addl	$1, %eax
	cmpl	%ecx, %eax
	jb	.L2
	movl	%eax, %eax
	addq	%rax, %rdx
.L3:
	movl	-380(%rbp), %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset, .-Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
