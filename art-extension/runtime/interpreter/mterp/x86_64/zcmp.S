/*
 * Generic one-operand compare-and-branch operation.  Provide a "revcmp"
 * fragment that specifies the *reverse* comparison to perform, e.g.
 * for "if-le" you would use "gt".
 *
 * for: if-eqz, if-nez, if-ltz, if-gez, if-gtz, if-lez
 */
    /* if-cmp vAA, +BBBB */
    cmpl    $$0, VREG_ADDRESS(rINSTq)       # compare (vA, 0)
    j${revcmp}   1f
    movswq  2(rPC), rINSTq                  # fetch signed displacement
    testq   rINSTq, rINSTq
    jmp     MterpCommonTakenBranch
1:
    cmpl    $$JIT_CHECK_OSR, rPROFILE
    je      .L_check_not_taken_osr
    cmpl    $$0, OFF_FP_PROFILE_ACTIVE(rFP)
    jne     2f
    ADVANCE_PC_FETCH_AND_GOTO_NEXT 2
2:  ADVANCE_PC 2
    EXPORT_PC
    leaq    OFF_FP_SHADOWFRAME(rFP), OUT_ARG0
    call    SYMBOL(MterpIncrementBB)          # (shadow_frame)
    FETCH_INST
    GOTO_NEXT
