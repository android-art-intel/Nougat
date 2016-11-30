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
.LFB238:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	call	__i686.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
	movl	16(%ebp), %eax
	addl	$1, %eax
	cmpl	$5, %eax
	ja	.L2
	sall	$2, %eax
	movl	.L9@GOTOFF(%eax,%ecx), %eax
	movl	%ecx, %edx
	subl	%eax, %edx
	movl	%edx, %eax
	jmp	*%eax
	.p2align 2
	.align 4
.L9:
	.long	_GLOBAL_OFFSET_TABLE_+[.-.L3]
	.long	_GLOBAL_OFFSET_TABLE_+[.-.L4]
	.long	_GLOBAL_OFFSET_TABLE_+[.-.L5]
	.long	_GLOBAL_OFFSET_TABLE_+[.-.L6]
	.long	_GLOBAL_OFFSET_TABLE_+[.-.L7]
	.long	_GLOBAL_OFFSET_TABLE_+[.-.L8]
.L3:
	movl	$10, 20(%ebp)
	jmp	.L10
.L4:
	addl	$1, 20(%ebp)
	jmp	.L10
.L5:
	movl	24(%ebp), %eax
	addl	%eax, 20(%ebp)
	jmp	.L10
.L6:
	movl	24(%ebp), %eax
	subl	%eax, 20(%ebp)
	addl	$5, 24(%ebp)
	jmp	.L10
.L7:
	movl	24(%ebp), %eax
	subl	%eax, 20(%ebp)
	addl	$6, 24(%ebp)
	jmp	.L10
.L8:
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
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE238:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch, .-Java_OptimizationTests_AutoFastJNIDetection_Switch_Main_nativeSwitch
	.section	.text.__i686.get_pc_thunk.cx,"axG",@progbits,__i686.get_pc_thunk.cx,comdat
	.globl	__i686.get_pc_thunk.cx
	.hidden	__i686.get_pc_thunk.cx
	.type	__i686.get_pc_thunk.cx, @function
__i686.get_pc_thunk.cx:
.LFB239:
	.cfi_startproc
	movl	(%esp), %ecx
	ret
	.cfi_endproc
.LFE239:
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
