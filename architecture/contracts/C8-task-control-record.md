---
type: Contract
title: "C8 - Task control record"
description: "Concrete private per-task command/control object shared by runtime state, policy, execution, time, and diagnostics blocks."
tags: [architecture, contract, task-control, memory, profiling]
status: draft
---

# Key goals

* Give each task one compact mutable scheduler-owned command/control object.
* Carry the saved continuation, scheduling state, wake handshake, timing state, and basic runtime profiling.
* Keep per-task SRAM predictable and small.
* Avoid duplicating immutable configuration already present in C0.

# Concrete runtime record

```c
typedef struct {
    uintptr_t          saved_psp;       /* continuation; 0 only before frame creation */
    arghtos_time_t     next_time;       /* next periodic release or sleep deadline */
    arghtos_time_t     ready_since;     /* async READY aging epoch */
    arghtos_time_t     run_start;       /* most recent dispatch timestamp */

    uint32_t           total_cpu_ticks; /* modulo-2^32 accumulated task CPU time */
    uint32_t           max_run_ticks;   /* largest observed contiguous run */
    uint32_t           last_run_ticks;  /* most recent contiguous run */

    uint16_t           dispatch_count;  /* saturating */
    uint16_t           preempt_count;   /* saturating */
    uint16_t           overrun_count;   /* saturating periodic release misses */

    uint8_t            state;           /* arghtos_task_state_t */
    uint8_t            flags;
} arghtos_task_control_t;
```

On the required 32-bit Cortex-M23 ABI:

```c
_Static_assert(sizeof(uintptr_t) == 4u, "Cortex-M23 port requires 32-bit uintptr_t");
_Static_assert(sizeof(arghtos_task_control_t) == 36u,
               "task control record SRAM contract changed");
```

At the default `ARGHTOS_MAX_TASKS=32`, the task-control array occupies exactly:

```text
32 * 36 = 1152 bytes
```

Task stacks and immutable descriptors are not included in that number.

# Flag bits

```c
#define ARGHTOS_TCF_WAKE_PENDING     (1u << 0)
#define ARGHTOS_TCF_BUDGET_EXCEEDED  (1u << 1)
#define ARGHTOS_TCF_RESERVED_MASK    (0xFCu)
```

Reserved bits must remain zero in v1.

# Field ownership

| Field | Primary writer | Readers |
|---|---|---|
| saved_psp | B4 execution path | B4 |
| next_time | B3 time/release, C1 sleep | B3 |
| ready_since | lifecycle/release transition | B2 policy |
| run_start | B5 dispatch accounting | B5 |
| total_cpu_ticks | B5 | C5 snapshot |
| max_run_ticks | B5 | C5 snapshot |
| last_run_ticks | B5 | C5 snapshot |
| dispatch_count | B5 | C5 snapshot |
| preempt_count | B5 | C5 snapshot |
| overrun_count | B3/B5 | C5 snapshot |
| state | B1 lifecycle core | all scheduler blocks |
| flags | B1/B5 | B1/B5/C5 |

One logical owner must exist for each mutation path even if the implementation keeps these functions in one translation unit.

# State-dependent field meaning

* `next_time` is meaningful for periodic tasks and SLEEPING async tasks.
* `ready_since` is meaningful only for READY async tasks.
* `run_start` is meaningful only for RUNNING tasks.
* `wake_pending` is meaningful only for async tasks.
* `overrun_count` is meaningful primarily for periodic tasks.

Unused fields need not be zeroed continuously, but initialization and diagnostics must never interpret a field outside its valid state/class.

# Counter behavior

All 16-bit counters saturate at `UINT16_MAX`; they never wrap.

`total_cpu_ticks` intentionally wraps modulo 2^32. It is a lightweight cumulative diagnostic counter, not a lifetime billing counter.

`last_run_ticks` and `max_run_ticks` represent contiguous execution intervals and are valid only when each measured interval is less than half the scheduler clock range.

# Continuation semantics

`saved_psp` is the task continuation.

No second continuation pointer exists. Application continuations/state machines are application data, not scheduler state.

This directly satisfies the original continuation requirement without duplicating processor context.

# Priority semantics

Base priority is immutable C0 configuration and is not copied into C8.

Effective async priority is calculated from:

```text
C0.async_priority + age(C8.ready_since)
```

and is never stored persistently unless a future measured optimization justifies adding storage.

# Atomicity

Any update involving both `state` and another control field that determines the meaning of that state is one atomic scheduler transaction. Examples:

* setting `ready_since` while entering READY;
* setting `next_time` while entering SLEEPING;
* consuming `wake_pending` while deciding whether WAITING is committed;
* saving PSP while relinquishing current CPU ownership.

# Dependencies

[C0](C0-static-task-configuration.md), [C1](C1-task-api-lifecycle.md), [C7](C7-common-types-abi.md), [P0](../../proof/P0-static-configuration-validity.md), [P1](../../proof/P1-task-state-invariants.md), [P7](../../proof/P7-bounded-memory-and-work.md).

# Dangerous aspects

* Adding "just one more field" per task and silently breaking the SRAM target.
* Duplicating immutable descriptor data.
* Wrapping diagnostic counters that are expected to indicate "many."
* Reading state-dependent fields when they are not valid.
* More than one code path believing it owns `saved_psp`.

# Boundary specification

C8 is private scheduler SRAM. Application code receives diagnostic snapshots, never a pointer to this record.
