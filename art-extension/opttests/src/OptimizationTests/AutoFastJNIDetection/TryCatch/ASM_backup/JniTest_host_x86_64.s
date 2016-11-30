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
.LFB237:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA237
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$56, %rsp
	movq	%rdi, -40(%rbp)
	movq	%rsi, -48(%rbp)
	movl	%edx, -52(%rbp)
	movl	%ecx, -56(%rbp)
	movl	%r8d, -60(%rbp)
	movl	$-111, -24(%rbp)
	movl	$4, %edi
	.cfi_offset 3, -24
	call	__cxa_allocate_exception@PLT
	movl	-56(%rbp), %edx
	movl	-52(%rbp), %ecx
	addl	%ecx, %edx
	addl	-60(%rbp), %edx
	movl	%edx, (%rax)
	movl	$0, %edx
	movq	_ZTIi@GOTPCREL(%rip), %rcx
	movq	%rcx, %rsi
	movq	%rax, %rdi
.LEHB0:
	call	__cxa_throw@PLT
.LEHE0:
.L5:
	cmpq	$1, %rdx
	je	.L4
	movq	%rax, %rdi
.LEHB1:
	call	_Unwind_Resume@PLT
.LEHE1:
.L4:
	movq	%rax, %rdi
	call	__cxa_begin_catch@PLT
	movl	(%rax), %eax
	movl	%eax, -20(%rbp)
	movl	-24(%rbp), %eax
	movl	-20(%rbp), %edx
	leal	(%rdx,%rax), %ebx
	call	__cxa_end_catch@PLT
	movl	%ebx, %eax
	addq	$56, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE237:
	.globl	__gxx_personality_v0
	.section	.gcc_except_table,"a",@progbits
	.align 4
.LLSDA237:
	.byte	0xff
	.byte	0x9b
	.uleb128 .LLSDATT237-.LLSDATTD237
.LLSDATTD237:
	.byte	0x1
	.uleb128 .LLSDACSE237-.LLSDACSB237
.LLSDACSB237:
	.uleb128 .LEHB0-.LFB237
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L5-.LFB237
	.uleb128 0x1
	.uleb128 .LEHB1-.LFB237
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0
	.uleb128 0
.LLSDACSE237:
	.byte	0x1
	.byte	0
	.align 4
	.long	DW.ref._ZTIi-.
.LLSDATT237:
	.text
	.size	Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch, .-Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch
	.hidden DW.ref._ZTIi
	.weak	DW.ref._ZTIi
	.section	.data.DW.ref._ZTIi,"awG",@progbits,DW.ref._ZTIi,comdat
	.align 8
	.type	DW.ref._ZTIi, @object
	.size	DW.ref._ZTIi, 8
DW.ref._ZTIi:
	.quad	_ZTIi
	.hidden DW.ref.__gxx_personality_v0
	.weak	DW.ref.__gxx_personality_v0
	.section	.data.DW.ref.__gxx_personality_v0,"awG",@progbits,DW.ref.__gxx_personality_v0,comdat
	.align 8
	.type	DW.ref.__gxx_personality_v0, @object
	.size	DW.ref.__gxx_personality_v0, 8
DW.ref.__gxx_personality_v0:
	.quad	__gxx_personality_v0
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
