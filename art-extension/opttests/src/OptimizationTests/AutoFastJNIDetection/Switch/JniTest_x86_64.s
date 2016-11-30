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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch
	.type	Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch, @function
Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch:
.LFB237:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movl	%edx, -20(%rbp)
	movl	%ecx, -24(%rbp)
	movl	%r8d, -28(%rbp)
	movl	-20(%rbp), %eax
	addl	$1, %eax
	cmpl	$5, %eax
	ja	.L2
	movl	%eax, %eax
	leaq	0(,%rax,4), %rdx
	leaq	.L4(%rip), %rax
	movl	(%rdx,%rax), %eax
	movslq	%eax, %rdx
	leaq	.L4(%rip), %rax
	addq	%rdx, %rax
	jmp	*%rax
	.section	.rodata
	.align 4
	.align 4
.L4:
	.long	.L3-.L4
	.long	.L5-.L4
	.long	.L6-.L4
	.long	.L7-.L4
	.long	.L8-.L4
	.long	.L9-.L4
	.text
.L3:
	movl	$10, -24(%rbp)
	jmp	.L10
.L5:
	addl	$1, -24(%rbp)
	jmp	.L10
.L6:
	movl	-28(%rbp), %eax
	addl	%eax, -24(%rbp)
	jmp	.L10
.L7:
	movl	-28(%rbp), %eax
	subl	%eax, -24(%rbp)
	addl	$5, -28(%rbp)
	jmp	.L10
.L8:
	movl	-28(%rbp), %eax
	subl	%eax, -24(%rbp)
	addl	$6, -28(%rbp)
	jmp	.L10
.L9:
	movl	-28(%rbp), %eax
	subl	%eax, -24(%rbp)
	addl	$6, -28(%rbp)
	jmp	.L10
.L2:
	movl	-28(%rbp), %eax
	subl	%eax, -24(%rbp)
	nop
.L10:
	movl	-24(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch, .-Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch
	.ident	"GCC: (GNU) 4.9 20140514 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
