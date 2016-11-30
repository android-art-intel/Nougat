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

	.arch armv5te
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 6
	.eabi_attribute 34, 0
	.eabi_attribute 18, 4
	.file	"JniTest.cpp"
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3
	.type	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3, %function
Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3:
	.fnstart
.LFB234:
	@ args = 4, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #16
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	str	r2, [fp, #-16]
	str	r3, [fp, #-20]
	ldr	r2, [fp, #-16]
	ldr	r3, [fp, #-20]
	add	r3, r2, r3
	mov	r0, r3
	ldr	r1, [fp, #4]
	bl	foo1(PLT)
	mov	r3, r0
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
	.cantunwind
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3, .-Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth3
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4
	.type	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4, %function
Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4:
	.fnstart
.LFB235:
	@ args = 4, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #16
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	str	r2, [fp, #-16]
	str	r3, [fp, #-20]
	ldr	r0, [fp, #4]
	ldr	r1, [fp, #-16]
	ldr	r2, [fp, #-20]
	bl	foo2(PLT)
	mov	r3, r0
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
	.cantunwind
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4, .-Java_OptimizationTests_AutoFastJNIDetection_NestedCalls_Main_nativeNestedCallsDepth4
	.align	2
	.global	foo
	.hidden	foo
	.type	foo, %function
foo:
	.fnstart
.LFB236:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	str	r0, [fp, #-8]
	ldr	r3, [fp, #-8]
	add	r3, r3, #5
	mov	r0, r3
	sub	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.cantunwind
	.fnend
	.size	foo, .-foo
	.align	2
	.global	foo1
	.hidden	foo1
	.type	foo1, %function
foo1:
	.fnstart
.LFB237:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {r4, fp, lr}
	add	fp, sp, #8
	sub	sp, sp, #12
	str	r0, [fp, #-16]
	str	r1, [fp, #-20]
	ldr	r0, [fp, #-16]
	bl	foo(PLT)
	mov	r4, r0
	ldr	r0, [fp, #-20]
	bl	foo(PLT)
	mov	r3, r0
	add	r3, r4, r3
	mov	r0, r3
	sub	sp, fp, #8
	@ sp needed
	ldmfd	sp!, {r4, fp, pc}
	.cantunwind
	.fnend
	.size	foo1, .-foo1
	.align	2
	.global	foo2
	.hidden	foo2
	.type	foo2, %function
foo2:
	.fnstart
.LFB238:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {r4, fp, lr}
	add	fp, sp, #8
	sub	sp, sp, #20
	str	r0, [fp, #-16]
	str	r1, [fp, #-20]
	str	r2, [fp, #-24]
	ldr	r2, [fp, #-16]
	ldr	r3, [fp, #-24]
	add	r2, r2, r3
	ldr	r3, [fp, #-20]
	mov	r3, r3, asl #1
	mov	r0, r2
	mov	r1, r3
	bl	foo1(PLT)
	mov	r4, r0
	ldr	r3, [fp, #-16]
	ldr	r2, [fp, #-24]
	mul	r2, r3, r2
	ldr	r1, [fp, #-20]
	ldr	r3, [fp, #-24]
	rsb	r3, r3, r1
	mov	r0, r2
	mov	r1, r3
	bl	foo1(PLT)
	mov	r3, r0
	rsb	r3, r3, r4
	mov	r0, r3
	sub	sp, fp, #8
	@ sp needed
	ldmfd	sp!, {r4, fp, pc}
	.cantunwind
	.fnend
	.size	foo2, .-foo2
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
