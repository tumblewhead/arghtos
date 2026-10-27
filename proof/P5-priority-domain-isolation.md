---
type: Proof
title: "P5 - Priority-domain isolation"
description: "Asynchronous task aging can prevent starvation without crossing into the protected RMS periodic priority domain."
tags: [scheduler, cortex-m23, proof, rms, priority, aging]
generated:
  by: openai/gpt-5.6-sol
  at: 2026-10-27T20:08:00Z
status: draft
---

# Statement

Aging of asynchronous tasks cannot change the dispatch order among protected periodic RMS tasks and cannot make an asynchronous task outrank a READY periodic task.

# Priority domains

The scheduler defines two disjoint software-priority domains:

1. **Periodic protected domain**: compile-time periodic tasks ordered by fixed RMS priority. Shorter period means higher priority; a compile-time deterministic tie-breaker resolves equal periods.
2. **Asynchronous domain**: event-driven tasks with base priority plus bounded aging.

Let `P_floor` be the least urgent priority value in the protected periodic domain under the implementation's chosen ordering convention.

For every asynchronous task `a` and every representable age `x`:

`effective_priority(a, x)` SHALL remain strictly outside the protected periodic domain.

# Proof

RMS priority for every periodic task is immutable after configuration.

Aging modifies only the asynchronous effective priority and is clamped at the asynchronous-domain ceiling.

Because that ceiling does not cross `P_floor`, no aging function can construct an asynchronous effective priority that outranks a READY periodic task.

Therefore:

* asynchronous aging cannot reorder periodic tasks;
* asynchronous aging cannot invalidate the periodic fixed-priority ordering; and
* asynchronous tasks execute only when no protected periodic task with higher scheduling entitlement is READY.

# Starvation scope

This proof does not promise that asynchronous work receives CPU time in an overloaded periodic system. Aging prevents starvation only among capacity available to the asynchronous domain.

# Architecture test

For all representable asynchronous base priorities and age values, exhaustive or property-based tests SHALL assert:

`async_effective_priority < periodic_domain`

using the concrete implementation's comparison convention.

Tests SHALL also assert that changing asynchronous age never changes the relative ordering of two periodic tasks.

# Dependencies

[P6](P6-rms-schedulability.md) relies on this proof to exclude asynchronous execution from periodic higher-priority interference.
