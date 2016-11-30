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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3
	.type	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3, @function
Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3:
.LFB238:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	movl	20(%ebp), %eax
	movl	16(%ebp), %edx
	addl	%eax, %edx
	movl	24(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	%edx, (%esp)
	call	foo1
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE238:
	.size	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3, .-Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3
	.globl	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4
	.type	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4, @function
Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4:
.LFB239:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	movl	20(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	16(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	24(%ebp), %eax
	movl	%eax, (%esp)
	call	foo2
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE239:
	.size	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4, .-Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4
	.globl	foo
	.hidden	foo
	.type	foo, @function
foo:
.LFB240:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	movl	8(%ebp), %eax
	addl	$5, %eax
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE240:
	.size	foo, .-foo
	.globl	foo1
	.hidden	foo1
	.type	foo1, @function
foo1:
.LFB241:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%esi
	subl	$4, %esp
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	.cfi_offset 6, -12
	call	foo
	movl	%eax, %esi
	movl	12(%ebp), %eax
	movl	%eax, (%esp)
	call	foo
	addl	%esi, %eax
	addl	$4, %esp
	popl	%esi
	.cfi_restore 6
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE241:
	.size	foo1, .-foo1
	.globl	foo2
	.hidden	foo2
	.type	foo2, @function
foo2:
.LFB242:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%esi
	subl	$8, %esp
	movl	12(%ebp), %eax
	leal	(%eax,%eax), %edx
	movl	16(%ebp), %eax
	movl	8(%ebp), %ecx
	addl	%ecx, %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	.cfi_offset 6, -12
	call	foo1
	movl	%eax, %esi
	movl	16(%ebp), %eax
	movl	12(%ebp), %edx
	subl	%eax, %edx
	movl	8(%ebp), %eax
	imull	16(%ebp), %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	foo1
	movl	%esi, %edx
	subl	%eax, %edx
	movl	%edx, %eax
	addl	$8, %esp
	popl	%esi
	.cfi_restore 6
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE242:
	.size	foo2, .-foo2
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
