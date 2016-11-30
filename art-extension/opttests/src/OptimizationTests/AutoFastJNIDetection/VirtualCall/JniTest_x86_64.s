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
	.section	.text._ZN1A8getValueEv,"axG",@progbits,_ZN1A8getValueEv,comdat
	.align 2
	.weak	_ZN1A8getValueEv
	.hidden	_ZN1A8getValueEv
	.type	_ZN1A8getValueEv, @function
_ZN1A8getValueEv:
.LFB237:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movl	$1, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE237:
	.size	_ZN1A8getValueEv, .-_ZN1A8getValueEv
	.section	.text._ZN1B8getValueEv,"axG",@progbits,_ZN1B8getValueEv,comdat
	.align 2
	.weak	_ZN1B8getValueEv
	.hidden	_ZN1B8getValueEv
	.type	_ZN1B8getValueEv, @function
_ZN1B8getValueEv:
.LFB238:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movl	$100, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE238:
	.size	_ZN1B8getValueEv, .-_ZN1B8getValueEv
	.section	.text._ZN1AC2Ev,"axG",@progbits,_ZN1AC5Ev,comdat
	.align 2
	.weak	_ZN1AC2Ev
	.hidden	_ZN1AC2Ev
	.type	_ZN1AC2Ev, @function
_ZN1AC2Ev:
.LFB242:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	leaq	16+_ZTV1A(%rip), %rdx
	movq	%rdx, (%rax)
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE242:
	.size	_ZN1AC2Ev, .-_ZN1AC2Ev
	.weak	_ZN1AC1Ev
	.hidden	_ZN1AC1Ev
	.set	_ZN1AC1Ev,_ZN1AC2Ev
	.section	.text._ZN1BC2Ev,"axG",@progbits,_ZN1BC5Ev,comdat
	.align 2
	.weak	_ZN1BC2Ev
	.hidden	_ZN1BC2Ev
	.type	_ZN1BC2Ev, @function
_ZN1BC2Ev:
.LFB244:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	-16(%rsp), %rsp
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN1AC2Ev
	movq	-8(%rbp), %rax
	leaq	16+_ZTV1B(%rip), %rdx
	movq	%rdx, (%rax)
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE244:
	.size	_ZN1BC2Ev, .-_ZN1BC2Ev
	.weak	_ZN1BC1Ev
	.hidden	_ZN1BC1Ev
	.set	_ZN1BC1Ev,_ZN1BC2Ev
	.text
	.globl	Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall
	.type	Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall, @function
Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall:
.LFB239:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	-48(%rsp), %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	%edx, -36(%rbp)
	movl	%ecx, -40(%rbp)
	movl	%r8d, -44(%rbp)
	leaq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN1BC1Ev
	leaq	-16(%rbp), %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	(%rax), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, %rdi
	call	*%rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE239:
	.size	Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall, .-Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall
	.hidden	_ZTV1B
	.weak	_ZTV1B
	.section	.data.rel.ro.local._ZTV1B,"awG",@progbits,_ZTV1B,comdat
	.align 16
	.type	_ZTV1B, @object
	.size	_ZTV1B, 24
_ZTV1B:
	.quad	0
	.quad	_ZTI1B
	.quad	_ZN1B8getValueEv
	.hidden	_ZTV1A
	.weak	_ZTV1A
	.section	.data.rel.ro.local._ZTV1A,"awG",@progbits,_ZTV1A,comdat
	.align 16
	.type	_ZTV1A, @object
	.size	_ZTV1A, 24
_ZTV1A:
	.quad	0
	.quad	_ZTI1A
	.quad	_ZN1A8getValueEv
	.hidden	_ZTS1B
	.weak	_ZTS1B
	.section	.rodata._ZTS1B,"aG",@progbits,_ZTS1B,comdat
	.type	_ZTS1B, @object
	.size	_ZTS1B, 3
_ZTS1B:
	.string	"1B"
	.hidden	_ZTI1B
	.weak	_ZTI1B
	.section	.data.rel.ro._ZTI1B,"awG",@progbits,_ZTI1B,comdat
	.align 16
	.type	_ZTI1B, @object
	.size	_ZTI1B, 24
_ZTI1B:
	.quad	_ZTVN10__cxxabiv120__si_class_type_infoE+16
	.quad	_ZTS1B
	.quad	_ZTI1A
	.hidden	_ZTS1A
	.weak	_ZTS1A
	.section	.rodata._ZTS1A,"aG",@progbits,_ZTS1A,comdat
	.type	_ZTS1A, @object
	.size	_ZTS1A, 3
_ZTS1A:
	.string	"1A"
	.hidden	_ZTI1A
	.weak	_ZTI1A
	.section	.data.rel.ro._ZTI1A,"awG",@progbits,_ZTI1A,comdat
	.align 16
	.type	_ZTI1A, @object
	.size	_ZTI1A, 16
_ZTI1A:
	.quad	_ZTVN10__cxxabiv117__class_type_infoE+16
	.quad	_ZTS1A
	.ident	"GCC: (GNU) 4.9 20140514 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
