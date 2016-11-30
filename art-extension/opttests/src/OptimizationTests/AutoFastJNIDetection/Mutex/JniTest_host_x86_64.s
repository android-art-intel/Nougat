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
	.hidden	count_mutex
	.globl	count_mutex
	.bss
	.align 32
	.type	count_mutex, @object
	.size	count_mutex, 40
count_mutex:
	.zero	40
	.hidden	count
	.globl	count
	.data
	.align 4
	.type	count, @object
	.size	count, 4
count:
	.long	5
	.text
	.globl	Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex
	.type	Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex, @function
Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex:
.LFB237:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movl	%edx, -20(%rbp)
	leaq	count_mutex(%rip), %rdi
	call	pthread_mutex_lock@PLT
	movl	count(%rip), %eax
	addl	$1, %eax
	movl	%eax, count(%rip)
	leaq	count_mutex(%rip), %rdi
	call	pthread_mutex_unlock@PLT
	movl	count(%rip), %eax
	addl	-20(%rbp), %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex, .-Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
