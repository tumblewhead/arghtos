.syntax unified
.thumb

#if ARGHTOS_R5AE_LOCAL_SERVICE_SECTION
.section .text.arghtos_service_veneer, "ax", %progbits
#endif

.global arghtos_port_service_call
.type arghtos_port_service_call, %function
.thumb_func
arghtos_port_service_call:
    svc #0
    bx lr
.size arghtos_port_service_call, .-arghtos_port_service_call

#if ARGHTOS_R6AM_ALIGNED_FIRST_DISPATCH_MSP
/*
 * R6am selected startup path.
 *
 * Entry is a C ABI boundary, so MSP is 8-byte aligned. This routine does not
 * touch SP/MSP before the first pending exception becomes eligible.
 */
.global arghtos_port_start
.type arghtos_port_start, %function
.thumb_func
arghtos_port_start:
    ldr r0, =0xE000ED04
    movs r1, #1
    lsls r1, r1, #28
    str r1, [r0]
    cpsie i
    isb
1:
    wfi
    b.n 1b
.size arghtos_port_start, .-arghtos_port_start

#endif

#if ARGHTOS_R6AK_ASM_PERIODIC_TRAMPOLINE
/*
 * Generated periodic entry pointer lives in task R4.
 * TASK_RETURN ignores arg0/R1 in the trusted service domain.
 */
.global arghtos_task_start_trampoline
.type arghtos_task_start_trampoline, %function
.thumb_func
arghtos_task_start_trampoline:
1:
    blx r4
    movs r0, #4
    svc #0
    b.n 1b
.size arghtos_task_start_trampoline, .-arghtos_task_start_trampoline

#endif
#if ARGHTOS_R5AD_COMPACT_API_VENEERS
/*
 * Public task-context service veneers for the trusted minimum-size target.
 * YIELD and WAIT leave R1 unchanged because their service cases ignore arg0.
 */
.global arghtos_yield
.type arghtos_yield, %function
.thumb_func
arghtos_yield:
    movs r0, #0
    b.n arghtos_port_service_call
.size arghtos_yield, .-arghtos_yield

.global arghtos_wait
.type arghtos_wait, %function
.thumb_func
arghtos_wait:
    movs r0, #1
    b.n arghtos_port_service_call
.size arghtos_wait, .-arghtos_wait

.global arghtos_sleep_until
.type arghtos_sleep_until, %function
.thumb_func
arghtos_sleep_until:
    mov r1, r0
    movs r0, #2
    b.n arghtos_port_service_call
.size arghtos_sleep_until, .-arghtos_sleep_until

.global arghtos_wake
.type arghtos_wake, %function
.thumb_func
arghtos_wake:
#if !ARGHTOS_R6AB_SINGLE_WAKE_TARGET
    mov r1, r0
#endif
    movs r0, #3
    b.n arghtos_port_service_call
.size arghtos_wake, .-arghtos_wake
#endif

#if ARGHTOS_R5AE_LOCAL_SERVICE_SECTION
.text
#endif

.global SVC_Handler
.type SVC_Handler, %function
.thumb_func
SVC_Handler:
    mrs r2, psp
    push {r2, lr}
    ldr r0, [r2, #0]
    ldr r1, [r2, #4]
    bl arghtos_kernel_service
    pop {r2, r3}
    str r0, [r2, #0]
#if ARGHTOS_COMPACT_SVC_RETURN
    bx r3
#else
    mov lr, r3
    bx lr
#endif
.size SVC_Handler, .-SVC_Handler

.global PendSV_Handler
.type PendSV_Handler, %function
.thumb_func
PendSV_Handler:
    mov r3, lr
#if ARGHTOS_R5B_COMPACT_EXC_CHECKS
    lsrs r3, r3, #3
    bcc 1f
#else
    movs r2, #4
    tst r3, r2
    beq 1f
#endif

    mrs r0, psp
    subs r0, #32
#if ARGHTOS_COMPACT_PENDSV
    mov r1, r0
    stmia r1!, {r4-r7}
    mov r4, r8
    mov r5, r9
    mov r6, r10
    mov r7, r11
    stmia r1!, {r4-r7}
#else
    str r4, [r0, #0]
    str r5, [r0, #4]
    str r6, [r0, #8]
    str r7, [r0, #12]
    mov r1, r8
    str r1, [r0, #16]
    mov r1, r9
    str r1, [r0, #20]
    mov r1, r10
    str r1, [r0, #24]
    mov r1, r11
    str r1, [r0, #28]
    movs r5, #0
#endif
    b 2f

1:
    /*
     * First dispatch entered from Thread mode using MSP.
     * Generic profiles inspect stacked xPSR[9] for optional alignment padding.
     * R6am proves the selected startup MSP is already 8-byte aligned and that
     * arghtos_port_start() consumes zero stack, so the frame is exactly 32 B.
     */
    mrs r5, msp
#if ARGHTOS_R6AM_ALIGNED_FIRST_DISPATCH_MSP
    adds r5, #32
#else
    ldr r1, [r5, #28]
#if ARGHTOS_R5AD_COMPACT_API_VENEERS
    adds r5, #32
    lsrs r1, r1, #10
    bcc 5f
    adds r5, #4
5:
#elif ARGHTOS_R5A_COMPACT_PENDSV_TAIL
    lsrs r1, r1, #7
    movs r2, #4
    ands r1, r2
    adds r5, #32
    adds r5, r5, r1
#else
    lsrs r1, r1, #9
    movs r2, #1
    ands r1, r2
    lsls r1, r1, #2
    adds r5, #32
    adds r5, r5, r1
#endif
#endif
    msr msp, r5
    movs r0, #0

2:
#if ARGHTOS_R6I_KNOWN_UNMASKED_PENDSV
    cpsid i
#else
    mrs r4, primask
    cpsid i
#endif

3:
    bl arghtos_kernel_dispatch
    cmp r0, #0
    bne 4f
    bl arghtos_port_idle_wait
    movs r0, #0
    b 3b

4:
#if !ARGHTOS_R6I_KNOWN_UNMASKED_PENDSV
    mov r1, r4
#endif
#if ARGHTOS_COMPACT_PENDSV
    ldmia r0!, {r4-r7}
    ldmia r0!, {r2-r3}
    mov r8, r2
    mov r9, r3
    ldmia r0!, {r2-r3}
    mov r10, r2
    mov r11, r3
    msr psp, r0
#if ARGHTOS_R6I_KNOWN_UNMASKED_PENDSV
    cpsie i
#else
    msr primask, r1
#endif
    movs r2, #2
    mvns r2, r2
#else
    ldr r4, [r0, #0]
    ldr r5, [r0, #4]
    ldr r6, [r0, #8]
    ldr r7, [r0, #12]
    ldr r2, [r0, #16]
    mov r8, r2
    ldr r2, [r0, #20]
    mov r9, r2
    ldr r2, [r0, #24]
    mov r10, r2
    ldr r2, [r0, #28]
    mov r11, r2
    adds r0, #32
    msr psp, r0
#if ARGHTOS_R6I_KNOWN_UNMASKED_PENDSV
    cpsie i
#else
    msr primask, r1
#endif
    ldr r2, =0xFFFFFFFD
#endif
#if ARGHTOS_R5A_COMPACT_PENDSV_TAIL
    bx r2
#else
    mov lr, r2
    bx lr
#endif
.size PendSV_Handler, .-PendSV_Handler


.global HardFault_Handler
.type HardFault_Handler, %function
.thumb_func
HardFault_Handler:
    mov r0, lr
    mrs r2, msp
    mrs r3, psp
#if ARGHTOS_R5B_COMPACT_EXC_CHECKS
    lsrs r1, r0, #3
    bcc 1f
#else
    movs r1, #4
    tst r0, r1
    beq 1f
#endif
    mov r1, r3
#if ARGHTOS_R5AD_COMPACT_API_VENEERS
    b 2f
1:
    mov r1, r2
2:
    b arghtos_m23_hardfault_entry
#else
    b arghtos_m23_hardfault_entry
1:
    mov r1, r2
    b arghtos_m23_hardfault_entry
#endif
.size HardFault_Handler, .-HardFault_Handler

#if ARGHTOS_R5AF_ASM_HARDFAULT_ENTRY
.global arghtos_m23_hardfault_entry
.type arghtos_m23_hardfault_entry, %function
.thumb_func
arghtos_m23_hardfault_entry:
    /*
     * Entry ABI:
     *   r0 = EXC_RETURN
     *   r1 = selected basic frame
     *   r2 = MSP at exception entry
     *   r3 = PSP at exception entry
     *
     * Two pushes create arghtos_m23_fault_context_t exactly:
     *   [sp+0]  msp
     *   [sp+4]  psp
     *   [sp+8]  exc_return
     *   [sp+12] basic_frame
     */
    push {r0, r1}
    push {r2, r3}
    mov r1, sp
    movs r0, #1
    lsls r0, r0, #8
    b arghtos_fatal
.size arghtos_m23_hardfault_entry, .-arghtos_m23_hardfault_entry
#endif

#if ARGHTOS_R5K_ASM_CRASH_FILL
.global arghtos_port_crash_fill
.type arghtos_port_crash_fill, %function
.thumb_func
arghtos_port_crash_fill:
    /* port_format = 0x4D23 */
    movs r2, #0x23
    strb r2, [r0, #16]
    movs r2, #0x4d
    strb r2, [r0, #17]

    cmp r1, #0
    beq 6f

    /*
     * Non-NULL target context. Optimistically set the full payload count;
     * the no-basic-frame branch corrects it to three words.
     */
    movs r2, #10
    strb r2, [r0, #18]

    adds r0, #20
    ldmia r1!, {r2, r3}
    stmia r0!, {r2, r3}
    ldmia r1!, {r2, r3}
    str r2, [r0, #0]

    cmp r3, #0
    beq 5f

    /* Copy stacked R0-R3. */
    adds r0, #4
    ldmia r3!, {r1, r2}
    stmia r0!, {r1, r2}
    ldmia r3!, {r1, r2}
    stmia r0!, {r1, r2}

    /* Skip stacked R12; copy stacked LR, PC, xPSR. */
    adds r3, #4
    ldmia r3!, {r1, r2}
    stmia r0!, {r1, r2}
    ldr r1, [r3, #0]
    str r1, [r0, #0]
    bx lr

5:
    /* r0 currently points at record + 28; port_words is record + 18. */
    subs r0, #10
    movs r2, #3
    strb r2, [r0, #0]
    bx lr

6:
    movs r2, #2
    strb r2, [r0, #18]
    mrs r2, msp
    str r2, [r0, #20]
    mrs r2, psp
    str r2, [r0, #24]
    bx lr
.size arghtos_port_crash_fill, .-arghtos_port_crash_fill
#endif
