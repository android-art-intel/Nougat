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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_TryCatchJava03_Main_nativeTryCatchJava03
	.type	Java_OptimizationTests_AutoFastJNIDetection_TryCatchJava03_Main_nativeTryCatchJava03, @function
Java_OptimizationTests_AutoFastJNIDetection_TryCatchJava03_Main_nativeTryCatchJava03:
.LFB234:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	leal	(%esp), %ebp
	.cfi_def_cfa_register 5
	movl	20(%ebp), %eax
	movl	16(%ebp), %edx
	addl	%eax, %edx
	movl	24(%ebp), %eax
	addl	%edx, %eax
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE234:
	.size	Java_OptimizationTests_AutoFastJNIDetection_TryCatchJava03_Main_nativeTryCatchJava03, .-Java_OptimizationTests_AutoFastJNIDetection_TryCatchJava03_Main_nativeTryCatchJava03
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",@progbits
