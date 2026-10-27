---
type: Contract
title: "C9 - Kernel global state and SRAM accounting"
description: "Concrete 12-byte global kernel state and default scheduler SRAM budget equation."
tags: [architecture, contract, memory, kernel-state, sram]
status: draft
---

# Key goals

* Keep mutable global kernel state explicit.
* Prevent untracked singleton state from appearing across modules.
* Produce a concrete default SRAM accounting equation.
* Leave MSP sizing visible rather than hiding it in the linker script.

# Concrete global state

```c
typedef struct {
    const arghtos_config_t *config;
    arghtos_time_t          start_time;

    arghtos_task_id_t       current_task;
    uint8_t                 task_count;
    uint8_t                 flags;
    uint8_t                 reserved;
} arghtos_kernel_state_t;

_Static_assert(sizeof(arghtos_kernel_state_t) == 12u,
               "global kernel state SRAM contract changed");
```

# Global flag bits

```c
#define ARGHTOS_KF_INITIALIZED  (1u << 0)
#define ARGHTOS_KF_STARTED      (1u << 1)
#define ARGHTOS_KF_FAULT_ACTIVE (1u << 2)
#define ARGHTOS_KF_RESERVED     (0xF8u)
```

Reserved bits remain zero in v1.

# Field semantics

* `config`: immutable C0 configuration pointer.
* `start_time`: scheduler epoch captured exactly once at start.
* `current_task`: RUNNING task ID or `ARGHTOS_TASK_ID_INVALID`.
* `task_count`: cached validated count from C0.
* `flags`: lifecycle/fault recursion state.
* `reserved`: zero.

# Single-source rule

No code block may create another mutable global copy of:

* current task ID;
* task count;
* scheduler start epoch;
* initialized/started state;
* fatal recursion state.

Derived local variables are allowed; persistent duplicate authorities are not.

# Default mandatory SRAM

At `ARGHTOS_MAX_TASKS=32`:

```text
C8 task-control array   32 * 28 = 896 B
C9 global state                    12 B
---------------------------------------
mandatory kernel state            908 B
```

If the application supplies the C5 crash record in SRAM:

```text
mandatory kernel state            908 B
crash record                       64 B
---------------------------------------
state + crash                      972 B
```

Immutable C0 descriptors should reside in flash and do not count toward mutable SRAM.

Application task stacks are excluded by requirement.

# MSP accounting

MSP is not hidden from the memory budget.

The integration report must state:

```text
total_scheduler_sram =
    908
  + crash_record_bytes
  + reserved_MSP_bytes
  + any port-specific mutable bytes
```

The soft target is:

```text
total_scheduler_sram <= 1536 bytes
```

for the default 32-task configuration.

Because application/device interrupt handlers also execute on MSP, final MSP sizing cannot be proven from scheduler code alone. Target qualification must measure or statically bound worst-case nested MSP use.

# Port-specific mutable storage

Any Cortex-M23 port-specific mutable object not already represented by C8/C9 must be individually listed in the memory report.

A port may not silently add per-task arrays.

# Fault recursion

C5 sets `ARGHTOS_KF_FAULT_ACTIVE` before crash capture.

If fatal handling is entered while this bit is already set, the implementation bypasses normal capture and immediately invokes `arghtos_port_fault_terminal()`.

This prevents recursive fault capture from consuming unbounded MSP.

# Dependencies

[C0](C0-static-task-configuration.md), [C5](C5-fault-diagnostics.md), [C8](C8-task-control-record.md), [P0](../../proof/P0-static-configuration-validity.md), [P7](../../proof/P7-bounded-memory-and-work.md).

# Dangerous aspects

* Hidden mutable globals in individual modules.
* Counting task stacks but forgetting MSP, or vice versa.
* Port-specific arrays that scale with task count.
* Recursive fault capture.
* Assuming application interrupt stack demand is a scheduler constant.

# Boundary specification

C9 owns scheduler-global mutable truth and accounting. Linker/startup code owns the physical MSP reservation; system integration must report both together.
