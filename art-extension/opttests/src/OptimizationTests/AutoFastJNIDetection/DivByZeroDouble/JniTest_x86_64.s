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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble
	.type	Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble, @function
Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble:
.LFB237:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movsd	%xmm0, -40(%rbp)
	movss	%xmm1, -44(%rbp)
	movl	%edx, -48(%rbp)
	cvtss2sd	-44(%rbp), %xmm4
	movq	%xmm4, %rax
	movq	%rax, %xmm2
	addsd	-40(%rbp), %xmm2
	movq	%xmm2, %rax
	movl	-48(%rbp), %edx
	subl	$5, %edx
	xorpd	%xmm0, %xmm0
	cvtsi2sd	%edx, %xmm0
	movq	%rax, %xmm3
	divsd	%xmm0, %xmm3
	movq	%xmm3, %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %xmm0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble, .-Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble
	.ident	"GCC: (GNU) 4.9 20140514 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
