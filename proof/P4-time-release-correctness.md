---
type: Proof
title: "P4 - Time and release correctness"
description: "Tickless absolute-time scheduling preserves due-event ordering and remains correct across bounded counter wraparound."
tags: [scheduler, cortex-m23, proof, tickless, timer, wraparound]
generated:
  by: openai/gpt-5.6-sol
  at: 2026-10-27T20:08:00Z
status: draft
---

# Statement

Using a fixed-width unsigned monotonic scheduler clock, absolute release times can be compared correctly across counter wraparound provided every legal interval is strictly less than half the counter range.

# Definitions

For a `w`-bit unsigned clock, define:

`HALF_RANGE = 2^(w-1)`.

Define `due(now, deadline)` by interpreting `now - deadline` as a signed `w`-bit difference:

`due(now, deadline) := signed(now - deadline) >= 0`.

The configuration contract SHALL require every sleep, period, relative deadline, and timer-programming horizon to satisfy:

`0 <= interval < HALF_RANGE`.

# Proof

Unsigned subtraction is modulo `2^w`.

When two relevant timestamps differ by less than `HALF_RANGE`, exactly one direction around the modular ring is the valid temporal direction. Interpreting the modular difference as a signed `w`-bit value therefore distinguishes "before" from "at or after" unambiguously within the allowed horizon.

Consequently the due predicate remains valid when the counter wraps between the release timestamp and the current timestamp.

A sleeping task becomes READY if and only if its release time is due according to this predicate.

# Periodic release rule

A periodic task represents one bounded job per release.

The scheduler SHALL NOT create two simultaneous job instances for one periodic task.

If the next release becomes due while the previous job is still active, the scheduler SHALL record the configured overrun/deadline-miss condition rather than allocate or queue another job instance.

# Tickless property

The scheduler stores absolute next-event times and asks the platform timer to interrupt at the earliest required release, subject to the platform timer's maximum programmable horizon.

A timer interrupt may release one or more tasks whose due predicates are true. The scheduler need not execute periodic polling ticks between releases.

# Architecture test

Tests SHALL cover:

* equality at the release time;
* one tick before and after release;
* wraparound on both sides of zero;
* the largest legal interval;
* rejection of intervals at or above `HALF_RANGE`;
* several tasks becoming due at one timestamp;
* timer reprogramming when a newly scheduled wake is earlier; and
* a periodic overrun at the next release.

# Dependencies

The `SLEEPING -> READY` transition is consumed by [P1](P1-task-state-invariants.md).

Timing overhead introduced by release processing is bounded by [P7](P7-bounded-memory-and-work.md) and included in [P6](P6-rms-schedulability.md).
