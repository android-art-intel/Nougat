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
.LFB234:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	leal	(%esp), %ebp
	.cfi_def_cfa_register 5
	call	__x86.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
	movl	16(%ebp), %eax
	addl	$1, %eax
	cmpl	$5, %eax
	ja	.L2
	sall	$2, %eax
	movl	.L4@GOTOFF(%eax,%ecx), %eax
	addl	%ecx, %eax
	jmp	*%eax
	.section	.rodata
	.align 4
	.align 4
.L4:
	.long	.L3@GOTOFF
	.long	.L5@GOTOFF
	.long	.L6@GOTOFF
	.long	.L7@GOTOFF
	.long	.L8@GOTOFF
	.long	.L9@GOTOFF
	.text
.L3:
	movl	$10, 20(%ebp)
	jmp	.L10
.L5:
	addl	$1, 20(%ebp)
	jmp	.L10
.L6:
	movl	24(%ebp), %eax
	addl	%eax, 20(%ebp)
	jmp	.L10
.L7:
	movl	24(%ebp), %eax
	subl	%eax, 20(%ebp)
	addl	$5, 24(%ebp)
	jmp	.L10
.L8:
	movl	24(%ebp), %eax
	subl	%eax, 20(%ebp)
	addl	$6, 24(%ebp)
	jmp	.L10
.L9:
	movl	24(%ebp), %eax
	subl	%eax, 20(%ebp)
	addl	$6, 24(%ebp)
	jmp	.L10
.L2:
	movl	24(%ebp), %eax
	subl	%eax, 20(%ebp)
	nop
.L10:
	movl	20(%ebp), %eax
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE234:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch, .-Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch
	.section	.text.__x86.get_pc_thunk.cx,"axG",@progbits,__x86.get_pc_thunk.cx,comdat
	.globl	__x86.get_pc_thunk.cx
	.hidden	__x86.get_pc_thunk.cx
	.type	__x86.get_pc_thunk.cx, @function
__x86.get_pc_thunk.cx:
.LFB235:
	.cfi_startproc
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	movl	(%esp), %ecx
	ret
	.cfi_endproc
.LFE235:
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",@progbits
