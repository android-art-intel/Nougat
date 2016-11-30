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
	.local	_ZL10global_var
	.comm	_ZL10global_var,4,4
	.text
	.globl	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal
	.type	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal, @function
Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal:
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
	movl	%eax, _ZL10global_var@GOTOFF(%ecx)
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE234:
	.size	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal, .-Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeSetGlobal
	.globl	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal
	.type	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal, @function
Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal:
.LFB235:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	call	__x86.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
	movl	_ZL10global_var@GOTOFF(%ecx), %eax
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE235:
	.size	Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal, .-Java_OptimizationTests_AutoFastJNIDetection_PCRelativeAddressing_Main_nativeGetGlobal
	.section	.text.__x86.get_pc_thunk.cx,"axG",@progbits,__x86.get_pc_thunk.cx,comdat
	.globl	__x86.get_pc_thunk.cx
	.hidden	__x86.get_pc_thunk.cx
	.type	__x86.get_pc_thunk.cx, @function
__x86.get_pc_thunk.cx:
.LFB236:
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
.LFE236:
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",@progbits
