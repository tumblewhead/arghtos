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
* Privileged application tasks obey the C4 trusted-task restriction and do not directly modify PRIMASK, PSP/MSP selection, or scheduler-owned exception configuration.

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


# I0 implementation evidence

Static Cortex-M23 disassembly and compiler stack-usage evidence are recorded in [I0 context static audit](../implementation/I0-context-static-audit.md).

Confirmed statically:

* R4-R11 save/restore structure is symmetric in the current PendSV object;
* SVC service arguments/results use the documented stacked R0/R1 ABI;
* HardFault reaches the generic crash path with target context;
* the first-dispatch path reclaims the startup MSP frame;
* the current normal-path ArghtOS-only MSP static lower bound is approximately 104 B.

P2 remains conditional on target register-canary, alignment, PSP/MSP ownership, and high-water tests.

# R4u initial-argument refinement

R4u does not change the size or symmetry of the Cortex-M23 saved task frame.

The only refinement is the value placed in initial stacked R0:

```text
ordinary profile: R0 = 0
R4u minimum profile: R0 = private task-entry pointer
```

R0 is part of the architecture-defined hardware frame already covered by P2.
The initial frame remains 64 B, the worst-case scheduler context reserve remains
68 B, and ordinary preemption still preserves R4-R11 plus the hardware frame.

The R4u trampoline retains the entry pointer in R4 across periodic suspension.
R4 is already included in the software context save/restore proof, so no new
saved state or asymmetry is introduced.

# R5l prebuilt-context refinement

R5l moves initial-frame construction from runtime port code to trusted generated
configuration but does not change the frame being proved.

The proof obligation becomes an equivalence obligation: the generated frame
must match the selected port's ordinary synthesized frame word-for-word for the
same task entry, trampoline, and return guard, and its saved token must remain
inside the task's configured stack interval.

Because the runtime PendSV format is unchanged, the existing save/restore
symmetry and 68-byte worst-case task scheduler-context reserve continue to
apply.

A target build should retain a static/golden-frame check or relocation audit,
and hardware register-canary/first-dispatch testing remains required.

# R6i PendSV mask-state refinement

R6i does not change the saved PSP frame, callee-saved register set, exception
return token, or MSP/PSP ownership.

Under C4's trusted generated precondition that every supported PendSV entry has
`PRIMASK=0`, replacing the save/restore sequence:

```text
read prior PRIMASK
mask
...
restore prior PRIMASK
```

with:

```text
mask
...
enable
```

restores the same architectural mask state on every admitted execution.

The dispatch and idle loop remain fully masked while scheduler state is read or
mutated. Interrupts become eligible at the same restore point immediately
before exception return. Therefore P2 context integrity and C4 race-free idle
ownership are unchanged.

# R6l startup mask-ownership refinement

R6l changes only who owns the scheduler mask on entry to
`arghtos_start()`.

The generated integration establishes PRIMASK=1 before the call. The portable
start transaction never unmasks, and `arghtos_port_start()` remains
non-returning and restores the normal interrupt state before first dispatch.

Therefore there is no additional context-switch interleaving, no changed
MSP/PSP ownership, and no window in which PendSV can observe partially
initialized scheduler state. The first-dispatch context proof is unchanged.

# R6z crash-payload range refinement

The Cortex-M23 crash filler has three bounded control-flow outcomes:

```text
NULL context             -> port_words = 2
context, no basic frame  -> port_words = 3
context + basic frame    -> port_words = 10
```

Each count names only words that the same branch writes, and every count lies
within the public ten-word payload.

Thus the selected port establishes the invariant:

```text
1 <= port_words <= 10
```

before returning to portable fatal capture.

R6z consumes this invariant to remove redundant portable range/zero checks. It
does not change fault-context interpretation, stack ownership, register
extraction, record layout, or commit ordering.

# R6ac crash-format preseed refinement

The selected port's payload decoder contract uses one fixed format identifier
and no runtime path changes that interpretation.

Moving that constant from `arghtos_port_crash_fill()` into the generated
initial crash record changes only the time at which the same 16-bit value is
materialized. The payload words, count, context extraction, and commit ordering
are unchanged.

# R6aj class-specialized initial-frame proof

R6aj changes only two generated words in the sole async initial hardware frame:
stacked PC and stacked LR. Frame size, alignment, saved R4-R11 area, xPSR,
stack interval, and saved PSP token are unchanged.

The async frame's stacked PC names the generated immutable application entry.
Its stacked LR names a non-returning async-return guard. Therefore normal
function return uses the ordinary Cortex-M23 ABI and cannot escape into
unowned memory.

Periodic initial frames remain byte-equivalent to the accepted R5l form.

Target evidence must verify the generated async slot uses direct application PC
plus the async-return guard before this profile is accepted.

# R6ak generated R4 entry-pointer proof

R6ak changes one word of each periodic generated software-saved frame:

~~~text
R4: 0 -> immutable periodic application entry pointer
~~~

and clears the no-longer-needed stacked R0 entry argument.

R4 already belongs to the P2 software-saved register set and is restored before
exception return. The saved-context size, PSP token, hardware frame, xPSR,
alignment, and stack interval are unchanged.

The target frame audit SHALL verify:

* all 23 periodic R4 relocation words target the generated application entry;
* periodic stacked R0 words carry no entry relocation;
* all 23 periodic stacked PCs target the assembly trampoline;
* the sole async stacked PC still targets the application entry directly; and
* all 24 stacked LRs target the return guard.

Thus the new initial frame remains structurally compatible with ordinary
PendSV save/restore.
