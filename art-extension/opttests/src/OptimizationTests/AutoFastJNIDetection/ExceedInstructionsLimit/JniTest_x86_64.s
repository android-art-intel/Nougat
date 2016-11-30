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
.LFB237:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	-40(%rsp), %rsp
	movq	%rdi, -136(%rbp)
	movq	%rsi, -144(%rbp)
	movl	%edx, -148(%rbp)
	movl	%ecx, -152(%rbp)
	movl	%r8d, -156(%rbp)
	movl	-148(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	leal	2(%rax), %edx
	movl	-152(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -8(%rbp)
	movl	-8(%rbp), %eax
	leal	3(%rax), %edx
	movl	-156(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -12(%rbp)
	movl	-4(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-152(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -16(%rbp)
	movl	-12(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-148(%rbp), %eax
	imull	-152(%rbp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -20(%rbp)
	movl	-12(%rbp), %eax
	imull	-8(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-148(%rbp), %eax
	imull	-152(%rbp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -24(%rbp)
	movl	-12(%rbp), %eax
	imull	-4(%rbp), %eax
	movl	%eax, %edx
	movl	-152(%rbp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	addl	%eax, %edx
	movl	-148(%rbp), %eax
	imull	-152(%rbp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -28(%rbp)
	movl	-12(%rbp), %eax
	imull	-8(%rbp), %eax
	movl	%eax, %edx
	movl	-156(%rbp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	leal	(%rdx,%rax), %ecx
	movl	-148(%rbp), %edx
	movl	$0, %eax
	subl	%edx, %eax
	sall	$2, %eax
	addl	%ecx, %eax
	movl	%eax, -32(%rbp)
	movl	-28(%rbp), %eax
	imull	-4(%rbp), %eax
	movl	%eax, %ecx
	movl	-148(%rbp), %edx
	movl	%edx, %eax
	addl	%eax, %eax
	addl	%edx, %eax
	addl	%eax, %eax
	negl	%eax
	addl	%eax, %ecx
	movl	-152(%rbp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	negl	%eax
	addl	%ecx, %eax
	movl	%eax, -36(%rbp)
	movl	-28(%rbp), %eax
	subl	-24(%rbp), %eax
	subl	-20(%rbp), %eax
	movl	%eax, -40(%rbp)
	movl	-152(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -44(%rbp)
	movl	-44(%rbp), %eax
	leal	2(%rax), %edx
	movl	-152(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -48(%rbp)
	movl	-48(%rbp), %eax
	leal	3(%rax), %edx
	movl	-156(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -52(%rbp)
	movl	-44(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-152(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -56(%rbp)
	movl	-52(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-152(%rbp), %eax
	imull	-152(%rbp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -60(%rbp)
	movl	-52(%rbp), %eax
	imull	-48(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-152(%rbp), %eax
	imull	-152(%rbp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -64(%rbp)
	movl	-52(%rbp), %eax
	imull	-44(%rbp), %eax
	movl	%eax, %edx
	movl	-152(%rbp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	addl	%eax, %edx
	movl	-152(%rbp), %eax
	imull	-152(%rbp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -68(%rbp)
	movl	-52(%rbp), %eax
	imull	-48(%rbp), %eax
	movl	%eax, %edx
	movl	-156(%rbp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	leal	(%rdx,%rax), %ecx
	movl	-152(%rbp), %edx
	movl	$0, %eax
	subl	%edx, %eax
	sall	$2, %eax
	addl	%ecx, %eax
	movl	%eax, -72(%rbp)
	movl	-68(%rbp), %eax
	imull	-44(%rbp), %eax
	movl	%eax, %ecx
	movl	-152(%rbp), %edx
	movl	%edx, %eax
	addl	%eax, %eax
	addl	%edx, %eax
	addl	%eax, %eax
	negl	%eax
	addl	%eax, %ecx
	movl	-152(%rbp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	negl	%eax
	addl	%ecx, %eax
	movl	%eax, -76(%rbp)
	movl	-68(%rbp), %eax
	subl	-64(%rbp), %eax
	subl	-60(%rbp), %eax
	movl	%eax, -80(%rbp)
	movl	-156(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -84(%rbp)
	movl	-84(%rbp), %eax
	leal	2(%rax), %edx
	movl	-152(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -88(%rbp)
	movl	-88(%rbp), %eax
	leal	3(%rax), %edx
	movl	-156(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -92(%rbp)
	movl	-84(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-152(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -96(%rbp)
	movl	-92(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-156(%rbp), %eax
	imull	-152(%rbp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -100(%rbp)
	movl	-92(%rbp), %eax
	imull	-88(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-156(%rbp), %eax
	imull	-152(%rbp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -104(%rbp)
	movl	-92(%rbp), %eax
	imull	-84(%rbp), %eax
	movl	%eax, %edx
	movl	-152(%rbp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	addl	%eax, %edx
	movl	-156(%rbp), %eax
	imull	-152(%rbp), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -108(%rbp)
	movl	-92(%rbp), %eax
	imull	-88(%rbp), %eax
	movl	%eax, %edx
	movl	-156(%rbp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	leal	(%rdx,%rax), %ecx
	movl	-156(%rbp), %edx
	movl	$0, %eax
	subl	%edx, %eax
	sall	$2, %eax
	addl	%ecx, %eax
	movl	%eax, -112(%rbp)
	movl	-108(%rbp), %eax
	imull	-84(%rbp), %eax
	movl	%eax, %ecx
	movl	-156(%rbp), %edx
	movl	%edx, %eax
	addl	%eax, %eax
	addl	%edx, %eax
	addl	%eax, %eax
	negl	%eax
	addl	%eax, %ecx
	movl	-152(%rbp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	negl	%eax
	addl	%ecx, %eax
	movl	%eax, -116(%rbp)
	movl	-108(%rbp), %eax
	subl	-104(%rbp), %eax
	subl	-100(%rbp), %eax
	movl	%eax, -120(%rbp)
	movl	-4(%rbp), %edx
	movl	-8(%rbp), %eax
	addl	%eax, %edx
	movl	-12(%rbp), %eax
	addl	%eax, %edx
	movl	-16(%rbp), %eax
	addl	%eax, %edx
	movl	-20(%rbp), %eax
	addl	%eax, %edx
	movl	-24(%rbp), %eax
	addl	%eax, %edx
	movl	-28(%rbp), %eax
	addl	%eax, %edx
	movl	-32(%rbp), %eax
	addl	%eax, %edx
	movl	-36(%rbp), %eax
	addl	%eax, %edx
	movl	-40(%rbp), %eax
	addl	%eax, %edx
	movl	-44(%rbp), %eax
	addl	%eax, %edx
	movl	-48(%rbp), %eax
	addl	%eax, %edx
	movl	-52(%rbp), %eax
	addl	%eax, %edx
	movl	-56(%rbp), %eax
	addl	%eax, %edx
	movl	-60(%rbp), %eax
	addl	%eax, %edx
	movl	-64(%rbp), %eax
	addl	%eax, %edx
	movl	-68(%rbp), %eax
	addl	%eax, %edx
	movl	-72(%rbp), %eax
	addl	%eax, %edx
	movl	-76(%rbp), %eax
	addl	%eax, %edx
	movl	-80(%rbp), %eax
	addl	%eax, %edx
	movl	-84(%rbp), %eax
	addl	%eax, %edx
	movl	-88(%rbp), %eax
	addl	%eax, %edx
	movl	-92(%rbp), %eax
	addl	%eax, %edx
	movl	-96(%rbp), %eax
	addl	%eax, %edx
	movl	-100(%rbp), %eax
	addl	%eax, %edx
	movl	-104(%rbp), %eax
	addl	%eax, %edx
	movl	-108(%rbp), %eax
	addl	%eax, %edx
	movl	-112(%rbp), %eax
	addl	%eax, %edx
	movl	-116(%rbp), %eax
	addl	%eax, %edx
	movl	-120(%rbp), %eax
	addl	%edx, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit, .-Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit
	.ident	"GCC: (GNU) 4.9 20140514 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
