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
	.global	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3
	.type	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3, %function
Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3:
.LFB237:
	.cfi_startproc
	stp	x29, x30, [sp, -48]!
	.cfi_def_cfa_offset 48
	.cfi_offset 29, -48
	.cfi_offset 30, -40
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	x0, [x29,40]
	str	x1, [x29,32]
	str	w2, [x29,28]
	str	w3, [x29,24]
	str	w4, [x29,20]
	ldr	w1, [x29,28]
	ldr	w0, [x29,24]
	add	w0, w1, w0
	ldr	w1, [x29,20]
	bl	foo1
	nop
	ldp	x29, x30, [sp], 48
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3, .-Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4
	.type	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4, %function
Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4:
.LFB238:
	.cfi_startproc
	stp	x29, x30, [sp, -48]!
	.cfi_def_cfa_offset 48
	.cfi_offset 29, -48
	.cfi_offset 30, -40
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	x0, [x29,40]
	str	x1, [x29,32]
	str	w2, [x29,28]
	str	w3, [x29,24]
	str	w4, [x29,20]
	ldr	w0, [x29,20]
	ldr	w1, [x29,28]
	ldr	w2, [x29,24]
	bl	foo2
	nop
	ldp	x29, x30, [sp], 48
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE238:
	.size	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4, .-Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4
	.align	2
	.global	foo
	.hidden	foo
	.type	foo, %function
foo:
.LFB239:
	.cfi_startproc
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	w0, [sp,12]
	ldr	w0, [sp,12]
	add	w0, w0, 5
	add	sp, sp, 16
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE239:
	.size	foo, .-foo
	.align	2
	.global	foo1
	.hidden	foo1
	.type	foo1, %function
foo1:
.LFB240:
	.cfi_startproc
	stp	x29, x30, [sp, -48]!
	.cfi_def_cfa_offset 48
	.cfi_offset 29, -48
	.cfi_offset 30, -40
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	x19, [sp,16]
	.cfi_offset 19, -32
	str	w0, [x29,44]
	str	w1, [x29,40]
	ldr	w0, [x29,44]
	bl	foo
	mov	w19, w0
	ldr	w0, [x29,40]
	bl	foo
	add	w0, w19, w0
	ldr	x19, [sp,16]
	.cfi_restore 19
	ldp	x29, x30, [sp], 48
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE240:
	.size	foo1, .-foo1
	.align	2
	.global	foo2
	.hidden	foo2
	.type	foo2, %function
foo2:
.LFB241:
	.cfi_startproc
	stp	x29, x30, [sp, -48]!
	.cfi_def_cfa_offset 48
	.cfi_offset 29, -48
	.cfi_offset 30, -40
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	x19, [sp,16]
	.cfi_offset 19, -32
	str	w0, [x29,44]
	str	w1, [x29,40]
	str	w2, [x29,36]
	ldr	w1, [x29,44]
	ldr	w0, [x29,36]
	add	w2, w1, w0
	ldr	w0, [x29,40]
	lsl	w1, w0, 1
	mov	w0, w2
	bl	foo1
	mov	w19, w0
	ldr	w1, [x29,44]
	ldr	w0, [x29,36]
	mul	w2, w1, w0
	ldr	w1, [x29,40]
	ldr	w0, [x29,36]
	sub	w1, w1, w0
	mov	w0, w2
	bl	foo1
	sub	w0, w19, w0
	ldr	x19, [sp,16]
	.cfi_restore 19
	ldp	x29, x30, [sp], 48
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE241:
	.size	foo2, .-foo2
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
