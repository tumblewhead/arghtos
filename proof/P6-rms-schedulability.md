---
type: Proof
title: "P6 - RMS schedulability"
description: "Synchronous D=T periodic tasks meet deadlines when fixed-priority response-time analysis succeeds with bounded release jitter, kernel overhead, and interrupt interference."
tags: [scheduler, cortex-m23, proof, rms, schedulability, real-time]
status: draft
---

# Statement

For every configured periodic task `i`, if the response-time recurrence converges to `R_i <= T_i`, then the task meets its v1 relative deadline `D_i=T_i` under the stated assumptions.

# V1 periodic model

Each periodic task has:

* `T_i`: compile-time period;
* `D_i = T_i`;
* phase = 0;
* `C_i`: independently justified WCET for one bounded application job;
* fixed priority assigned by rate-monotonic ordering.

Periodic tasks do not call wait or sleep. Yield is allowed but must be included conservatively in kernel overhead if used.

Runtime `max_run_ticks` is diagnostic only and is not a WCET proof.

# Release jitter

Let `J_i` bound lateness between an ideal periodic release and the scheduler making that job READY.

`J_i` must conservatively include:

* platform timer delivery jitter;
* higher-priority handler interference before the timer/release path runs;
* bounded timer-handler release-processing work.

C3's drift-free epoch arithmetic means this jitter does not accumulate from period to period.

# Interference model

Let:

* `hp(i)`: periodic tasks with shorter period, plus deterministic equal-period predecessors;
* `B_i`: explicit blocking term, target zero in v1 because there are no scheduler mutex/semaphore primitives;
* `K_i(R)`: bounded scheduler/SVC/PendSV/context overhead in a window of length `R`;
* `I_i(R)`: bounded non-scheduler hardware-handler interference in the window.

By P5, async application execution never runs ahead of READY periodic work. Async work may contribute only bounded transition/dispatch overhead already represented in `K_i`.

# Response-time recurrence

A conservative fixed-priority recurrence is:

```text
R_i(0) = C_i + B_i + J_i

R_i(n+1) =
    C_i
  + B_i
  + J_i
  + sum over j in hp(i):
        ceil((R_i(n) + J_j) / T_j) * C_j
  + K_i(R_i(n))
  + I_i(R_i(n))
```

Iterate until:

* a fixed point is reached and `R_i <= T_i`; or
* `R_i > T_i`, in which case schedulability is not proven.

A more conservative accepted analysis is permitted if it upper-bounds the same effects.

# Why async execution is excluded

Suppose a periodic release makes any periodic task READY while an async task is RUNNING.

The release path requests dispatch.

After bounded handler/kernel latency, P5 class-first selection chooses periodic work before any async task.

Thus async application execution after the ideal periodic release is bounded only by release/dispatch latency already represented in `J_i`, `K_i`, and `I_i`, not by the async task's application execution requirement.

# Periodic overrun

C3 never creates multiple simultaneous instances.

An observed release while the prior job remains active increments the missed-release count.

Any such overrun is runtime evidence that the assumed timing envelope was violated or the task set was not operating within its proven conditions.

# Build-time acceptance

A build claiming periodic schedulability SHALL have deterministic analysis inputs for:

* every `T_i`;
* every `C_i`;
* timer jitter/release jitter bounds;
* kernel/context overhead bounds;
* allowed hardware-handler interference bounds.

The analysis SHALL reject or refuse a schedulability claim if any required bound is absent, arithmetic overflows, or any `R_i > T_i`.

# Conditional status

The theorem and recurrence are internally consistent with C0/C3/C5/C6.

The proof remains conditional until actual implementation timing bounds and application WCET inputs exist.

# Dependencies

Depends on [P4](P4-time-release-correctness.md), [P5](P5-priority-domain-isolation.md), and [P7](P7-bounded-memory-and-work.md).

# R5p generated RMS-order refinement

C0 may encode periodic RMS order directly in generated task IDs for the trusted
minimum-size profile. If periodic tasks occupy a prefix sorted by
nondecreasing period, with deterministic increasing-ID order for equal periods,
then selecting the first READY periodic task in that prefix is exactly
equivalent to the `hp(i)` ordering used by this proof.

This representation refinement does not change any `T_i`, `C_i`, deadline,
release, jitter, or interference term. It only removes runtime reconstruction
of an ordering already proved by the generator.

# R6v unit-tick dispatch overhead

R6v may execute one redundant PendSV/dispatcher pass on a unit timer tick that
does not create new READY work, for example while a period-1 job is already
active.

This is a bounded runtime-cost trade, not a free timing transformation.
Build-time response-time analysis selecting R6v SHALL include at most one
additional bounded PendSV/dispatch cost per scheduler timer tick in `K_i(R)`.

No application execution priority changes; the additional work is scheduler
overhead already represented by the P6 kernel-overhead term.

# R6ad unconditional wake-dispatch overhead

R6ad may execute one redundant PendSV/dispatcher pass for a wake that did not
change target eligibility, for example a repeated wake while the sole async
worker is READY, RUNNING, or SLEEPING.

This does not alter fixed-priority selection or allow async application work to
run ahead of READY periodic work. It adds only bounded kernel overhead.

Any schedulability analysis selecting R6ad SHALL include the maximum rate and
cost of these additional wake-triggered PendSV/dispatch passes in the
appropriate `K_i(R)` / interrupt-overhead bound.

No application WCET or priority relation changes.
