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
	.hidden	count_mutex
	.global	count_mutex
	.bss
	.align	3
	.type	count_mutex, %object
	.size	count_mutex, 40
count_mutex:
	.zero	40
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
	adrp	x0, count_mutex
	add	x0, x0, :lo12:count_mutex
	bl	pthread_mutex_lock
	adrp	x0, count
	add	x0, x0, :lo12:count
	ldr	w0, [x0]
	add	w1, w0, 1
	adrp	x0, count
	add	x0, x0, :lo12:count
	str	w1, [x0]
	adrp	x0, count_mutex
	add	x0, x0, :lo12:count_mutex
	bl	pthread_mutex_unlock
	adrp	x0, count
	add	x0, x0, :lo12:count
	ldr	w1, [x0]
	ldr	w0, [x29,28]
	add	w0, w1, w0
	ldp	x29, x30, [sp], 48
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE237:
	.size	Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex, .-Java_OptimizationTests_AutoFastJNIDetection_Mutex_Main_nativeMutex
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
