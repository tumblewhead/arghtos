---
type: Proof
title: "P2 - Context and stack integrity"
description: "Scheduler and exception execution use MSP while task execution uses the task PSP, with bounded statically assigned stacks."
tags: [scheduler, cortex-m23, proof, psp, msp, context-switch]
generated:
  by: openai/gpt-5.6-sol
  at: 2026-10-27T20:08:00Z
status: draft
sources:
  - id: arm-cortex-m23-dgu
    title: "Arm Cortex-M23 Devices Generic User Guide"
    resource: https://documentation-service.arm.com/static/5eff4394cafe527e86f5b3c4
  - id: armv8m-arm
    title: "Armv8-M Architecture Reference Manual"
    resource: https://documentation-service.arm.com/static/5f8eff92f86e16515cdbe5e0
sources:
  - id: arm-cortex-m23
    title: "Arm Cortex-M23 processor documentation"
    resource: https://developer.arm.com/Processors/Cortex-M23
---

# Statement

During normal operation:

1. scheduler and exception handlers execute using MSP;
2. task Thread-mode execution uses PSP;
3. each task has one statically assigned stack region;
4. a dispatched task's PSP refers to that task's saved continuation;
5. a context switch preserves every architectural register required by the selected Cortex-M23 ABI and exception model; and
6. the scheduler never intentionally restores a PSP outside the selected task's configured stack region.

# Assumptions

* The target is Cortex-M23 without TrustZone use.
* No floating-point context exists in the scheduler contract.
* Task stacks are application-owned static storage.
* Stack-region non-overlap is a configuration/link-time responsibility.
* Arbitrary memory corruption cannot be contained without an MPU and is not claimed to be recoverable.

# Proof sketch

For each task `t`, define a closed-open stack interval `S_t = [base_t, end_t)`.

[P0](P0-static-configuration-validity.md) establishes that each task has one statically configured stack region and that task identity is finite.

The port saves the outgoing task continuation to the runtime record for exactly the currently running task, then changes scheduler state, then restores the continuation associated with exactly the selected READY task.

Because Handler mode uses the scheduler/exception stack and Thread mode uses the selected task PSP, ordinary task stack consumption does not consume MSP and scheduler exception execution does not consume a task's PSP stack.

If configuration checks establish that every saved/restored PSP lies within its task interval and that all `S_t` intervals are pairwise non-overlapping, restoring task `t` cannot intentionally select another task's stack.

# Architecture test

The Cortex-M23 port SHALL have tests or static checks for:

1. initial synthetic task-frame layout;
2. save/restore symmetry for all software-saved registers;
3. correct exception return into Thread mode using PSP;
4. MSP use in scheduler/exception handlers;
5. required stack alignment and the possible 4-byte hardware exception-alignment padding indicated by stacked xPSR bit 9;
6. pairwise stack-region non-overlap;
7. PSP-in-range checks in debug or validation builds; and
8. a stack sentinel or equivalent low-cost high-water/corruption diagnostic when enabled; and
9. at least 68 bytes of scheduler context reserve in each configured task stack before application stack demand is added.

A hardware test SHALL context-switch between tasks containing distinct register canaries and demonstrate exact restoration.

# Context-size note

The synthesized initial frame is 64 bytes when created on an 8-byte-aligned stack top. A later exception can require one additional 4-byte alignment word, so runtime scheduler context overhead can reach 68 bytes.

# Non-claim

This proof does not claim memory isolation. With no MPU-based containment requirement, a task that writes outside its assigned memory can corrupt scheduler or other-task state.

# Dependencies

Depends on [P0](P0-static-configuration-validity.md) and the concrete Cortex-M23 port contract produced during architecture design.
