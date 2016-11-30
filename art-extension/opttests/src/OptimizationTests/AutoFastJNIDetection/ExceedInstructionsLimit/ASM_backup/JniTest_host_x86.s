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
.LFB238:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	addl	$-128, %esp
	movl	16(%ebp), %eax
	addl	$1, %eax
	movl	%eax, -120(%ebp)
	movl	-120(%ebp), %eax
	addl	$2, %eax
	addl	20(%ebp), %eax
	movl	%eax, -116(%ebp)
	movl	-116(%ebp), %eax
	addl	$3, %eax
	addl	24(%ebp), %eax
	movl	%eax, -112(%ebp)
	movl	-120(%ebp), %eax
	subl	$4, %eax
	addl	20(%ebp), %eax
	movl	%eax, -108(%ebp)
	movl	-112(%ebp), %eax
	leal	-4(%eax), %edx
	movl	16(%ebp), %eax
	imull	20(%ebp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -104(%ebp)
	movl	-112(%ebp), %eax
	imull	-116(%ebp), %eax
	leal	-4(%eax), %edx
	movl	16(%ebp), %eax
	imull	20(%ebp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -100(%ebp)
	movl	-112(%ebp), %eax
	movl	%eax, %edx
	imull	-120(%ebp), %edx
	movl	20(%ebp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	addl	%eax, %edx
	movl	16(%ebp), %eax
	imull	20(%ebp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -96(%ebp)
	movl	-112(%ebp), %eax
	movl	%eax, %edx
	imull	-116(%ebp), %edx
	movl	24(%ebp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	leal	(%edx,%eax), %ecx
	movl	16(%ebp), %edx
	movl	$0, %eax
	subl	%edx, %eax
	sall	$2, %eax
	addl	%ecx, %eax
	movl	%eax, -92(%ebp)
	movl	-96(%ebp), %eax
	movl	%eax, %edx
	imull	-120(%ebp), %edx
	movl	16(%ebp), %eax
	imull	$-6, %eax, %eax
	leal	(%edx,%eax), %ecx
	movl	20(%ebp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	negl	%eax
	addl	%ecx, %eax
	movl	%eax, -88(%ebp)
	movl	-100(%ebp), %eax
	movl	-96(%ebp), %edx
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	subl	-104(%ebp), %eax
	movl	%eax, -84(%ebp)
	movl	20(%ebp), %eax
	addl	$1, %eax
	movl	%eax, -80(%ebp)
	movl	-80(%ebp), %eax
	addl	$2, %eax
	addl	20(%ebp), %eax
	movl	%eax, -76(%ebp)
	movl	-76(%ebp), %eax
	addl	$3, %eax
	addl	24(%ebp), %eax
	movl	%eax, -72(%ebp)
	movl	-80(%ebp), %eax
	subl	$4, %eax
	addl	20(%ebp), %eax
	movl	%eax, -68(%ebp)
	movl	-72(%ebp), %eax
	leal	-4(%eax), %edx
	movl	20(%ebp), %eax
	imull	20(%ebp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -64(%ebp)
	movl	-72(%ebp), %eax
	imull	-76(%ebp), %eax
	leal	-4(%eax), %edx
	movl	20(%ebp), %eax
	imull	20(%ebp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -60(%ebp)
	movl	-72(%ebp), %eax
	movl	%eax, %edx
	imull	-80(%ebp), %edx
	movl	20(%ebp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	addl	%eax, %edx
	movl	20(%ebp), %eax
	imull	20(%ebp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -56(%ebp)
	movl	-72(%ebp), %eax
	movl	%eax, %edx
	imull	-76(%ebp), %edx
	movl	24(%ebp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	leal	(%edx,%eax), %ecx
	movl	20(%ebp), %edx
	movl	$0, %eax
	subl	%edx, %eax
	sall	$2, %eax
	addl	%ecx, %eax
	movl	%eax, -52(%ebp)
	movl	-56(%ebp), %eax
	movl	%eax, %edx
	imull	-80(%ebp), %edx
	movl	20(%ebp), %eax
	imull	$-6, %eax, %eax
	leal	(%edx,%eax), %ecx
	movl	20(%ebp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	negl	%eax
	addl	%ecx, %eax
	movl	%eax, -48(%ebp)
	movl	-60(%ebp), %eax
	movl	-56(%ebp), %edx
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	subl	-64(%ebp), %eax
	movl	%eax, -44(%ebp)
	movl	24(%ebp), %eax
	addl	$1, %eax
	movl	%eax, -40(%ebp)
	movl	-40(%ebp), %eax
	addl	$2, %eax
	addl	20(%ebp), %eax
	movl	%eax, -36(%ebp)
	movl	-36(%ebp), %eax
	addl	$3, %eax
	addl	24(%ebp), %eax
	movl	%eax, -32(%ebp)
	movl	-40(%ebp), %eax
	subl	$4, %eax
	addl	20(%ebp), %eax
	movl	%eax, -28(%ebp)
	movl	-32(%ebp), %eax
	leal	-4(%eax), %edx
	movl	24(%ebp), %eax
	imull	20(%ebp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -24(%ebp)
	movl	-32(%ebp), %eax
	imull	-36(%ebp), %eax
	leal	-4(%eax), %edx
	movl	24(%ebp), %eax
	imull	20(%ebp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -20(%ebp)
	movl	-32(%ebp), %eax
	movl	%eax, %edx
	imull	-40(%ebp), %edx
	movl	20(%ebp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	addl	%eax, %edx
	movl	24(%ebp), %eax
	imull	20(%ebp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -16(%ebp)
	movl	-32(%ebp), %eax
	movl	%eax, %edx
	imull	-36(%ebp), %edx
	movl	24(%ebp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	leal	(%edx,%eax), %ecx
	movl	24(%ebp), %edx
	movl	$0, %eax
	subl	%edx, %eax
	sall	$2, %eax
	addl	%ecx, %eax
	movl	%eax, -12(%ebp)
	movl	-16(%ebp), %eax
	movl	%eax, %edx
	imull	-40(%ebp), %edx
	movl	24(%ebp), %eax
	imull	$-6, %eax, %eax
	leal	(%edx,%eax), %ecx
	movl	20(%ebp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	negl	%eax
	addl	%ecx, %eax
	movl	%eax, -8(%ebp)
	movl	-20(%ebp), %eax
	movl	-16(%ebp), %edx
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	subl	-24(%ebp), %eax
	movl	%eax, -4(%ebp)
	movl	-116(%ebp), %eax
	movl	-120(%ebp), %edx
	addl	%edx, %eax
	addl	-112(%ebp), %eax
	addl	-108(%ebp), %eax
	addl	-104(%ebp), %eax
	addl	-100(%ebp), %eax
	addl	-96(%ebp), %eax
	addl	-92(%ebp), %eax
	addl	-88(%ebp), %eax
	addl	-84(%ebp), %eax
	addl	-80(%ebp), %eax
	addl	-76(%ebp), %eax
	addl	-72(%ebp), %eax
	addl	-68(%ebp), %eax
	addl	-64(%ebp), %eax
	addl	-60(%ebp), %eax
	addl	-56(%ebp), %eax
	addl	-52(%ebp), %eax
	addl	-48(%ebp), %eax
	addl	-44(%ebp), %eax
	addl	-40(%ebp), %eax
	addl	-36(%ebp), %eax
	addl	-32(%ebp), %eax
	addl	-28(%ebp), %eax
	addl	-24(%ebp), %eax
	addl	-20(%ebp), %eax
	addl	-16(%ebp), %eax
	addl	-12(%ebp), %eax
	addl	-8(%ebp), %eax
	addl	-4(%ebp), %eax
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE238:
	.size	Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit, .-Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
