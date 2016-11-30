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
.LFB248:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	cmpl	$10, 16(%ebp)
	jbe	.L2
#APP
# 9 "/export/users/nrinskay/art_optimization_tests/m-dev/src/OptimizationTests/AutoFastJNIDetection//Interrupt/JniTest.cpp" 1
	INT $3;
# 0 "" 2
#NO_APP
	jmp	.L3
.L2:
	addl	$5, 16(%ebp)
.L3:
	movl	16(%ebp), %eax
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE248:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Interrupt_Main_nativeInterrupt, .-Java_OptimizationTests_AutoFastJNIDetection_Interrupt_Main_nativeInterrupt
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
