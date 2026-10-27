---
type: Proof
title: "P6 - RMS schedulability"
description: "Each compile-time periodic task meets its deadline when fixed-priority response-time analysis succeeds with bounded kernel and ISR interference."
tags: [scheduler, cortex-m23, proof, rms, schedulability, real-time]
generated:
  by: openai/gpt-5.6-sol
  at: 2026-10-27T20:08:00Z
status: draft
---

# Statement

For every compile-time periodic task `i`, if fixed-priority response-time analysis converges to a response time `R_i <= D_i`, then task `i` meets its relative deadline under the scheduler assumptions recorded here.

# Task parameters

Each periodic task has compile-time parameters:

* `T_i`: period;
* `D_i`: relative deadline, with default `D_i = T_i`;
* `C_i`: proven or conservatively supplied WCET for one bounded job;
* `J_i`: release jitter bound, default zero unless the architecture introduces one; and
* fixed priority assigned by rate-monotonic ordering.

Runtime profiling may record an observed maximum execution time, but an observed maximum is not itself a proof of WCET.

# Interference model

Let:

* `hp(i)` be periodic tasks with higher RMS priority than `i`;
* `B_i` be any explicitly bounded blocking term. Version 1 intends no scheduler mutex/semaphore blocking, so the target value is zero;
* `K_i(R)` be bounded scheduler/context-switch overhead occurring within a window of length `R`; and
* `I_i(R)` be bounded hardware ISR interference within that window.

By [P5](P5-priority-domain-isolation.md), asynchronous task execution does not contribute higher-priority task interference while periodic work is READY.

# Response-time equation

The proof SHALL use exact fixed-priority response-time analysis or a more conservative accepted bound.

A suitable recurrence is:

`R_i^(0) = C_i + B_i`

`R_i^(n+1) = C_i + B_i + sum_{j in hp(i)} ceil((R_i^n + J_j) / T_j) * C_j + K_i(R_i^n) + I_i(R_i^n)`

Iterate until either:

* `R_i^(n+1) = R_i^n`, in which case the task is schedulable if `R_i <= D_i`; or
* `R_i > D_i`, in which case the configured task set is rejected as not proven schedulable.

# Proof

Under fixed-priority preemptive scheduling, a job of task `i` can be delayed only by:

1. its own execution;
2. bounded blocking admitted by the model;
3. releases of higher-priority periodic jobs;
4. bounded kernel/context-switch work; and
5. bounded interrupt service.

The recurrence upper-bounds each of those terms in the busy window.

[P5](P5-priority-domain-isolation.md) excludes asynchronous tasks from higher-priority interference.

Therefore a fixed point `R_i <= D_i` is an upper bound on job completion relative to release, proving the deadline.

# Compile-time / build-time acceptance test

The project SHALL provide a deterministic analysis tool or compile-generated input that evaluates every configured periodic task.

A build intended to claim schedulability SHALL fail or clearly refuse the claim if:

* any required `C_i`, `T_i`, or `D_i` is absent;
* a timing bound used for kernel or ISR interference is absent;
* the recurrence exceeds a deadline; or
* arithmetic overflows the analysis representation.

The generated result SHOULD be retained as evidence.

# Conditional status

This proof is presently conditional because task parameters and measured/proven kernel and ISR bounds do not yet exist. The architecture SHALL be designed so these quantities can be supplied without changing scheduling semantics.

# Dependencies

Depends on [P4](P4-time-release-correctness.md), [P5](P5-priority-domain-isolation.md), and [P7](P7-bounded-memory-and-work.md).
