---
type: Contract
title: "C9 - Kernel global state and resident-footprint accounting"
description: "Concrete 20-byte global state plus complete SRAM-resident scheduler footprint accounting."
tags: [architecture, contract, memory, kernel-state, sram, footprint]
status: draft
---

# Key goals

* Keep mutable global kernel state explicit.
* Avoid retaining redundant top-level configuration.
* Count every ArghtOS-resident byte, including code and read-only data.
* Make the 1.5 KiB target a linker-verifiable footprint requirement rather than a mutable-state estimate.

# Concrete global state

```c
typedef struct {
    const arghtos_task_config_t *tasks;
    arghtos_crash_record_t      *crash_record;

    arghtos_time_t               start_time;
    arghtos_time_t               run_start;

    arghtos_task_id_t            current_task;
    uint8_t                      task_count;
    uint8_t                      flags;
    uint8_t                      reserved;
} arghtos_kernel_state_t;

_Static_assert(sizeof(arghtos_kernel_state_t) == 20u,
               "kernel-global resident-footprint contract changed");
```

# Global flags

```c
#define ARGHTOS_KF_INITIALIZED  (1u << 0)
#define ARGHTOS_KF_STARTED      (1u << 1)
#define ARGHTOS_KF_FAULT_ACTIVE (1u << 2)
#define ARGHTOS_KF_RESERVED     (0xF8u)
```

# Initialization ownership

C0's `arghtos_config_t` is only an initialization argument.

C9 copies:

* task-table pointer;
* crash-record pointer;
* task count.

The top-level config object need not remain allocated after `arghtos_init()` returns.

# Single-source rule

No module may keep another persistent copy of:

* current task ID;
* task count;
* scheduler start epoch;
* current run-start timestamp;
* task-table pointer;
* crash-record pointer;
* initialized/started/fault-active state.

# Pre-code resident metadata lower bound

Let:

* `C = ARGHTOS_MAX_TASKS` be compile-time control capacity;
* `N = task_count` be configured tasks, with `N <= C`.

Then:

```text
C0 task descriptors = 16*N
C8 task controls    = 20*C
C9 kernel global    = 20
--------------------------------
metadata(C,N)       = 20*C + 16*N + 20
```

Unused C8 capacity remains resident.

For the fully populated default build `C=N=32`:

```text
descriptors   512 B
controls      640 B
global         20 B
-------------------
metadata     1172 B
```

Optional crash record:

```text
1172 + 64 = 1236 B
```

# Complete ArghtOS resident footprint

The soft-target quantity is not "mutable SRAM." It is the complete incremental ArghtOS footprint resident in SRAM:

```text
F_arghtos =
    ArghtOS/port .text
  + ArghtOS/port .rodata
  + ArghtOS/port literal pools / jump tables
  + ArghtOS .data
  + ArghtOS .bss
  + ArghtOS .noinit
  + required C0 descriptor table
  + ArghtOS-only MSP requirement
  + any other ArghtOS-required resident section
```

The linker map is the authority. Section names do not create exemptions.

Where the descriptor/control/global objects already appear in `.rodata/.data/.bss`, they are counted once through the linker-map total, not added twice.

# Soft target

For a fully populated default 32-task build:

```text
F_arghtos <= 1536 bytes     SHOULD
```

This is a soft target, not presently a proven property.

With the current contracts, metadata alone consumes 1172 B, leaving only:

```text
1536 - 1172 = 364 B
```

for ArghtOS code, constants, MSP reservation, and other port/runtime bytes, or 300 B if the optional crash record is included.

Therefore **the current contract set does not establish feasibility of the 1.5 KiB soft target**. The target remains an explicit architecture risk to measure and optimize.

# Scope of the ArghtOS overhead target

Included:

* ArghtOS kernel code;
* Cortex-M23 scheduler-port code;
* ArghtOS constants/literals;
* task descriptor table required by ArghtOS;
* task control/global/crash data;
* scheduler/handler MSP reservation;
* any resident adapter code required solely for ArghtOS.

Excluded from the ArghtOS overhead target by prior requirement:

* application task stacks, including their saved frames;
* application task bodies;
* driver/application event FIFOs, DMA buffers, or application state.

Those excluded bytes still count in the MCU's **total system SRAM** and must be reported in the S0 integration budget.

# MSP qualification

There is one physical MSP, shared by ArghtOS exception paths and application/device handlers.

Two different quantities are reported:

* `M_MSP_arghtos`: the worst MSP depth required by ArghtOS-owned execution paths under the defined exception-priority model, excluding application/device-handler bodies.
* `M_MSP_system`: the actual linker/startup MSP reservation required for all permitted nested ArghtOS and application/device handlers.

The 1.5 KiB ArghtOS-overhead subtotal uses `M_MSP_arghtos`.

The S0 total-system SRAM report uses the physical `M_MSP_system` reservation exactly once.

No report may treat MSP as free merely because startup code already reserved it.

# Fault recursion

If fatal handling begins while `ARGHTOS_KF_FAULT_ACTIVE` is already set, normal capture is bypassed and the terminal hook is invoked immediately.

# Build acceptance evidence

Once code exists, every size-reporting build must retain:

* linker map;
* section-size summary;
* symbol/section attribution sufficient to identify ArghtOS and port bytes;
* configured task count;
* whether crash capture is enabled;
* MSP reservation.

# Dependencies

[C0](C0-static-task-configuration.md), [C5](C5-fault-diagnostics.md), [C8](C8-task-control-record.md), [P0](../../proof/P0-static-configuration-validity.md), [P7](../../proof/P7-bounded-memory-and-work.md).

# Dangerous aspects

* Reporting only `.bss + .data`.
* Treating `.text`, `.rodata`, or `const` objects as free.
* Double-counting objects already included in section totals.
* Excluding required scheduler metadata because the application declared it.
* Hiding MSP in startup/linker configuration.

# Boundary specification

C9 defines what "scheduler SRAM footprint" means. P0/P7 prove boundedness; the linker map later determines the actual value.
