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
	subq	$40, %rsp
	movq	%rdi, -136(%rbp)
	movq	%rsi, -144(%rbp)
	movl	%edx, -148(%rbp)
	movl	%ecx, -152(%rbp)
	movl	%r8d, -156(%rbp)
	movl	-148(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -120(%rbp)
	movl	-120(%rbp), %eax
	addl	$2, %eax
	addl	-152(%rbp), %eax
	movl	%eax, -116(%rbp)
	movl	-116(%rbp), %eax
	addl	$3, %eax
	addl	-156(%rbp), %eax
	movl	%eax, -112(%rbp)
	movl	-120(%rbp), %eax
	subl	$4, %eax
	addl	-152(%rbp), %eax
	movl	%eax, -108(%rbp)
	movl	-112(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-148(%rbp), %eax
	imull	-152(%rbp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -104(%rbp)
	movl	-112(%rbp), %eax
	imull	-116(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-148(%rbp), %eax
	imull	-152(%rbp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -100(%rbp)
	movl	-112(%rbp), %eax
	movl	%eax, %edx
	imull	-120(%rbp), %edx
	movl	-152(%rbp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	addl	%eax, %edx
	movl	-148(%rbp), %eax
	imull	-152(%rbp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -96(%rbp)
	movl	-112(%rbp), %eax
	movl	%eax, %edx
	imull	-116(%rbp), %edx
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
	movl	%eax, -92(%rbp)
	movl	-96(%rbp), %eax
	movl	%eax, %ecx
	imull	-120(%rbp), %ecx
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
	movl	%eax, -88(%rbp)
	movl	-100(%rbp), %eax
	movl	-96(%rbp), %edx
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	subl	-104(%rbp), %eax
	movl	%eax, -84(%rbp)
	movl	-152(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -80(%rbp)
	movl	-80(%rbp), %eax
	addl	$2, %eax
	addl	-152(%rbp), %eax
	movl	%eax, -76(%rbp)
	movl	-76(%rbp), %eax
	addl	$3, %eax
	addl	-156(%rbp), %eax
	movl	%eax, -72(%rbp)
	movl	-80(%rbp), %eax
	subl	$4, %eax
	addl	-152(%rbp), %eax
	movl	%eax, -68(%rbp)
	movl	-72(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-152(%rbp), %eax
	imull	-152(%rbp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -64(%rbp)
	movl	-72(%rbp), %eax
	imull	-76(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-152(%rbp), %eax
	imull	-152(%rbp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -60(%rbp)
	movl	-72(%rbp), %eax
	movl	%eax, %edx
	imull	-80(%rbp), %edx
	movl	-152(%rbp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	addl	%eax, %edx
	movl	-152(%rbp), %eax
	imull	-152(%rbp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -56(%rbp)
	movl	-72(%rbp), %eax
	movl	%eax, %edx
	imull	-76(%rbp), %edx
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
	movl	%eax, -52(%rbp)
	movl	-56(%rbp), %eax
	movl	%eax, %ecx
	imull	-80(%rbp), %ecx
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
	movl	%eax, -48(%rbp)
	movl	-60(%rbp), %eax
	movl	-56(%rbp), %edx
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	subl	-64(%rbp), %eax
	movl	%eax, -44(%rbp)
	movl	-156(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -40(%rbp)
	movl	-40(%rbp), %eax
	addl	$2, %eax
	addl	-152(%rbp), %eax
	movl	%eax, -36(%rbp)
	movl	-36(%rbp), %eax
	addl	$3, %eax
	addl	-156(%rbp), %eax
	movl	%eax, -32(%rbp)
	movl	-40(%rbp), %eax
	subl	$4, %eax
	addl	-152(%rbp), %eax
	movl	%eax, -28(%rbp)
	movl	-32(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-156(%rbp), %eax
	imull	-152(%rbp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -24(%rbp)
	movl	-32(%rbp), %eax
	imull	-36(%rbp), %eax
	leal	-4(%rax), %edx
	movl	-156(%rbp), %eax
	imull	-152(%rbp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -20(%rbp)
	movl	-32(%rbp), %eax
	movl	%eax, %edx
	imull	-40(%rbp), %edx
	movl	-152(%rbp), %ecx
	movl	$0, %eax
	subl	%ecx, %eax
	sall	$2, %eax
	addl	%eax, %edx
	movl	-156(%rbp), %eax
	imull	-152(%rbp), %eax
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	movl	%eax, -16(%rbp)
	movl	-32(%rbp), %eax
	movl	%eax, %edx
	imull	-36(%rbp), %edx
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
	movl	%eax, -12(%rbp)
	movl	-16(%rbp), %eax
	movl	%eax, %ecx
	imull	-40(%rbp), %ecx
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
	movl	%eax, -8(%rbp)
	movl	-20(%rbp), %eax
	movl	-16(%rbp), %edx
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	subl	-24(%rbp), %eax
	movl	%eax, -4(%rbp)
	movl	-116(%rbp), %eax
	movl	-120(%rbp), %edx
	addl	%edx, %eax
	addl	-112(%rbp), %eax
	addl	-108(%rbp), %eax
	addl	-104(%rbp), %eax
	addl	-100(%rbp), %eax
	addl	-96(%rbp), %eax
	addl	-92(%rbp), %eax
	addl	-88(%rbp), %eax
	addl	-84(%rbp), %eax
	addl	-80(%rbp), %eax
	addl	-76(%rbp), %eax
	addl	-72(%rbp), %eax
	addl	-68(%rbp), %eax
	addl	-64(%rbp), %eax
	addl	-60(%rbp), %eax
	addl	-56(%rbp), %eax
	addl	-52(%rbp), %eax
	addl	-48(%rbp), %eax
	addl	-44(%rbp), %eax
	addl	-40(%rbp), %eax
	addl	-36(%rbp), %eax
	addl	-32(%rbp), %eax
	addl	-28(%rbp), %eax
	addl	-24(%rbp), %eax
	addl	-20(%rbp), %eax
	addl	-16(%rbp), %eax
	addl	-12(%rbp), %eax
	addl	-8(%rbp), %eax
	addl	-4(%rbp), %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit, .-Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
