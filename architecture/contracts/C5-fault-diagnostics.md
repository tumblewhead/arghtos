---
type: Contract
title: "C5 - Fault, crash record, and runtime profiling"
description: "Concrete fixed-size crash record, diagnostic snapshot API, and runtime accounting rules."
tags: [architecture, contract, fault, diagnostics, profiling]
status: draft
---

# Key goals

* Capture fatal evidence without dynamic allocation or transport dependencies.
* Provide lightweight per-task runtime profiling.
* Distinguish observed execution maxima from proven WCET.
* Keep all diagnostic storage bounded and explicit.

# Public profiling snapshot

```c
typedef struct {
    uint32_t total_cpu_ticks;
    uint32_t max_run_ticks;
    uint32_t last_run_ticks;

    uint16_t dispatch_count;
    uint16_t preempt_count;
    uint16_t overrun_count;

    uint8_t  state; /* arghtos_task_state_t */
    uint8_t  flags; /* public subset of C8 flags */
} arghtos_task_profile_t;

_Static_assert(sizeof(arghtos_task_profile_t) == 20u,
               "profile ABI changed");
```

# Public profiling API

```c
arghtos_result_t arghtos_profile_get(
    arghtos_task_id_t task_id,
    arghtos_task_profile_t *out);
```

Call context: TASK only.

The snapshot is copied atomically with respect to scheduler updates using one bounded critical section.

Return values:

* `ARGHTOS_OK`;
* `ARGHTOS_ERR_BAD_TASK_ID`;
* `ARGHTOS_ERR_BAD_ARGUMENT` for `out == NULL`;
* `ARGHTOS_ERR_BAD_CONTEXT`.

The caller never receives a pointer to the private C8 record.

# Runtime accounting rules

On every dispatch of task `t`:

```text
t.run_start = now
t.dispatch_count = saturating_increment(t.dispatch_count)
```

On every deschedule:

```text
elapsed = now - t.run_start
t.last_run_ticks = elapsed
t.max_run_ticks = max(t.max_run_ticks, elapsed)
t.total_cpu_ticks += elapsed    modulo 2^32
```

If the deschedule reason is preemption:

```text
t.preempt_count = saturating_increment(t.preempt_count)
```

If a periodic release occurs while the prior job is still active:

```text
t.overrun_count = saturating_increment(t.overrun_count)
```

# Runtime budget

C0 `budget == 0` disables budget checking.

For nonzero budget, when a contiguous execution interval exceeds the configured budget:

```text
C8.flags |= ARGHTOS_TCF_BUDGET_EXCEEDED
```

V1 records the violation but does not automatically kill or suspend the task.

A budget violation is diagnostic evidence, not proof of a deadline miss unless the configured timing model says so.

# Observed maximum versus WCET

`max_run_ticks` is an **observed maximum contiguous execution interval**.

It is not named or treated as WCET.

P6 may consume an independently justified WCET value, but C5 never promotes measured runtime into proof evidence by itself.

# Fatal reason codes

Stored reason codes are fixed-width values:

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

Additional values require updating this contract.

# Concrete 64-byte crash record

```c
#define ARGHTOS_CRASH_MAGIC        0x41524748u /* "ARGH" */
#define ARGHTOS_CRASH_COMMIT_MAGIC 0x43524153u /* "CRAS" */
#define ARGHTOS_CRASH_VERSION      1u

typedef struct arghtos_crash_record {
    uint32_t magic;
    uint16_t version;
    uint16_t size;

    uint16_t reason;
    uint8_t  task_id;
    uint8_t  flags;

    uint32_t timestamp;
    uint32_t msp;
    uint32_t psp;
    uint32_t exc_return;

    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t xpsr;

    uint32_t commit_magic;
} arghtos_crash_record_t;

_Static_assert(sizeof(arghtos_crash_record_t) == 64u,
               "crash record ABI changed");
```

# Crash-record commit protocol

To reduce false-valid records after interrupted capture:

1. set `commit_magic = 0`;
2. write all other fields;
3. write `magic`, `version`, and `size` consistently;
4. write `commit_magic = ARGHTOS_CRASH_COMMIT_MAGIC` last.

A record is valid only if:

```text
magic == ARGHTOS_CRASH_MAGIC
version == ARGHTOS_CRASH_VERSION
size == 64
commit_magic == ARGHTOS_CRASH_COMMIT_MAGIC
```

# Fatal kernel entry

Internal kernel code uses:

```c
_Noreturn void arghtos_fatal(
    arghtos_fault_reason_t reason,
    const void *exception_frame);
```

`exception_frame` may be NULL for a kernel-detected invariant fault.

The fatal path:

1. prevents normal scheduler mutation;
2. captures the best available task ID, time, MSP, PSP, exception-return value, and stacked register frame;
3. commits the crash record if C0 supplied one;
4. invokes the terminal port hook.

# Terminal port hook

The platform provides:

```c
_Noreturn void arghtos_port_fault_terminal(void);
```

This hook may reset or halt.

It must not return, allocate memory, call scheduler APIs, or depend on the failed task stack.

# Persistence

C0 may supply `crash_record == NULL`.

If a record is supplied, the application chooses its storage section. A `.noinit` SRAM section is recommended when post-reset inspection is desired.

Flash logging is outside the kernel contract.

# Dependencies

[C0](C0-static-task-configuration.md), [C7](C7-common-types-abi.md), [C8](C8-task-control-record.md), [P6](../../proof/P6-rms-schedulability.md), [P7](../../proof/P7-bounded-memory-and-work.md).

# Dangerous aspects

* Fault capture recursively faulting.
* Depending on corrupted task-stack data when a hardware frame is unavailable.
* Calling formatted I/O or flash drivers in the fatal path.
* Mislabeling `max_run_ticks` as WCET.
* Expanding profile state without rechecking the SRAM budget.

# Boundary specification

C5 captures fixed binary evidence and returns bounded snapshots. Storage export, human-readable formatting, telemetry, and post-reset policy belong outside ArghtOS.
