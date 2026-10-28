---
type: Contract
title: "C5 - Fault, crash record, and runtime profiling"
description: "Concrete fixed-size crash record, diagnostic snapshot API, and lightweight runtime accounting."
tags: [architecture, contract, fault, diagnostics, profiling]
status: draft
---

# Key goals

* Capture fatal evidence without dynamic allocation or transport dependencies.
* Provide the required runtime profiling: cumulative CPU time and observed maximum contiguous run.
* Keep diagnostic storage bounded and SRAM-visible.
* Distinguish observed runtime from proven WCET.

# Public profiling snapshot

```c
typedef struct {
    uint32_t total_cpu_ticks;
    uint32_t max_run_ticks;

    uint8_t  missed_release_count;
    uint8_t  age_credit;
    uint8_t  state;
    uint8_t  flags;
} arghtos_task_profile_t;

_Static_assert(sizeof(arghtos_task_profile_t) == 12u,
               "profile ABI changed");
```

# Public profiling API

```c
arghtos_result_t arghtos_profile_get(
    arghtos_task_id_t task_id,
    arghtos_task_profile_t *out);
```

Call context: TASK only.

The snapshot is copied atomically using one bounded critical section.

# Runtime accounting

C9 owns one global `run_start` timestamp because at most one task is RUNNING.

On dispatch:

```text
kernel.run_start = now
```

On deschedule:

```text
elapsed = now - kernel.run_start
task.total_cpu_ticks += elapsed
task.max_run_ticks = max(task.max_run_ticks, elapsed)
```

`total_cpu_ticks` wraps modulo 2^32.

`max_run_ticks` is an observed maximum contiguous scheduler run. It is not a proof of job WCET.

The profiling contract requires each uninterrupted RUNNING residency to be shorter than one full 2^32-tick clock cycle. Periodic tasks satisfy a much tighter bound through P6; async workers are expected to yield, wait, sleep, or be preempted before a full clock cycle elapses.

# Periodic missed-release count

When a periodic release is missed while the prior job remains active, `missed_release_count` is increased by the number of missed releases, saturating at `UINT8_MAX`.

The count may therefore advance by more than one in one delayed timer service.

# Async age credit

The snapshot exposes C8 `age_credit` only as diagnostic state. C5 does not interpret it as time.

# Generic port crash payload

The public crash record is architecture-neutral. The kernel does not expose or interpret a Cortex exception-frame type.

Crash flags are generic:

```c
#define ARGHTOS_CRASH_F_TASK_VALID       (1u << 0)
#define ARGHTOS_CRASH_F_PORT_STATE_VALID (1u << 1)
#define ARGHTOS_CRASH_F_RESERVED         (0xFCu)
```

The target port owns the meaning of `port_format`, `port_words`, and `port_state[]` below.

# Fault reasons

```c
typedef uint16_t arghtos_fault_reason_t;

enum {
    ARGHTOS_FAULT_NONE             = 0x0000u,
    ARGHTOS_FAULT_KERNEL_INVARIANT = 0x0001u,
    ARGHTOS_FAULT_BAD_SERVICE      = 0x0002u,
    ARGHTOS_FAULT_CONTEXT_CORRUPT  = 0x0003u,
    ARGHTOS_FAULT_ASYNC_RETURNED   = 0x0004u,
    ARGHTOS_FAULT_STACK_RANGE      = 0x0005u,
    ARGHTOS_FAULT_BAD_DISPATCH     = 0x0006u,
    ARGHTOS_FAULT_CPU_EXCEPTION    = 0x0100u
};
```

# Required architecture-neutral 64-byte crash record

```c
#define ARGHTOS_CRASH_MAGIC        0x41524748u
#define ARGHTOS_CRASH_COMMIT_MAGIC 0x43524153u
#define ARGHTOS_CRASH_VERSION      1u

typedef struct arghtos_crash_record {
    uint32_t magic;
    uint16_t version;
    uint16_t size;

    uint16_t reason;
    uint8_t  task_id;
    uint8_t  flags;

    uint32_t timestamp;

    uint16_t port_format;
    uint8_t  port_words;
    uint8_t  reserved;

    uint32_t port_state[10];

    uint32_t commit_magic;
} arghtos_crash_record_t;

_Static_assert(sizeof(arghtos_crash_record_t) == 64u,
               "crash record ABI changed");
```

`port_words` is in the range 0..10. The portable kernel zeros the port payload before asking the selected port to fill it. A target port may define one or more `port_format` identifiers in its own port documentation.

# Crash commit protocol

`commit_magic` is cleared first and written valid last.

Before the final commit-marker write, portable C executes a C11 release fence. The Cortex-M23 reference compiler lowers this to a DMB, ensuring prior crash-record stores are ordered before the validity marker.

A record is valid only if magic, version, size, and commit marker all match.

# Fatal kernel entry

Portable kernel faults use:

```c
_Noreturn void arghtos_fatal(
    arghtos_fault_reason_t reason,
    const void *port_fault_context);
```

The `port_fault_context` pointer is opaque to portable code. It may be NULL for a kernel-detected invariant fault.

The fatal path:

1. sets the C9 fault-active guard;
2. prevents normal scheduler mutation;
3. if the fault is attributable to the current task, marks that task FAULTED;
4. writes generic crash header/task/time fields;
5. zeros the generic port payload;
6. calls `arghtos_port_crash_fill(record, port_fault_context)`;
7. writes the crash commit marker last;
8. calls the terminal port hook.

The portable kernel never dereferences `port_fault_context`.

# Terminal hook

```c
_Noreturn void arghtos_port_fault_terminal(void);
```

It must not return, allocate, call normal scheduler APIs, or depend on the failed task stack.

# Resident-memory accounting

The required 64-byte crash record counts against the resident SRAM budget.

Code implementing diagnostics, fault capture, constants, and literal data also counts because ArghtOS code is SRAM-resident.

# Dependencies

[C0](C0-static-task-configuration.md), [C7](C7-common-types-abi.md), [C8](C8-task-control-record.md), [C9](C9-kernel-global-state.md), [P6](../../proof/P6-rms-schedulability.md), [P7](../../proof/P7-bounded-memory-and-work.md).

# Dangerous aspects

* Treating observed max runtime as WCET.
* Fault recursion.
* Calling transport/formatting code from the fatal path.
* Forgetting crash/fault code size in the resident-footprint total.

# Boundary specification

C5 captures architecture-neutral evidence. Target register/frame extraction belongs entirely to the selected port. Export, formatting, telemetry, and post-reset policy are external.

# R3 minimum-footprint production profile

The generic/debug profile validates profile call context, output pointer, and task ID.

The R3 production profile assumes:

* `arghtos_profile_get()` is called from task context;
* `out` is non-NULL;
* `task_id` identifies a configured task.

The atomic snapshot operation and returned field semantics are unchanged for valid calls.

Fatal-path integrity checks are not removed by R3.
