---
type: Contract
title: "C6 - Scheduling and dispatch policy"
description: "Concrete protected RMS selector, async aging rule, deterministic ties, and kernel dispatch transaction."
tags: [architecture, contract, scheduling, rms, aging, priority, dispatch]
status: draft
---

# Key goals

* Preserve fixed rate-monotonic ordering for periodic tasks.
* Give asynchronous tasks residual CPU capacity only.
* Use bounded aging to prevent indefinite async priority starvation when async dispatch opportunities exist.
* Make all tie behavior deterministic.
* Keep selection bounded by `ARGHTOS_MAX_TASKS`.
* Keep policy independent of Cortex-M context mechanics.

# Internal selector API

```c
arghtos_task_id_t arghtos_select_next(arghtos_time_t now);
```

Call context: KERNEL only, with scheduler state serialized.

The function returns `ARGHTOS_TASK_ID_INVALID` when no task is READY.

It does not mutate task lifecycle state.

# Top-level class rule

Selection is lexicographic by class:

```text
if any periodic task is READY:
    select periodic
else if any async task is READY:
    select async
else:
    select none
```

Periodic and asynchronous numeric priorities are never compared.

# Periodic RMS ordering

For READY periodic tasks `a` and `b`:

```text
a outranks b if:
    period_a < period_b
or:
    period_a == period_b and task_id_a < task_id_b
```

Period is immutable C0 configuration.

Deadline does not alter v1 priority assignment. It is consumed by P6 response-time acceptance.

# Async base priority

Base priority is C0 `async_priority`.

Larger numeric values are more urgent.

```text
0 <= base < ARGHTOS_ASYNC_PRIORITY_LEVELS
```

# Async aging

Aging applies only while an async task is READY.

For READY async task `t`:

```c
uint32_t age_ticks = now - t.ready_since;
uint32_t age_steps = age_ticks / ARGHTOS_AGING_QUANTUM_TICKS;
uint32_t ceiling = ARGHTOS_ASYNC_PRIORITY_LEVELS - 1u;

effective = min((uint32_t)base + age_steps, ceiling);
```

The result is clamped before conversion back to `arghtos_priority_t`.

Aging never changes C0 base priority.

# Async tie order

Among READY async tasks, compare in this order:

1. larger `effective` priority wins;
2. if equal, larger elapsed READY age `now - ready_since` wins;
3. if still equal, smaller task ID wins.

This gives deterministic FIFO-like preference among equal/saturated priorities without maintaining a queue.

# ready_since update rule

For an async task, `ready_since = now` whenever it newly enters READY from:

* DORMANT by wake;
* WAITING by wake;
* SLEEPING by timer release;
* RUNNING by yield;
* RUNNING by preemption.

A READY task that receives another idempotent wake does not reset `ready_since`.

Therefore repeated wake calls cannot erase accrued aging.

# Starvation statement

C6 guarantees no absolute CPU-time bound for asynchronous tasks because the periodic domain may consume all processor capacity.

Conditional guarantee:

> If asynchronous dispatch opportunities continue to occur, a continuously READY async task eventually reaches the async priority ceiling and receives oldest-ready tie preference over async tasks that became READY later.

This guarantee does not cross the periodic-domain boundary.

# Bounded selector

The reference implementation performs at most one bounded scan over `task_count <= ARGHTOS_MAX_TASKS`.

A two-pass implementation is also permitted if its measured bound is included in P6/P7.

No dynamic ready queue is required by the contract.

# Concrete kernel dispatch transaction

C4 calls:

```c
uintptr_t arghtos_kernel_dispatch(uintptr_t outgoing_saved_psp);
```

The kernel performs this serialized transaction:

```text
now = port_time_now()

if outgoing_saved_psp != 0 and current_task != INVALID:
    current.saved_psp = outgoing_saved_psp
    account elapsed runtime

    if current.state == RUNNING:
        # dispatch was requested by preemption/reconsideration,
        # not by an earlier block/sleep/yield transition
        current.state = READY
        if current is async:
            current.ready_since = now
        saturating_increment(current.preempt_count)

next = arghtos_select_next(now)

if next == INVALID:
    current_task = INVALID
    return 0

assert next.state == READY
next.state = RUNNING
next.run_start = now
saturating_increment(next.dispatch_count)
current_task = next
return next.saved_psp
```

Lifecycle APIs such as wait/yield/sleep may update the outgoing task state before PendSV. In those cases the `state == RUNNING` test prevents the dispatch path from incorrectly classifying the transition as a preemption.

# Preemption behavior

A scheduling-relevant event requests dispatch when it may change the winner.

Examples:

* a periodic release;
* an async wake;
* current task yield/wait/sleep/return;
* scheduler start.

PendSV re-evaluates the full policy. If the outgoing task remains the winner, restoring the same saved PSP is legal.

# Periodic overrun interaction

C6 never queues another periodic instance.

When B3 detects a release while the prior job state is not inactive/DORMANT, it increments the overrun counter and leaves the existing job as the only active instance.

The configured fault/diagnostic policy may report this, but selection continues using the existing task state.

# Complexity contract

For `N = task_count`:

```text
selection work = O(N)
0 <= N <= ARGHTOS_MAX_TASKS
```

There is no operation proportional to historical wake count, number of past releases, or elapsed runtime.

Measured worst-case selector cost at `ARGHTOS_MAX_TASKS` is an input to P6.

# Dependencies

[C0](C0-static-task-configuration.md), [C7](C7-common-types-abi.md), [C8](C8-task-control-record.md), [P1](../../proof/P1-task-state-invariants.md), [P5](../../proof/P5-priority-domain-isolation.md), [P6](../../proof/P6-rms-schedulability.md), [P7](../../proof/P7-bounded-memory-and-work.md).

# Dangerous aspects

* Resetting `ready_since` on an idempotent wake.
* Aging WAITING tasks instead of READY tasks.
* Comparing async numbers directly with periodic priority.
* Incrementing preemption count for a voluntary wait/yield/sleep.
* Using deadline-monotonic ordering while claiming RMS.
* Adding hidden queue/history work to selection.

# Boundary specification

C6 owns only winner selection and the serialized dispatch transaction. C3 owns when tasks become due, C1/C2 own lifecycle requests, C4 owns processor switching, and C5 owns diagnostic interpretation.
