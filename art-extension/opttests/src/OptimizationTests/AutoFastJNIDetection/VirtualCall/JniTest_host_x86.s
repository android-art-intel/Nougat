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
.LFB238:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	movl	$1, %eax
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE238:
	.size	_ZN1A8getValueEv, .-_ZN1A8getValueEv
	.section	.text._ZN1B8getValueEv,"axG",@progbits,_ZN1B8getValueEv,comdat
	.align 2
	.weak	_ZN1B8getValueEv
	.hidden	_ZN1B8getValueEv
	.type	_ZN1B8getValueEv, @function
_ZN1B8getValueEv:
.LFB239:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	movl	$100, %eax
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE239:
	.size	_ZN1B8getValueEv, .-_ZN1B8getValueEv
	.section	.text._ZN1AC2Ev,"axG",@progbits,_ZN1AC5Ev,comdat
	.align 2
	.weak	_ZN1AC2Ev
	.hidden	_ZN1AC2Ev
	.type	_ZN1AC2Ev, @function
_ZN1AC2Ev:
.LFB243:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	call	__i686.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
	movl	8(%ebp), %eax
	leal	8+_ZTV1A@GOTOFF(%ecx), %edx
	movl	%edx, (%eax)
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE243:
	.size	_ZN1AC2Ev, .-_ZN1AC2Ev
	.section	.text._ZN1BC2Ev,"axG",@progbits,_ZN1BC5Ev,comdat
	.align 2
	.weak	_ZN1BC2Ev
	.hidden	_ZN1BC2Ev
	.type	_ZN1BC2Ev, @function
_ZN1BC2Ev:
.LFB245:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$20, %esp
	.cfi_offset 3, -12
	call	__i686.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN1AC2Ev
	movl	8(%ebp), %eax
	leal	8+_ZTV1B@GOTOFF(%ebx), %edx
	movl	%edx, (%eax)
	addl	$20, %esp
	popl	%ebx
	.cfi_restore 3
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE245:
	.size	_ZN1BC2Ev, .-_ZN1BC2Ev
	.text
	.globl	Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall
	.type	Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall, @function
Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall:
.LFB240:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	leal	-16(%ebp), %eax
	movl	%eax, (%esp)
	call	_ZN1BC1Ev
	leal	-16(%ebp), %eax
	movl	%eax, -12(%ebp)
	movl	-12(%ebp), %eax
	movl	(%eax), %eax
	movl	(%eax), %edx
	movl	-12(%ebp), %eax
	movl	%eax, (%esp)
	call	*%edx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE240:
	.size	Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall, .-Java_OptimizationTests_AutoFastJNIDetection_VirtualCall_Main_nativeVirtualCall
	.hidden	_ZTV1B
	.weak	_ZTV1B
	.section	.data.rel.ro._ZTV1B,"awG",@progbits,_ZTV1B,comdat
	.align 8
	.type	_ZTV1B, @object
	.size	_ZTV1B, 12
_ZTV1B:
	.long	0
	.long	_ZTI1B
	.long	_ZN1B8getValueEv
	.hidden	_ZTV1A
	.weak	_ZTV1A
	.section	.data.rel.ro._ZTV1A,"awG",@progbits,_ZTV1A,comdat
	.align 8
	.type	_ZTV1A, @object
	.size	_ZTV1A, 12
_ZTV1A:
	.long	0
	.long	_ZTI1A
	.long	_ZN1A8getValueEv
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
	.align 4
	.type	_ZTI1B, @object
	.size	_ZTI1B, 12
_ZTI1B:
	.long	_ZTVN10__cxxabiv120__si_class_type_infoE+8
	.long	_ZTS1B
	.long	_ZTI1A
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
	.align 4
	.type	_ZTI1A, @object
	.size	_ZTI1A, 8
_ZTI1A:
	.long	_ZTVN10__cxxabiv117__class_type_infoE+8
	.long	_ZTS1A
	.weak	_ZN1AC1Ev
	.hidden	_ZN1AC1Ev
	.set	_ZN1AC1Ev,_ZN1AC2Ev
	.weak	_ZN1BC1Ev
	.hidden	_ZN1BC1Ev
	.set	_ZN1BC1Ev,_ZN1BC2Ev
	.section	.text.__i686.get_pc_thunk.cx,"axG",@progbits,__i686.get_pc_thunk.cx,comdat
	.globl	__i686.get_pc_thunk.cx
	.hidden	__i686.get_pc_thunk.cx
	.type	__i686.get_pc_thunk.cx, @function
__i686.get_pc_thunk.cx:
.LFB247:
	.cfi_startproc
	movl	(%esp), %ecx
	ret
	.cfi_endproc
.LFE247:
	.section	.text.__i686.get_pc_thunk.bx,"axG",@progbits,__i686.get_pc_thunk.bx,comdat
	.globl	__i686.get_pc_thunk.bx
	.hidden	__i686.get_pc_thunk.bx
	.type	__i686.get_pc_thunk.bx, @function
__i686.get_pc_thunk.bx:
.LFB248:
	.cfi_startproc
	movl	(%esp), %ebx
	ret
	.cfi_endproc
.LFE248:
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
