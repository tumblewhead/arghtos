# Cortex-M23 reference port

This directory implements the architecture-neutral `arghtos_port.h` ABI for Arm Cortex-M23 / Armv8-M Baseline.

## Execution model

- Tasks: privileged Thread mode on PSP.
- Kernel and exceptions: Handler mode on MSP.
- Task service call: fixed `SVC #0`, service ID in R0, arg0 in R1, result in R0.
- Dispatch request / task handoff: PendSV.
- Scheduler serialization: PRIMASK.
- Idle: PRIMASK-serialized WFI protocol.
- Context token: saved PSP address encoded in `arghtos_context_t`.

## Context layout

The software-saved frame contains R4-R11 below the Cortex-M basic hardware frame. The synthesized initial frame is 64 bytes. Runtime exception stacking can consume one additional 4-byte alignment word, so `arghtos_port_stack_valid()` requires at least 68 bytes of scheduler-context reserve before application stack demand.

## Board adapter required

The MCU/board integration supplies `arghtos_m23_board_time_now()`, `arghtos_m23_board_timer_arm()`, `arghtos_m23_board_timer_disarm()`, and `arghtos_m23_board_fault_terminal()`.

The board adapter must document scheduler tick frequency, timer width, timer-delivery jitter, and clock-change behavior.

## Crash payload format 0x4D23

`port_state[]` words are MSP, PSP, EXC_RETURN, stacked R0, R1, R2, R3, LR, PC, and xPSR.

## Trusted privileged-task restriction

Because v1 tasks are privileged, task code must not directly modify PRIMASK, CONTROL stack selection, PSP/MSP, PendSV/SVC configuration, or scheduler exception priorities.

## Verification still required on hardware

- R4-R11 canary preservation.
- First context restore.
- SVC service-call hardware validation.
- PendSV save/restore and same-task reselect.
- Runtime alignment-padding case.
- Nested PRIMASK restoration.
- Idle check-to-WFI stress.
- MSP/PSP high-water measurements.
- Board timer jitter and wrap behavior.
