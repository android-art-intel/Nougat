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
.LFB238:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA238
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%esi
	pushl	%ebx
	subl	$32, %esp
	.cfi_offset 3, -16
	.cfi_offset 6, -12
	call	__i686.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	$-111, -16(%ebp)
	movl	$4, (%esp)
	call	__cxa_allocate_exception@PLT
	movl	20(%ebp), %edx
	movl	16(%ebp), %ecx
	addl	%ecx, %edx
	addl	24(%ebp), %edx
	movl	%edx, (%eax)
	movl	$0, 8(%esp)
	movl	_ZTIi@GOT(%ebx), %edx
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
.LEHB0:
	call	__cxa_throw@PLT
.LEHE0:
.L5:
	cmpl	$1, %edx
	je	.L4
	movl	%eax, (%esp)
.LEHB1:
	call	_Unwind_Resume@PLT
.LEHE1:
.L4:
	movl	%eax, (%esp)
	call	__cxa_begin_catch@PLT
	movl	(%eax), %eax
	movl	%eax, -12(%ebp)
	movl	-16(%ebp), %eax
	movl	-12(%ebp), %edx
	leal	(%edx,%eax), %esi
	call	__cxa_end_catch@PLT
	movl	%esi, %eax
	addl	$32, %esp
	popl	%ebx
	.cfi_restore 3
	popl	%esi
	.cfi_restore 6
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE238:
	.globl	__gxx_personality_v0
	.section	.gcc_except_table,"a",@progbits
	.align 4
.LLSDA238:
	.byte	0xff
	.byte	0x9b
	.uleb128 .LLSDATT238-.LLSDATTD238
.LLSDATTD238:
	.byte	0x1
	.uleb128 .LLSDACSE238-.LLSDACSB238
.LLSDACSB238:
	.uleb128 .LEHB0-.LFB238
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L5-.LFB238
	.uleb128 0x1
	.uleb128 .LEHB1-.LFB238
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0
	.uleb128 0
.LLSDACSE238:
	.byte	0x1
	.byte	0
	.align 4
	.long	DW.ref._ZTIi-.
.LLSDATT238:
	.text
	.size	Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch, .-Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch
	.section	.text.__i686.get_pc_thunk.bx,"axG",@progbits,__i686.get_pc_thunk.bx,comdat
	.globl	__i686.get_pc_thunk.bx
	.hidden	__i686.get_pc_thunk.bx
	.type	__i686.get_pc_thunk.bx, @function
__i686.get_pc_thunk.bx:
.LFB239:
	.cfi_startproc
	movl	(%esp), %ebx
	ret
	.cfi_endproc
.LFE239:
	.hidden DW.ref._ZTIi
	.weak	DW.ref._ZTIi
	.section	.data.DW.ref._ZTIi,"awG",@progbits,DW.ref._ZTIi,comdat
	.align 4
	.type	DW.ref._ZTIi, @object
	.size	DW.ref._ZTIi, 4
DW.ref._ZTIi:
	.long	_ZTIi
	.hidden DW.ref.__gxx_personality_v0
	.weak	DW.ref.__gxx_personality_v0
	.section	.data.DW.ref.__gxx_personality_v0,"awG",@progbits,DW.ref.__gxx_personality_v0,comdat
	.align 4
	.type	DW.ref.__gxx_personality_v0, @object
	.size	DW.ref.__gxx_personality_v0, 4
DW.ref.__gxx_personality_v0:
	.long	__gxx_personality_v0
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
