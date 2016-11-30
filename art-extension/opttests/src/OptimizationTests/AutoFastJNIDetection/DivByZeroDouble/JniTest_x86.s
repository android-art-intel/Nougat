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
.LFB234:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	leal	(%esp), %ebp
	.cfi_def_cfa_register 5
	leal	-32(%esp), %esp
	movl	16(%ebp), %eax
	movl	%eax, -24(%ebp)
	movl	20(%ebp), %eax
	movl	%eax, -20(%ebp)
	cvtss2sd	24(%ebp), %xmm0
	addsd	-24(%ebp), %xmm0
	movl	28(%ebp), %eax
	subl	$5, %eax
	cvtsi2sd	%eax, %xmm1
	divsd	%xmm1, %xmm0
	movsd	%xmm0, -8(%ebp)
	movsd	-8(%ebp), %xmm0
	movsd	%xmm0, -32(%ebp)
	fldl	-32(%ebp)
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE234:
	.size	Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble, .-Java_OptimizationTests_AutoFastJNIDetection_DivByZeroDouble_Main_nativeDivByZeroDouble
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",@progbits
