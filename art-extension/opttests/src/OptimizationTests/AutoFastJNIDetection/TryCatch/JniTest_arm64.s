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
	.global	Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch
	.type	Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch, %function
Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch:
.LFB237:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA237
	stp	x29, x30, [sp, -80]!
	.cfi_def_cfa_offset 80
	.cfi_offset 29, -80
	.cfi_offset 30, -72
	add	x29, sp, 0
	.cfi_def_cfa_register 29
	str	x19, [sp,16]
	.cfi_offset 19, -64
	str	x0, [x29,56]
	str	x1, [x29,48]
	str	w2, [x29,44]
	str	w3, [x29,40]
	str	w4, [x29,36]
	mov	w0, -111
	str	w0, [x29,76]
	mov	x0, 4
	bl	__cxa_allocate_exception
	mov	x2, x0
	ldr	w1, [x29,44]
	ldr	w0, [x29,40]
	add	w1, w1, w0
	ldr	w0, [x29,36]
	add	w0, w1, w0
	str	w0, [x2]
	adrp	x0, :got:_ZTIi
	ldr	x1, [x0, #:got_lo12:_ZTIi]
	mov	x0, x2
	mov	x2, 0
.LEHB0:
	bl	__cxa_throw
.LEHE0:
.L6:
	mov	x2, x0
	mov	x0, x1
	cmp	x0, 1
	beq	.L5
	mov	x0, x2
.LEHB1:
	bl	_Unwind_Resume
.LEHE1:
.L5:
	mov	x0, x2
	bl	__cxa_begin_catch
	ldr	w0, [x0]
	str	w0, [x29,72]
	ldr	w1, [x29,72]
	ldr	w0, [x29,76]
	add	w19, w1, w0
	bl	__cxa_end_catch
	mov	w0, w19
	ldr	x19, [sp,16]
	.cfi_restore 19
	ldp	x29, x30, [sp], 80
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa 31, 0
	ret
	.cfi_endproc
.LFE237:
	.global	__gxx_personality_v0
	.section	.gcc_except_table,"a",%progbits
	.align	2
.LLSDA237:
	.byte	0xff
	.byte	0x9b
	.uleb128 .LLSDATT237-.LLSDATTD237
.LLSDATTD237:
	.byte	0x1
	.uleb128 .LLSDACSE237-.LLSDACSB237
.LLSDACSB237:
	.uleb128 .LEHB0-.LFB237
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L6-.LFB237
	.uleb128 0x1
	.uleb128 .LEHB1-.LFB237
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0
	.uleb128 0
.LLSDACSE237:
	.byte	0x1
	.byte	0
	.align	2
	.4byte	DW.ref._ZTIi-.
.LLSDATT237:
	.text
	.size	Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch, .-Java_OptimizationTests_AutoFastJNIDetection_TryCatch_Main_nativeTryCatch
	.hidden	DW.ref._ZTIi
	.weak	DW.ref._ZTIi
	.section	.data.DW.ref._ZTIi,"awG",%progbits,DW.ref._ZTIi,comdat
	.align	3
	.type	DW.ref._ZTIi, %object
	.size	DW.ref._ZTIi, 8
DW.ref._ZTIi:
	.xword	_ZTIi
	.hidden	DW.ref.__gxx_personality_v0
	.weak	DW.ref.__gxx_personality_v0
	.section	.data.DW.ref.__gxx_personality_v0,"awG",%progbits,DW.ref.__gxx_personality_v0,comdat
	.align	3
	.type	DW.ref.__gxx_personality_v0, %object
	.size	DW.ref.__gxx_personality_v0, 8
DW.ref.__gxx_personality_v0:
	.xword	__gxx_personality_v0
	.ident	"GCC: (GNU) 4.9 20140827 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
