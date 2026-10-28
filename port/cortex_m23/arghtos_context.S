.syntax unified
.thumb

.global arghtos_port_service_call
.type arghtos_port_service_call, %function
.thumb_func
arghtos_port_service_call:
    svc #0
    bx lr
.size arghtos_port_service_call, .-arghtos_port_service_call

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
    mov lr, r3
    bx lr
.size SVC_Handler, .-SVC_Handler

.global PendSV_Handler
.type PendSV_Handler, %function
.thumb_func
PendSV_Handler:
    mov r3, lr
    movs r2, #4
    tst r3, r2
    beq 1f

    mrs r0, psp
    subs r0, #32
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
    b 2f

1:
    movs r0, #0

2:
    mrs r4, primask
    cpsid i

3:
    bl arghtos_kernel_dispatch
    cmp r0, #0
    bne 4f
    bl arghtos_port_idle_wait
    movs r0, #0
    b 3b

4:
    mov r1, r4
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
    msr primask, r1
    ldr r2, =0xFFFFFFFD
    mov lr, r2
    bx lr
.size PendSV_Handler, .-PendSV_Handler


.global HardFault_Handler
.type HardFault_Handler, %function
.thumb_func
HardFault_Handler:
    mov r0, lr
    mrs r2, msp
    mrs r3, psp
    movs r1, #4
    tst r0, r1
    beq 1f
    mov r1, r3
    b arghtos_m23_hardfault_entry
1:
    mov r1, r2
    b arghtos_m23_hardfault_entry
.size HardFault_Handler, .-HardFault_Handler
