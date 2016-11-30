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
.LFB238:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%edi
	pushl	%esi
	subl	$404, %esp
	leal	-408(%ebp), %eax
	movl	16(%ebp), %esi
	.cfi_offset 6, -16
	.cfi_offset 7, -12
	movl	%eax, %edx
	movl	%esi, %eax
	movzbl	%al, %ecx
	movl	$16843009, %eax
	imull	%ecx, %eax
	movl	$100, %ecx
	movl	%edx, %edi
	rep stosl
	movl	%edi, %edx
	movl	$400, %eax
	andl	$3, %eax
	movl	%eax, %ecx
	andb	$255, %ch
	movl	%ecx, -412(%ebp)
	cmpl	$0, -412(%ebp)
	je	.L3
	movl	$0, %eax
.L2:
	movl	%esi, %ecx
	movb	%cl, (%edx,%eax)
	addl	$1, %eax
	cmpl	-412(%ebp), %eax
	jb	.L2
	addl	%eax, %edx
.L3:
	movl	-388(%ebp), %eax
	addl	$404, %esp
	popl	%esi
	.cfi_restore 6
	popl	%edi
	.cfi_restore 7
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE238:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset, .-Java_OptimizationTests_AutoFastJNIDetection_Memset_Main_nativeMemset
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
