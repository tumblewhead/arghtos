---
type: ImplementationCheckpoint
title: "I0 - Portable kernel and Cortex-M23 footprint skeleton"
description: "First implementation checkpoint: functional portable kernel, host verification port, Cortex-M23 reference port compile, and first linked SRAM footprint evidence."
tags: [implementation, footprint, cortex-m23, host-test, checkpoint]
status: current
---

# Result

The first implementation checkpoint is functional enough to test the portable scheduler semantics and link the Cortex-M23 reference build.

The 1.5 KiB complete ArghtOS resident-footprint soft target is **not met** by the first real implementation.

# Portable-kernel behavior implemented

* static configuration validation;
* pairwise stack overlap rejection;
* initialization and synchronous periodic start;
* class-first RMS selector;
* async dispatch-opportunity aging;
* yield / wait / sleep / wake lifecycle;
* one-bit wake/wait race handshake;
* timer-only sleep;
* wrap-safe absolute time;
* arithmetic periodic catch-up with no loop per missed period;
* saturating missed-release count;
* opaque context-token dispatch transaction;
* cumulative CPU and observed maximum-run profiling;
* generic 64-byte committed crash record;
* periodic task trampoline and async-return fatal path;
* architecture-neutral port ABI.

# Host verification

Passing checks:

```text
GCC -Wall -Wextra -Werror host suite       PASS
GCC ASan + UBSan host suite                PASS
Clang -Wall -Wextra -Werror host suite     PASS
portable source Cortex-identifier scan     PASS
```

# Cortex-M23 compile

The same `src/arghtos_kernel.c` compiles unchanged for:

```text
--target=arm-none-eabi -mcpu=cortex-m23 -mthumb
```

The Cortex-M23 reference port and context assembly also compile with Clang 17.

# Linked footprint baseline

Canonical first footprint build uses `-Oz`, section GC, no libc, and all sections linked into SRAM.

`tools/footprint_report.py` reports:

```json
{
  "kernel_code_data": 2896,
  "port_c_code_data": 376,
  "context_asm": 136,
  "board_adapter": 12,
  "task_descriptors": 512,
  "crash_record": 64,
  "linker_resident_metadata": 16,
  "arghtos_only_msp": 0,
  "arghtos_resident_total": 4012,
  "soft_target": 1536,
  "delta_to_target": -2476,
  "target_met": false
}
```

This is still optimistic because `arghtos_only_msp` is zero in the current report.

Therefore:

```text
measured before MSP    4012 B
soft target            1536 B
excess                 2476 B
```

# Compiler-size sweep

Exploratory full-link section totals:

```text
-Os baseline .text             3176 B
-Oz .text                      2944 B
-Oz + LTO .text                2656 B
```

LTO helps, but not nearly enough to bridge the gap. The footprint problem is architectural/feature-representation scale, not merely an optimization-flag problem.

# Important implementation limitations at I0

The portable scheduler semantics are host-tested, but the Cortex-M23 port has only compile/link evidence so far.

Still required before P2 is discharged:

* target R4-R11 canary tests;
* first-context hardware validation;
* service-call hardware validation;
* PendSV save/restore hardware validation;
* race-free idle hardware stress;
* PSP/MSP high-water measurement;
* exception-alignment-padding test;
* target timer adapter and jitter measurement.

# Gate / consequence

Implementation can continue, but no artifact may claim the 1.5 KiB target is met.

Before investing heavily in further code-size polishing, the next engineering decision should explicitly address the ~2.5 KiB footprint deficit through architectural or requirement-level reduction rather than accounting changes.
