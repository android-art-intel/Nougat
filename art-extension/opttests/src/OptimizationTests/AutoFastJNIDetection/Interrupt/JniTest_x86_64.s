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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_Interrupt_Main_nativeInterrupt
	.type	Java_OptimizationTests_AutoFastJNIDetection_Interrupt_Main_nativeInterrupt, @function
Java_OptimizationTests_AutoFastJNIDetection_Interrupt_Main_nativeInterrupt:
.LFB242:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movl	%edx, -20(%rbp)
	cmpl	$10, -20(%rbp)
	jbe	.L2
#APP
# 9 "/export/users/nrinskay/art_optimization_tests/m-dev/src/OptimizationTests/AutoFastJNIDetection//Interrupt/JniTest.cpp" 1
	INT $3;
# 0 "" 2
#NO_APP
	jmp	.L3
.L2:
	addl	$5, -20(%rbp)
.L3:
	movl	-20(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE242:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Interrupt_Main_nativeInterrupt, .-Java_OptimizationTests_AutoFastJNIDetection_Interrupt_Main_nativeInterrupt
	.ident	"GCC: (GNU) 4.9 20140514 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
