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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit
	.type	Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit, @function
Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit:
.LFB234:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	leal	(%esp), %ebp
	.cfi_def_cfa_register 5
	leal	-128(%esp), %esp
	movl	16(%ebp), %eax
	addl	$1, %eax
	movl	%eax, -4(%ebp)
	movl	-4(%ebp), %eax
	movl	%eax, %edx
	addl	$2, %edx
	movl	20(%ebp), %eax
	addl	%edx, %eax
	movl	%eax, -8(%ebp)
	movl	-8(%ebp), %eax
	movl	%eax, %edx
	addl	$3, %edx
	movl	24(%ebp), %eax
	addl	%edx, %eax
	movl	%eax, -12(%ebp)
	movl	-4(%ebp), %eax
	movl	%eax, %edx
	subl	$4, %edx
	movl	20(%ebp), %eax
	addl	%edx, %eax
	movl	%eax, -16(%ebp)
	movl	-12(%ebp), %eax
	movl	%eax, %edx
	subl	$4, %edx
	movl	16(%ebp), %eax
	imull	20(%ebp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -20(%ebp)
	movl	-12(%ebp), %eax
	imull	-8(%ebp), %eax
	movl	%eax, %edx
	subl	$4, %edx
	movl	16(%ebp), %eax
	imull	20(%ebp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -24(%ebp)
	movl	-12(%ebp), %eax
	imull	-4(%ebp), %eax
	movl	%eax, %edx
	movl	20(%ebp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	addl	%eax, %edx
	movl	16(%ebp), %eax
	imull	20(%ebp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -28(%ebp)
	movl	-12(%ebp), %eax
	imull	-8(%ebp), %eax
	movl	%eax, %edx
	movl	24(%ebp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	movl	%edx, %ecx
	addl	%eax, %ecx
	movl	16(%ebp), %edx
	movl	$0, %eax
	subl	%edx, %eax
	sall	$2, %eax
	addl	%ecx, %eax
	movl	%eax, -32(%ebp)
	movl	-28(%ebp), %eax
	imull	-4(%ebp), %eax
	movl	%eax, %edx
	movl	16(%ebp), %eax
	imull	$-6, %eax, %eax
	movl	%edx, %ecx
	addl	%eax, %ecx
	movl	20(%ebp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	negl	%eax
	addl	%ecx, %eax
	movl	%eax, -36(%ebp)
	movl	-24(%ebp), %eax
	movl	-28(%ebp), %edx
	subl	%eax, %edx
	movl	%edx, %eax
	subl	-20(%ebp), %eax
	movl	%eax, -40(%ebp)
	movl	20(%ebp), %eax
	addl	$1, %eax
	movl	%eax, -44(%ebp)
	movl	-44(%ebp), %eax
	movl	%eax, %edx
	addl	$2, %edx
	movl	20(%ebp), %eax
	addl	%edx, %eax
	movl	%eax, -48(%ebp)
	movl	-48(%ebp), %eax
	movl	%eax, %edx
	addl	$3, %edx
	movl	24(%ebp), %eax
	addl	%edx, %eax
	movl	%eax, -52(%ebp)
	movl	-44(%ebp), %eax
	movl	%eax, %edx
	subl	$4, %edx
	movl	20(%ebp), %eax
	addl	%edx, %eax
	movl	%eax, -56(%ebp)
	movl	-52(%ebp), %eax
	movl	%eax, %edx
	subl	$4, %edx
	movl	20(%ebp), %eax
	imull	20(%ebp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -60(%ebp)
	movl	-52(%ebp), %eax
	imull	-48(%ebp), %eax
	movl	%eax, %edx
	subl	$4, %edx
	movl	20(%ebp), %eax
	imull	20(%ebp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -64(%ebp)
	movl	-52(%ebp), %eax
	imull	-44(%ebp), %eax
	movl	%eax, %edx
	movl	20(%ebp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	addl	%eax, %edx
	movl	20(%ebp), %eax
	imull	20(%ebp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -68(%ebp)
	movl	-52(%ebp), %eax
	imull	-48(%ebp), %eax
	movl	%eax, %edx
	movl	24(%ebp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	movl	%edx, %ecx
	addl	%eax, %ecx
	movl	20(%ebp), %edx
	movl	$0, %eax
	subl	%edx, %eax
	sall	$2, %eax
	addl	%ecx, %eax
	movl	%eax, -72(%ebp)
	movl	-68(%ebp), %eax
	imull	-44(%ebp), %eax
	movl	%eax, %edx
	movl	20(%ebp), %eax
	imull	$-6, %eax, %eax
	movl	%edx, %ecx
	addl	%eax, %ecx
	movl	20(%ebp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	negl	%eax
	addl	%ecx, %eax
	movl	%eax, -76(%ebp)
	movl	-64(%ebp), %eax
	movl	-68(%ebp), %edx
	subl	%eax, %edx
	movl	%edx, %eax
	subl	-60(%ebp), %eax
	movl	%eax, -80(%ebp)
	movl	24(%ebp), %eax
	addl	$1, %eax
	movl	%eax, -84(%ebp)
	movl	-84(%ebp), %eax
	movl	%eax, %edx
	addl	$2, %edx
	movl	20(%ebp), %eax
	addl	%edx, %eax
	movl	%eax, -88(%ebp)
	movl	-88(%ebp), %eax
	movl	%eax, %edx
	addl	$3, %edx
	movl	24(%ebp), %eax
	addl	%edx, %eax
	movl	%eax, -92(%ebp)
	movl	-84(%ebp), %eax
	movl	%eax, %edx
	subl	$4, %edx
	movl	20(%ebp), %eax
	addl	%edx, %eax
	movl	%eax, -96(%ebp)
	movl	-92(%ebp), %eax
	movl	%eax, %edx
	subl	$4, %edx
	movl	24(%ebp), %eax
	imull	20(%ebp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -100(%ebp)
	movl	-92(%ebp), %eax
	imull	-88(%ebp), %eax
	movl	%eax, %edx
	subl	$4, %edx
	movl	24(%ebp), %eax
	imull	20(%ebp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -104(%ebp)
	movl	-92(%ebp), %eax
	imull	-84(%ebp), %eax
	movl	%eax, %edx
	movl	20(%ebp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	addl	%eax, %edx
	movl	24(%ebp), %eax
	imull	20(%ebp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -108(%ebp)
	movl	-92(%ebp), %eax
	imull	-88(%ebp), %eax
	movl	%eax, %edx
	movl	24(%ebp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	movl	%edx, %ecx
	addl	%eax, %ecx
	movl	24(%ebp), %edx
	movl	$0, %eax
	subl	%edx, %eax
	sall	$2, %eax
	addl	%ecx, %eax
	movl	%eax, -112(%ebp)
	movl	-108(%ebp), %eax
	imull	-84(%ebp), %eax
	movl	%eax, %edx
	movl	24(%ebp), %eax
	imull	$-6, %eax, %eax
	movl	%edx, %ecx
	addl	%eax, %ecx
	movl	20(%ebp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	negl	%eax
	addl	%ecx, %eax
	movl	%eax, -116(%ebp)
	movl	-104(%ebp), %eax
	movl	-108(%ebp), %edx
	subl	%eax, %edx
	movl	%edx, %eax
	subl	-100(%ebp), %eax
	movl	%eax, -120(%ebp)
	movl	-8(%ebp), %eax
	movl	-4(%ebp), %edx
	addl	%eax, %edx
	movl	-12(%ebp), %eax
	addl	%eax, %edx
	movl	-16(%ebp), %eax
	addl	%eax, %edx
	movl	-20(%ebp), %eax
	addl	%eax, %edx
	movl	-24(%ebp), %eax
	addl	%eax, %edx
	movl	-28(%ebp), %eax
	addl	%eax, %edx
	movl	-32(%ebp), %eax
	addl	%eax, %edx
	movl	-36(%ebp), %eax
	addl	%eax, %edx
	movl	-40(%ebp), %eax
	addl	%eax, %edx
	movl	-44(%ebp), %eax
	addl	%eax, %edx
	movl	-48(%ebp), %eax
	addl	%eax, %edx
	movl	-52(%ebp), %eax
	addl	%eax, %edx
	movl	-56(%ebp), %eax
	addl	%eax, %edx
	movl	-60(%ebp), %eax
	addl	%eax, %edx
	movl	-64(%ebp), %eax
	addl	%eax, %edx
	movl	-68(%ebp), %eax
	addl	%eax, %edx
	movl	-72(%ebp), %eax
	addl	%eax, %edx
	movl	-76(%ebp), %eax
	addl	%eax, %edx
	movl	-80(%ebp), %eax
	addl	%eax, %edx
	movl	-84(%ebp), %eax
	addl	%eax, %edx
	movl	-88(%ebp), %eax
	addl	%eax, %edx
	movl	-92(%ebp), %eax
	addl	%eax, %edx
	movl	-96(%ebp), %eax
	addl	%eax, %edx
	movl	-100(%ebp), %eax
	addl	%eax, %edx
	movl	-104(%ebp), %eax
	addl	%eax, %edx
	movl	-108(%ebp), %eax
	addl	%eax, %edx
	movl	-112(%ebp), %eax
	addl	%eax, %edx
	movl	-116(%ebp), %eax
	addl	%eax, %edx
	movl	-120(%ebp), %eax
	addl	%edx, %eax
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE234:
	.size	Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit, .-Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",@progbits
