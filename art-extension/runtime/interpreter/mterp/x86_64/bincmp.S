/*
 * Generic two-operand compare-and-branch operation.  Provide a "revcmp"
 * fragment that specifies the *reverse* comparison to perform, e.g.
 * for "if-le" you would use "gt".
 *
 * For: if-eq, if-ne, if-lt, if-ge, if-gt, if-le
 */
    /* if-cmp vA, vB, +CCCC */
    movl    rINST, %ecx                     # rcx <- A+
    sarl    $$4, rINST                      # rINST <- B
    andb    $$0xf, %cl                      # rcx <- A
    GET_VREG %eax, %rcx                     # eax <- vA
    cmpl    VREG_ADDRESS(rINSTq), %eax      # compare (vA, vB)
    j${revcmp}   1f
    movswq  2(rPC), rINSTq                  # Get signed branch offset
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
