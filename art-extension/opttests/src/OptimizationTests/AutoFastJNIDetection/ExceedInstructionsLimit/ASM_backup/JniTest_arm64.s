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
	.global	Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit
	.type	Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit, %function
Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit:
.LFB237:
	.cfi_startproc
	sub	sp, sp, #160
	.cfi_def_cfa_offset 160
	str	x0, [sp,24]
	str	x1, [sp,16]
	str	w2, [sp,12]
	str	w3, [sp,8]
	str	w4, [sp,4]
	ldr	w0, [sp,12]
	add	w0, w0, 1
	str	w0, [sp,156]
	ldr	w0, [sp,156]
	add	w1, w0, 2
	ldr	w0, [sp,8]
	add	w0, w1, w0
	str	w0, [sp,152]
	ldr	w0, [sp,152]
	add	w1, w0, 3
	ldr	w0, [sp,4]
	add	w0, w1, w0
	str	w0, [sp,148]
	ldr	w0, [sp,156]
	sub	w1, w0, #4
	ldr	w0, [sp,8]
	add	w0, w1, w0
	str	w0, [sp,144]
	ldr	w0, [sp,148]
	sub	w1, w0, #4
	ldr	w2, [sp,12]
	ldr	w0, [sp,8]
	mul	w0, w2, w0
	sub	w0, w1, w0
	str	w0, [sp,140]
	ldr	w1, [sp,148]
	ldr	w0, [sp,152]
	mul	w0, w1, w0
	sub	w1, w0, #4
	ldr	w2, [sp,12]
	ldr	w0, [sp,8]
	mul	w0, w2, w0
	sub	w0, w1, w0
	str	w0, [sp,136]
	ldr	w1, [sp,148]
	ldr	w0, [sp,156]
	mul	w2, w1, w0
	ldr	w1, [sp,8]
	mov	w0, w1
	lsl	w1, w1, 1
	sub	w0, w0, w1
	lsl	w0, w0, 2
	add	w1, w2, w0
	ldr	w2, [sp,12]
	ldr	w0, [sp,8]
	mul	w0, w2, w0
	sub	w0, w1, w0
	str	w0, [sp,132]
	ldr	w1, [sp,148]
	ldr	w0, [sp,152]
	mul	w2, w1, w0
	ldr	w1, [sp,4]
	mov	w0, w1
	lsl	w1, w1, 1
	sub	w0, w0, w1
	lsl	w0, w0, 2
	add	w2, w2, w0
	ldr	w1, [sp,12]
	mov	w0, w1
	lsl	w1, w1, 1
	sub	w0, w0, w1
	lsl	w0, w0, 2
	add	w0, w2, w0
	str	w0, [sp,128]
	ldr	w1, [sp,132]
	ldr	w0, [sp,156]
	mul	w2, w1, w0
	ldr	w1, [sp,12]
	mov	w0, w1
	lsl	w1, w1, 2
	sub	w0, w0, w1
	lsl	w0, w0, 1
	add	w2, w2, w0
	ldr	w1, [sp,8]
	mov	w0, w1
	lsl	w0, w0, 2
	add	w0, w0, w1
	neg	w0, w0
	add	w0, w2, w0
	str	w0, [sp,124]
	ldr	w1, [sp,132]
	ldr	w0, [sp,136]
	sub	w1, w1, w0
	ldr	w0, [sp,140]
	sub	w0, w1, w0
	str	w0, [sp,120]
	ldr	w0, [sp,8]
	add	w0, w0, 1
	str	w0, [sp,116]
	ldr	w0, [sp,116]
	add	w1, w0, 2
	ldr	w0, [sp,8]
	add	w0, w1, w0
	str	w0, [sp,112]
	ldr	w0, [sp,112]
	add	w1, w0, 3
	ldr	w0, [sp,4]
	add	w0, w1, w0
	str	w0, [sp,108]
	ldr	w0, [sp,116]
	sub	w1, w0, #4
	ldr	w0, [sp,8]
	add	w0, w1, w0
	str	w0, [sp,104]
	ldr	w0, [sp,108]
	sub	w1, w0, #4
	ldr	w2, [sp,8]
	ldr	w0, [sp,8]
	mul	w0, w2, w0
	sub	w0, w1, w0
	str	w0, [sp,100]
	ldr	w1, [sp,108]
	ldr	w0, [sp,112]
	mul	w0, w1, w0
	sub	w1, w0, #4
	ldr	w2, [sp,8]
	ldr	w0, [sp,8]
	mul	w0, w2, w0
	sub	w0, w1, w0
	str	w0, [sp,96]
	ldr	w1, [sp,108]
	ldr	w0, [sp,116]
	mul	w2, w1, w0
	ldr	w1, [sp,8]
	mov	w0, w1
	lsl	w1, w1, 1
	sub	w0, w0, w1
	lsl	w0, w0, 2
	add	w1, w2, w0
	ldr	w2, [sp,8]
	ldr	w0, [sp,8]
	mul	w0, w2, w0
	sub	w0, w1, w0
	str	w0, [sp,92]
	ldr	w1, [sp,108]
	ldr	w0, [sp,112]
	mul	w2, w1, w0
	ldr	w1, [sp,4]
	mov	w0, w1
	lsl	w1, w1, 1
	sub	w0, w0, w1
	lsl	w0, w0, 2
	add	w2, w2, w0
	ldr	w1, [sp,8]
	mov	w0, w1
	lsl	w1, w1, 1
	sub	w0, w0, w1
	lsl	w0, w0, 2
	add	w0, w2, w0
	str	w0, [sp,88]
	ldr	w1, [sp,92]
	ldr	w0, [sp,116]
	mul	w2, w1, w0
	ldr	w1, [sp,8]
	mov	w0, w1
	lsl	w1, w1, 2
	sub	w0, w0, w1
	lsl	w0, w0, 1
	add	w2, w2, w0
	ldr	w1, [sp,8]
	mov	w0, w1
	lsl	w0, w0, 2
	add	w0, w0, w1
	neg	w0, w0
	add	w0, w2, w0
	str	w0, [sp,84]
	ldr	w1, [sp,92]
	ldr	w0, [sp,96]
	sub	w1, w1, w0
	ldr	w0, [sp,100]
	sub	w0, w1, w0
	str	w0, [sp,80]
	ldr	w0, [sp,4]
	add	w0, w0, 1
	str	w0, [sp,76]
	ldr	w0, [sp,76]
	add	w1, w0, 2
	ldr	w0, [sp,8]
	add	w0, w1, w0
	str	w0, [sp,72]
	ldr	w0, [sp,72]
	add	w1, w0, 3
	ldr	w0, [sp,4]
	add	w0, w1, w0
	str	w0, [sp,68]
	ldr	w0, [sp,76]
	sub	w1, w0, #4
	ldr	w0, [sp,8]
	add	w0, w1, w0
	str	w0, [sp,64]
	ldr	w0, [sp,68]
	sub	w1, w0, #4
	ldr	w2, [sp,4]
	ldr	w0, [sp,8]
	mul	w0, w2, w0
	sub	w0, w1, w0
	str	w0, [sp,60]
	ldr	w1, [sp,68]
	ldr	w0, [sp,72]
	mul	w0, w1, w0
	sub	w1, w0, #4
	ldr	w2, [sp,4]
	ldr	w0, [sp,8]
	mul	w0, w2, w0
	sub	w0, w1, w0
	str	w0, [sp,56]
	ldr	w1, [sp,68]
	ldr	w0, [sp,76]
	mul	w2, w1, w0
	ldr	w1, [sp,8]
	mov	w0, w1
	lsl	w1, w1, 1
	sub	w0, w0, w1
	lsl	w0, w0, 2
	add	w1, w2, w0
	ldr	w2, [sp,4]
	ldr	w0, [sp,8]
	mul	w0, w2, w0
	sub	w0, w1, w0
	str	w0, [sp,52]
	ldr	w1, [sp,68]
	ldr	w0, [sp,72]
	mul	w2, w1, w0
	ldr	w1, [sp,4]
	mov	w0, w1
	lsl	w1, w1, 1
	sub	w0, w0, w1
	lsl	w0, w0, 2
	add	w2, w2, w0
	ldr	w1, [sp,4]
	mov	w0, w1
	lsl	w1, w1, 1
	sub	w0, w0, w1
	lsl	w0, w0, 2
	add	w0, w2, w0
	str	w0, [sp,48]
	ldr	w1, [sp,52]
	ldr	w0, [sp,76]
	mul	w2, w1, w0
	ldr	w1, [sp,4]
	mov	w0, w1
	lsl	w1, w1, 2
	sub	w0, w0, w1
	lsl	w0, w0, 1
	add	w2, w2, w0
	ldr	w1, [sp,8]
	mov	w0, w1
	lsl	w0, w0, 2
	add	w0, w0, w1
	neg	w0, w0
	add	w0, w2, w0
	str	w0, [sp,44]
	ldr	w1, [sp,52]
	ldr	w0, [sp,56]
	sub	w1, w1, w0
	ldr	w0, [sp,60]
	sub	w0, w1, w0
	str	w0, [sp,40]
	ldr	w1, [sp,156]
	ldr	w0, [sp,152]
	add	w1, w1, w0
	ldr	w0, [sp,148]
	add	w1, w1, w0
	ldr	w0, [sp,144]
	add	w1, w1, w0
	ldr	w0, [sp,140]
	add	w1, w1, w0
	ldr	w0, [sp,136]
	add	w1, w1, w0
	ldr	w0, [sp,132]
	add	w1, w1, w0
	ldr	w0, [sp,128]
	add	w1, w1, w0
	ldr	w0, [sp,124]
	add	w1, w1, w0
	ldr	w0, [sp,120]
	add	w1, w1, w0
	ldr	w0, [sp,116]
	add	w1, w1, w0
	ldr	w0, [sp,112]
	add	w1, w1, w0
	ldr	w0, [sp,108]
	add	w1, w1, w0
	ldr	w0, [sp,104]
	add	w1, w1, w0
	ldr	w0, [sp,100]
	add	w1, w1, w0
	ldr	w0, [sp,96]
	add	w1, w1, w0
	ldr	w0, [sp,92]
	add	w1, w1, w0
	ldr	w0, [sp,88]
	add	w1, w1, w0
	ldr	w0, [sp,84]
	add	w1, w1, w0
	ldr	w0, [sp,80]
	add	w1, w1, w0
	ldr	w0, [sp,76]
	add	w1, w1, w0
	ldr	w0, [sp,72]
	add	w1, w1, w0
	ldr	w0, [sp,68]
	add	w1, w1, w0
	ldr	w0, [sp,64]
	add	w1, w1, w0
	ldr	w0, [sp,60]
	add	w1, w1, w0
	ldr	w0, [sp,56]
	add	w1, w1, w0
	ldr	w0, [sp,52]
	add	w1, w1, w0
	ldr	w0, [sp,48]
	add	w1, w1, w0
	ldr	w0, [sp,44]
	add	w1, w1, w0
	ldr	w0, [sp,40]
	add	w0, w1, w0
	add	sp, sp, 160
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit, .-Java_OptimizationTests_AutoFastJNIDetection_ExceedInstructionsLimit_Main_nativeExceedInstructionsLimit
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
