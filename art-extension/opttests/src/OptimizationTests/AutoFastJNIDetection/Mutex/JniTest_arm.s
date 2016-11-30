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
	.hidden	count_mutex
	.global	count_mutex
	.bss
	.align	2
	.type	count_mutex, %object
	.size	count_mutex, 4
count_mutex:
	.space	4
	.hidden	count
	.global	count
	.data
	.align	2
	.type	count, %object
	.size	count, 4
count:
	.word	5
	.text
	.align	2
	.global	Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex
	.type	Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex, %function
Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex:
	.fnstart
.LFB243:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	.save {fp, lr}
	.setfp fp, sp, #4
	add	fp, sp, #4
	.pad #16
	sub	sp, sp, #16
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	str	r2, [fp, #-16]
	ldr	r3, .L3
.LPIC0:
	add	r3, pc, r3
	mov	r0, r3
	bl	pthread_mutex_lock(PLT)
	ldr	r3, .L3+4
.LPIC1:
	add	r3, pc, r3
	ldr	r3, [r3]
	add	r2, r3, #1
	ldr	r3, .L3+8
.LPIC2:
	add	r3, pc, r3
	str	r2, [r3]
	ldr	r3, .L3+12
.LPIC3:
	add	r3, pc, r3
	mov	r0, r3
	bl	pthread_mutex_unlock(PLT)
	ldr	r3, .L3+16
.LPIC4:
	add	r3, pc, r3
	ldr	r2, [r3]
	ldr	r3, [fp, #-16]
	add	r3, r2, r3
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
.L4:
	.align	2
.L3:
	.word	count_mutex-(.LPIC0+8)
	.word	count-(.LPIC1+8)
	.word	count-(.LPIC2+8)
	.word	count_mutex-(.LPIC3+8)
	.word	count-(.LPIC4+8)
	.fnend
	.size	Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex, .-Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex
	.ident	"GCC: (GNU) 4.8"
	.section	.note.GNU-stack,"",%progbits
