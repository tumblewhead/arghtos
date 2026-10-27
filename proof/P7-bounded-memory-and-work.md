---
type: Proof
title: "P7 - Bounded memory and scheduler work"
description: "Scheduler memory, critical sections, dispatch work, and fault capture are finite and bounded by compile-time configuration."
tags: [scheduler, cortex-m23, proof, bounded-work, memory, critical-section]
generated:
  by: openai/gpt-5.6-sol
  at: 2026-10-27T20:08:00Z
status: draft
---

# Statement

For a fixed `SCHED_MAX_TASKS=N`, every scheduler operation terminates after a bounded amount of scheduler-owned work, scheduler-owned SRAM is compile-time bounded, and every interrupt-disabled critical section has a finite architecture-defined upper bound.

# Memory bound

[P0](P0-static-configuration-validity.md) establishes:

`M(N) = M_fixed + N * M_task + M_sets(N)`.

No runtime operation increases `N` or allocates scheduler storage.

Therefore scheduler-owned SRAM is bounded for the lifetime of the program.

# Work bound

The scheduler SHALL contain no loop whose iteration count depends on an unbounded runtime-produced collection.

Permitted loops must have a compile-time upper bound such as:

* at most `N` task slots;
* at most a fixed number of priority bitmap words;
* at most the statically configured number of periodic releases made due by one timestamp; or
* a fixed architectural register count during context save/restore.

Thus each scheduler path is finite.

An `O(N)` path is acceptable when explicitly identified and bounded by `SCHED_MAX_TASKS`; the proof does not require all scheduler operations to be `O(1)`.

# Critical-section bound

Every section that masks or otherwise prevents normal interrupt preemption SHALL:

1. contain no task/application callback;
2. contain no wait for hardware;
3. contain no unbounded retry;
4. have a statically identifiable maximum iteration count; and
5. be separately measurable on the target.

The worst such duration becomes an input to latency analysis and [P6](P6-rms-schedulability.md).

# Fault-record bound

Crash/fault capture SHALL use a fixed-size scheduler-owned record or application-provided fixed storage.

Fault capture SHALL NOT allocate memory and SHALL NOT depend on the crashed task continuing to execute correctly.

A minimal record may include:

* magic/version;
* scheduler fault reason;
* active task ID;
* timestamp;
* PSP and MSP;
* stacked PC/LR/xPSR when architecturally available; and
* selected scheduler state useful for post-reset diagnosis.

Export through UART, flash, or another transport is outside the scheduler core.

# Architecture test

The project SHALL provide:

* a map-file or equivalent SRAM report separating kernel state, MSP reservation, and application task stacks;
* a default-configuration check for `SCHED_MAX_TASKS=32`;
* static or review-generated identification of every loop in scheduler code and its bound;
* static or runtime instrumentation of every critical section;
* measured context-switch and scheduler-path timing on the target MCU; and
* fault-injection tests proving fixed-size crash capture.

The default scheduler-owned SRAM SHOULD remain at or below the 1.5 KiB soft target, excluding application-owned task stacks.

# Dependencies

Depends on [P0](P0-static-configuration-validity.md).

Its timing bounds are inputs to [P6](P6-rms-schedulability.md).
