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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_Loop2_Main_nativeLoop2
	.type	Java_OptimizationTests_AutoFastJNIDetection_Loop2_Main_nativeLoop2, @function
Java_OptimizationTests_AutoFastJNIDetection_Loop2_Main_nativeLoop2:
.LFB238:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
.L3:
	cmpl	$0, 24(%ebp)
	jg	.L2
	movl	20(%ebp), %eax
	movl	16(%ebp), %edx
	addl	%edx, %eax
	popl	%ebp
	.cfi_remember_state
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
.L2:
	.cfi_restore_state
	subl	$1, 24(%ebp)
	addl	$5, 20(%ebp)
	jmp	.L3
	.cfi_endproc
.LFE238:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Loop2_Main_nativeLoop2, .-Java_OptimizationTests_AutoFastJNIDetection_Loop2_Main_nativeLoop2
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
