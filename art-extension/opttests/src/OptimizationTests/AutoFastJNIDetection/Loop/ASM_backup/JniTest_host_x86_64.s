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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_Loop_Main_nativeLoop
	.type	Java_OptimizationTests_AutoFastJNIDetection_Loop_Main_nativeLoop, @function
Java_OptimizationTests_AutoFastJNIDetection_Loop_Main_nativeLoop:
.LFB237:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	%edx, -36(%rbp)
	movl	%ecx, -40(%rbp)
	movl	%r8d, -44(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L2
.L3:
	movl	-40(%rbp), %eax
	addl	%eax, -36(%rbp)
	addl	$1, -4(%rbp)
.L2:
	movl	-4(%rbp), %eax
	cmpl	-44(%rbp), %eax
	setl	%al
	testb	%al, %al
	jne	.L3
	movl	-40(%rbp), %eax
	movl	-36(%rbp), %edx
	addl	%edx, %eax
	addl	-44(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Loop_Main_nativeLoop, .-Java_OptimizationTests_AutoFastJNIDetection_Loop_Main_nativeLoop
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
