---
type: Contract
title: "C8 - Task control record"
description: "Concrete 28-byte private per-task command/control object shared by runtime state, policy, execution, time, and diagnostics blocks."
tags: [architecture, contract, task-control, memory, profiling]
status: draft
---

# Key goals

* Give each task one compact mutable scheduler-owned command/control object.
* Carry continuation, lifecycle state, wake handshake, timing/aging state, and required profiling.
* Keep per-task SRAM predictable and small.
* Avoid duplicating immutable C0 configuration.

# Concrete runtime record

```c
typedef struct {
    uintptr_t          saved_psp;       /* continuation */
    arghtos_time_t     next_time;       /* periodic release or sleep deadline */
    arghtos_time_t     state_time;      /* READY age epoch OR RUNNING start time */

    uint32_t           total_cpu_ticks; /* modulo-2^32 accumulated CPU time */
    uint32_t           max_run_ticks;   /* largest observed contiguous run */

    uint16_t           dispatch_count;  /* saturating */
    uint16_t           preempt_count;   /* saturating */
    uint16_t           overrun_count;   /* saturating periodic release misses */

    uint8_t            state;           /* arghtos_task_state_t */
    uint8_t            flags;
} arghtos_task_control_t;
```

On the required 32-bit Cortex-M23 ABI:

```c
_Static_assert(sizeof(uintptr_t) == 4u, "Cortex-M23 requires 32-bit uintptr_t");
_Static_assert(sizeof(arghtos_task_control_t) == 28u,
               "task control record SRAM contract changed");
```

At the default `ARGHTOS_MAX_TASKS=32`:

```text
32 * 28 = 896 bytes
```

This is mandatory scheduler SRAM.

# Why state_time is shared

READY-aging time and RUNNING-start time are never simultaneously needed.

`state_time` therefore has state-dependent meaning:

| State/class | state_time meaning |
|---|---|
| READY async | timestamp at which current READY residency began |
| RUNNING any | timestamp at which current contiguous run began |
| all other cases | unspecified / ignored |

The transition READY -> RUNNING consumes the aging timestamp and replaces it with the run-start timestamp.

The transition RUNNING -> READY for an async task consumes the run-start timestamp during accounting and replaces it with the new READY epoch.

This saves four bytes per task without losing information required by the contracts.

# Flag bits

```c
#define ARGHTOS_TCF_WAKE_PENDING     (1u << 0)
#define ARGHTOS_TCF_BUDGET_EXCEEDED  (1u << 1)
#define ARGHTOS_TCF_RESERVED_MASK    (0xFCu)
```

Reserved bits remain zero in v1.

# Field ownership

| Field | Primary writer | Readers |
|---|---|---|
| saved_psp | B4 execution path | B4 |
| next_time | B3 time/release, C1 sleep | B3 |
| state_time | B1/B5 transition accounting | B2/B5 |
| total_cpu_ticks | B5 | C5 snapshot |
| max_run_ticks | B5 | C5 snapshot |
| dispatch_count | B5 | C5 snapshot |
| preempt_count | B5 | C5 snapshot |
| overrun_count | B3/B5 | C5 snapshot |
| state | B1 lifecycle core | scheduler blocks |
| flags | B1/B5 | B1/B5/C5 |

# next_time meaning

* periodic task: next absolute release epoch;
* SLEEPING async task: sleep deadline;
* otherwise: ignored.

# Counter behavior

All 16-bit counters saturate at `UINT16_MAX`; they never wrap.

`total_cpu_ticks` intentionally wraps modulo 2^32.

`max_run_ticks` is meaningful only because each contiguous run is required to be less than the scheduler half-range.

# Continuation semantics

`saved_psp` is the only scheduler continuation pointer.

Application state-machine continuations are application data and do not belong in the scheduler object.

# Priority semantics

Base async priority remains immutable in C0.

Effective priority is derived from C0 base priority and the READY-age represented by C8 `state_time`. It is not stored.

# Atomicity

Any update involving both `state` and the state-dependent meaning of `state_time` is one atomic scheduler transaction.

Examples:

* entering async READY and writing the READY epoch;
* entering RUNNING and replacing state_time with run-start time;
* consuming `wake_pending` while deciding whether WAITING is committed;
* saving PSP while relinquishing current CPU ownership.

# Dependencies

[C0](C0-static-task-configuration.md), [C1](C1-task-api-lifecycle.md), [C7](C7-common-types-abi.md), [P0](../../proof/P0-static-configuration-validity.md), [P1](../../proof/P1-task-state-invariants.md), [P7](../../proof/P7-bounded-memory-and-work.md).

# Dangerous aspects

* Reading `state_time` using the wrong state interpretation.
* Adding per-task fields without rechecking the SRAM target.
* Duplicating immutable descriptor data.
* More than one code path believing it owns `saved_psp`.
* Allowing saturating counters to wrap.

# Boundary specification

C8 is private scheduler SRAM. Application code receives C5 snapshots, never a pointer to this record.
