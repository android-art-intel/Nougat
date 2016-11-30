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
	.globl	Java_OptimizationTests_AutoFastJNIDetection_Nop_Main_nativeNop
	.type	Java_OptimizationTests_AutoFastJNIDetection_Nop_Main_nativeNop, @function
Java_OptimizationTests_AutoFastJNIDetection_Nop_Main_nativeNop:
.LFB234:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	leal	(%esp), %ebp
	.cfi_def_cfa_register 5
#APP
# 5 "/export/users/nrinskay/art_optimization_tests/m-dev/src/OptimizationTests/AutoFastJNIDetection//Nop/JniTest.cpp" 1
	nop
	
# 0 "" 2
#NO_APP
	movl	16(%ebp), %eax
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE234:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Nop_Main_nativeNop, .-Java_OptimizationTests_AutoFastJNIDetection_Nop_Main_nativeNop
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",@progbits
