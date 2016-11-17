/*
 * Generic one-operand compare-and-branch operation.  Provide a "revcmp"
 * fragment that specifies the *reverse* comparison to perform, e.g.
 * for "if-le" you would use "gt".
 *
 * for: if-eqz, if-nez, if-ltz, if-gez, if-gtz, if-lez
 */
    /* if-cmp vAA, +BBBB */
    cmpl    $$0, VREG_ADDRESS(rINST)        # compare (vA, 0)
    j${revcmp}   1f
    movswl  2(rPC), rINST                   # fetch signed displacement
    testl   rINST, rINST
    jmp     MterpCommonTakenBranch
1:
    cmpw    $$JIT_CHECK_OSR, rPROFILE
    je      .L_check_not_taken_osr
    cmpl    $$0, OFF_FP_PROFILE_ACTIVE(rFP)
    jne     2f
    ADVANCE_PC_FETCH_AND_GOTO_NEXT 2
2:  ADVANCE_PC 2
    EXPORT_PC
    leal    OFF_FP_SHADOWFRAME(rFP), %ecx
    movl    %ecx, OUT_ARG0(%esp)
    call    SYMBOL(MterpIncrementBB)          # (shadow_frame)
    RESTORE_IBASE
    FETCH_INST
    GOTO_NEXT
