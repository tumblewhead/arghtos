---
type: Contract
title: "C6 - Scheduling and dispatch policy"
description: "Concrete protected RMS selector, opportunity-based async aging, deterministic ties, and kernel dispatch transaction."
tags: [architecture, contract, scheduling, rms, aging, priority, dispatch]
status: draft
---

# Key goals

* Preserve fixed rate-monotonic ordering for periodic tasks.
* Give asynchronous tasks residual CPU capacity only.
* Prevent starvation among async tasks when async dispatch opportunities continue.
* Avoid time-wrap dependence in the aging mechanism.
* Keep selection bounded by `ARGHTOS_MAX_TASKS`.

# Internal selector API

```c
arghtos_task_id_t arghtos_select_next(void);
```

Call context: KERNEL only, with scheduler state serialized.

The function returns `ARGHTOS_TASK_ID_INVALID` when no task is READY.

# Top-level class rule

Selection is lexicographic by task class:

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

V1 has synchronous releases and `D=T`.

# Async base priority

Larger numeric values are more urgent:

```text
0 <= base < ARGHTOS_ASYNC_PRIORITY_LEVELS
```

# Opportunity-based async aging

Aging occurs only when:

1. no periodic task is READY; and
2. at least one async task is READY.

That event is an **async dispatch opportunity**.

Before comparing READY async candidates, each receives:

```c
if (age_credit != UINT8_MAX) {
    age_credit++;
}
```

Effective priority is:

```c
uint16_t raised = (uint16_t)base + (uint16_t)age_credit;
uint8_t ceiling = ARGHTOS_ASYNC_PRIORITY_LEVELS - 1u;
effective = (raised > ceiling) ? ceiling : (uint8_t)raised;
```

The selected task resets `age_credit=0` when it becomes RUNNING.

# Async tie order

Among READY async tasks, compare:

1. larger effective priority;
2. larger `age_credit`;
3. smaller task ID.

The second comparison preserves waiting order after several tasks have saturated at the priority ceiling.

# Starvation statement

C6 does not promise async CPU time while periodic work consumes all available processor capacity.

Conditional guarantee:

> If async dispatch opportunities continue to occur, every continuously READY async task is eventually selected.

Reason: a nonselected READY async task's credit increases monotonically to saturation, while a selected task resets to zero. Thus a task cannot remain permanently below repeatedly selected peers.

# Wake and READY credit rules

* DORMANT/WAITING/SLEEPING -> READY: credit becomes zero.
* RUNNING -> READY by yield/preemption: credit becomes zero.
* wake of an already READY task does not reset credit.
* credit is ignored outside READY async state.

# Bounded selector

The reference selector uses bounded scans only.

A permitted implementation is:

1. one scan to find the best READY periodic task;
2. if none exists, one scan over READY async tasks that increments credit and selects the best candidate.

Thus selection work is at most `2 * task_count` task inspections.

No dynamic ready queue exists.

# Concrete kernel dispatch transaction

C4 calls:

```c
uintptr_t arghtos_kernel_dispatch(uintptr_t outgoing_saved_psp);
```

Serialized behavior:

```text
now = port_time_now()

if outgoing_saved_psp != 0 and current_task != INVALID:
    current.saved_psp = outgoing_saved_psp
    elapsed = now - kernel.run_start
    account total/max runtime

    if current.state == RUNNING:
        current.state = READY
        if current is async:
            current.age_credit = 0

next = arghtos_select_next()

if next == INVALID:
    current_task = INVALID
    return 0

assert next.state == READY
next.state = RUNNING
if next is async:
    next.age_credit = 0
kernel.run_start = now
current_task = next
return next.saved_psp
```

Lifecycle APIs may change the outgoing task state before PendSV. In those cases the `state == RUNNING` test prevents a voluntary block/yield from being reclassified as a preemption.

# Preemption behavior

A scheduling-relevant event requests dispatch when it may change the winner:

* periodic release;
* async wake;
* yield/wait/sleep/return;
* scheduler start.

PendSV re-evaluates policy.

Restoring the same task is legal if it remains the winner.

# Periodic overrun interaction

No additional periodic job instance is queued.

C3 records missed releases and advances the release epoch arithmetically.

# Complexity contract

For `N = task_count`:

```text
selector inspections <= 2*N
N <= ARGHTOS_MAX_TASKS
```

Measured worst-case selector cost is an input to P6.

# Dependencies

[C0](C0-static-task-configuration.md), [C8](C8-task-control-record.md), [C9](C9-kernel-global-state.md), [P1](../../proof/P1-task-state-invariants.md), [P5](../../proof/P5-priority-domain-isolation.md), [P6](../../proof/P6-rms-schedulability.md), [P7](../../proof/P7-bounded-memory-and-work.md), [P8](../../proof/P8-async-starvation-avoidance.md).

# Dangerous aspects

* Aging periodic tasks.
* Resetting async credit on an idempotent READY wake.
* Comparing async numeric priority with periodic priority.
* Aging on timer ticks instead of dispatch opportunities.
* Hidden unbounded queue/history work.

# Boundary specification

C6 owns winner selection and the serialized dispatch transaction. C3 owns timed release, C1/C2 own lifecycle requests, C4 owns processor switching, and C5 owns diagnostic presentation.
