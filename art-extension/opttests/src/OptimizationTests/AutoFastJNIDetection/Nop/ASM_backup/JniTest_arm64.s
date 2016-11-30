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

	.cpu generic+fp+simd
	.file	"JniTest.cpp"
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_Nop_Main_nativeNop
	.type	Java_OptimizationTests_AutoFastJNIDetection_Nop_Main_nativeNop, %function
Java_OptimizationTests_AutoFastJNIDetection_Nop_Main_nativeNop:
.LFB237:
	.cfi_startproc
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	str	x0, [sp,24]
	str	x1, [sp,16]
	str	w2, [sp,12]
#APP
// 5 "/export/users/nrinskay/art_optimization_tests/m-dev/src/OptimizationTests/AutoFastJNIDetection//Nop/JniTest.cpp" 1
	nop
	
// 0 "" 2
#NO_APP
	ldr	w0, [sp,12]
	add	sp, sp, 32
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Nop_Main_nativeNop, .-Java_OptimizationTests_AutoFastJNIDetection_Nop_Main_nativeNop
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
