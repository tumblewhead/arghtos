---
type: Contract
title: "C8 - Task control record"
description: "Concrete 20-byte private per-task control object for continuation, timing, aging, state, and runtime profiling."
tags: [architecture, contract, task-control, memory, profiling]
status: draft
---

# Key goals

* Keep mutable scheduler state compact because it is always SRAM-resident.
* Preserve the continuation and timed-release information required by the scheduler.
* Preserve the required profiling quantities: cumulative CPU time and observed maximum contiguous run.
* Implement starvation-avoidance aging without depending on wall-clock wraparound.

# Concrete 20-byte runtime record

```c
typedef struct {
    uintptr_t          saved_psp;       /* 4 B */
    arghtos_time_t     next_time;       /* 4 B: periodic release or sleep deadline */
    uint32_t           total_cpu_ticks; /* 4 B: modulo-2^32 */
    uint32_t           max_run_ticks;   /* 4 B: observed maximum contiguous run */

    uint8_t            overrun_count;   /* saturating periodic release misses */
    uint8_t            age_credit;      /* async dispatch-opportunity aging */
    uint8_t            state;           /* arghtos_task_state_t */
    uint8_t            flags;
} arghtos_task_control_t;

_Static_assert(sizeof(uintptr_t) == 4u,
               "Cortex-M23 requires 32-bit uintptr_t");
_Static_assert(sizeof(arghtos_task_control_t) == 20u,
               "task control resident-footprint contract changed");
```

At `ARGHTOS_MAX_TASKS=32`:

```text
32 * 20 B = 640 B
```

# Flag bits

```c
#define ARGHTOS_TCF_WAKE_PENDING (1u << 0)
#define ARGHTOS_TCF_RESERVED     (0xFEu)
```

The wake bit is the one-bit handshake required by P3. It is not an event counter.

# Field meaning

| Field | Meaning |
|---|---|
| `saved_psp` | task continuation |
| `next_time` | periodic next release, or async sleep deadline |
| `total_cpu_ticks` | cumulative CPU execution time, modulo 2^32 |
| `max_run_ticks` | largest observed contiguous RUNNING interval |
| `overrun_count` | saturating periodic missed-release count |
| `age_credit` | saturating async starvation-prevention credit |
| `state` | abstract C7 task state |
| `flags` | wake handshake and reserved bits |

# Runtime start time is global

Only one task can be RUNNING.

Therefore the timestamp at which the current task began its present RUNNING interval is stored once in C9 as `run_start`, not once per task.

On deschedule:

```text
elapsed = now - kernel.run_start
control.total_cpu_ticks += elapsed
control.max_run_ticks = max(control.max_run_ticks, elapsed)
```

This removes four bytes per task without losing any observable profiling quantity.

# Age-credit semantics

`age_credit` is meaningful only for READY asynchronous tasks.

It measures missed **asynchronous dispatch opportunities**, not elapsed clock time.

Rules:

* DORMANT/WAITING/SLEEPING -> READY: `age_credit = 0`.
* RUNNING -> READY by yield or preemption: `age_credit = 0`.
* An idempotent wake of an already READY task does not reset credit.
* Whenever no periodic task is READY and the selector is choosing among READY async tasks, each READY async candidate receives one saturating age increment before comparison.
* The selected async task resets `age_credit = 0` when it becomes RUNNING.
* Credit saturates at `UINT8_MAX`.

This is independent of the 32-bit scheduler clock and remains valid across arbitrary clock wraparound.

# Effective async priority

C6 derives effective urgency from:

```text
base_priority + min(age_credit, async_priority_ceiling)
```

clamped at the async priority ceiling.

Credit above the number needed to reach the priority ceiling remains useful as a tie-breaker among saturated tasks.

# Counter behavior

`overrun_count` saturates at `UINT8_MAX`.

`total_cpu_ticks` intentionally wraps modulo 2^32. It is a compact cumulative diagnostic, not a lifetime billing quantity.

`max_run_ticks` is valid because one contiguous RUNNING interval must be shorter than `ARGHTOS_TIME_HALF_RANGE`.

# Continuation semantics

`saved_psp` is the sole scheduler continuation pointer.

Application continuations/state machines are application data.

# Atomicity

Updates that change a task's state and its related wake/age fields are one serialized scheduler transaction.

Saving `saved_psp` and relinquishing CPU ownership is one serialized dispatch transaction.

# Dependencies

[C0](C0-static-task-configuration.md), [C1](C1-task-api-lifecycle.md), [C6](C6-scheduling-policy.md), [C7](C7-common-types-abi.md), [C9](C9-kernel-global-state.md), [P0](../../proof/P0-static-configuration-validity.md), [P1](../../proof/P1-task-state-invariants.md), [P7](../../proof/P7-bounded-memory-and-work.md).

# Dangerous aspects

* Treating age credit as elapsed time.
* Resetting age credit on an idempotent READY wake.
* Adding per-task profiling counters without rechecking the resident-footprint budget.
* More than one code path owning `saved_psp`.

# Boundary specification

C8 is private ArghtOS SRAM and always counts against the resident-footprint budget.
