.syntax unified
.thumb

.extern task_stub
.extern arghtos_task_start_trampoline
.extern arghtos_task_return_guard

.section .app_stacks,"aw",%progbits
.balign 8

.global app_stacks
.type app_stacks, %object
app_stacks:

.macro ARGHTOS_PREBUILT_STACK
    /* 64 bytes ordinary task stack below the initial saved context. */
    .space 64

    /* Software-saved R4-R11. */
    .word 0, 0, 0, 0, 0, 0, 0, 0

    /* Hardware frame: R0-R3, R12, LR, PC, xPSR. */
    .word task_stub
    .word 0, 0, 0, 0
    .word arghtos_task_return_guard
    .word arghtos_task_start_trampoline - 1
    .word 0x01000000
.endm

.rept 24
    ARGHTOS_PREBUILT_STACK
.endr

.size app_stacks, .-app_stacks
