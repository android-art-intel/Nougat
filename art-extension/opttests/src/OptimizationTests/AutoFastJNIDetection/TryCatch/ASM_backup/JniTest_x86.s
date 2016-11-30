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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch
	.type	Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch, @function
Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch:
.LFB234:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA234
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%esi
	pushl	%ebx
	leal	-32(%esp), %esp
	.cfi_offset 6, -12
	.cfi_offset 3, -16
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	$-111, -12(%ebp)
	movl	$4, (%esp)
	call	__cxa_allocate_exception@PLT
	movl	20(%ebp), %edx
	movl	16(%ebp), %ecx
	addl	%edx, %ecx
	movl	24(%ebp), %edx
	addl	%ecx, %edx
	movl	%edx, (%eax)
	movl	$0, 8(%esp)
	movl	_ZTIi@GOT(%ebx), %edx
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
.LEHB0:
	call	__cxa_throw@PLT
.LEHE0:
.L6:
	cmpl	$1, %edx
	je	.L5
	movl	%eax, (%esp)
.LEHB1:
	call	_Unwind_Resume@PLT
.LEHE1:
.L5:
	movl	%eax, (%esp)
	call	__cxa_begin_catch@PLT
	movl	(%eax), %eax
	movl	%eax, -16(%ebp)
	movl	-12(%ebp), %eax
	movl	-16(%ebp), %edx
	movl	%edx, %esi
	addl	%eax, %esi
	call	__cxa_end_catch@PLT
	movl	%esi, %eax
	leal	32(%esp), %esp
	popl	%ebx
	.cfi_restore 3
	popl	%esi
	.cfi_restore 6
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE234:
	.globl	__gxx_personality_v0
	.section	.gcc_except_table,"a",@progbits
	.align 4
.LLSDA234:
	.byte	0xff
	.byte	0x9b
	.uleb128 .LLSDATT234-.LLSDATTD234
.LLSDATTD234:
	.byte	0x1
	.uleb128 .LLSDACSE234-.LLSDACSB234
.LLSDACSB234:
	.uleb128 .LEHB0-.LFB234
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L6-.LFB234
	.uleb128 0x1
	.uleb128 .LEHB1-.LFB234
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0
	.uleb128 0
.LLSDACSE234:
	.byte	0x1
	.byte	0
	.align 4
	.long	DW.ref._ZTIi-.
.LLSDATT234:
	.text
	.size	Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch, .-Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch
	.section	.text.__x86.get_pc_thunk.bx,"axG",@progbits,__x86.get_pc_thunk.bx,comdat
	.globl	__x86.get_pc_thunk.bx
	.hidden	__x86.get_pc_thunk.bx
	.type	__x86.get_pc_thunk.bx, @function
__x86.get_pc_thunk.bx:
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
	movl	(%esp), %ebx
	ret
	.cfi_endproc
.LFE235:
	.hidden	DW.ref._ZTIi
	.weak	DW.ref._ZTIi
	.section	.data.DW.ref._ZTIi,"awG",@progbits,DW.ref._ZTIi,comdat
	.align 4
	.type	DW.ref._ZTIi, @object
	.size	DW.ref._ZTIi, 4
DW.ref._ZTIi:
	.long	_ZTIi
	.hidden	DW.ref.__gxx_personality_v0
	.weak	DW.ref.__gxx_personality_v0
	.section	.data.DW.ref.__gxx_personality_v0,"awG",@progbits,DW.ref.__gxx_personality_v0,comdat
	.align 4
	.type	DW.ref.__gxx_personality_v0, @object
	.size	DW.ref.__gxx_personality_v0, 4
DW.ref.__gxx_personality_v0:
	.long	__gxx_personality_v0
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",@progbits
