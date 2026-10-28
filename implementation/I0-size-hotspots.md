---
type: ImplementationAnalysis
title: "I0 - Cortex-M23 resident-size hotspot analysis"
description: "Symbol-level analysis of the stabilized I0 footprint showing why the 1.5 KiB target cannot be recovered by one local optimization."
tags: [implementation, footprint, size, optimization, cortex-m23]
status: current
---

# Stabilized baseline

```text
ArghtOS resident before MSP  4118 B
soft target                  1536 B
deficit                      2582 B
```

The build uses Cortex-M23, `-Oz`, section GC, no libc, and all ArghtOS sections in SRAM.

# Largest retained portable symbols

| Symbol | Bytes |
|---|---:|
| `arghtos_init` | 352 |
| `arghtos_kernel_service` | 304 |
| `process_time_locked` | 264 |
| `arghtos_select_next` | 252 |
| `arghtos_fatal` | 172 |
| `arghtos_kernel_dispatch` | 168 |
| `arghtos_start` | 148 |
| `wake_locked` | 144 |
| `arghtos_profile_get` | 76 |
| `arghtos_wake_from_interrupt` | 72 |
| `arghtos_task_start_trampoline` | 68 |
| `arghtos_timer_interrupt` | 64 |

The four largest portable functions total only 1172 B. Removing all four would still leave a 1390 B deficit while also destroying core scheduler functionality.

# Cortex-M23 port symbols

| Symbol | Bytes |
|---|---:|
| `PendSV_Handler` | 114 |
| `arghtos_port_crash_fill` | 84 |
| `arghtos_port_build_initial_context` | 68 |
| `arghtos_port_stack_valid` | 52 |
| `HardFault_Handler` | 28 |
| `arghtos_m23_hardfault_entry` | 22 |
| `arghtos_port_idle_wait` | 22 |
| `SVC_Handler` | 22 |

Complete port C + context/fault assembly attribution is 580 B before any ArghtOS-only MSP allowance.

# Public runtime API floor

The out-of-line runtime wrappers alone are approximately:

```text
yield                 28 B
wait                  28 B
sleep_until           30 B
sleep_for             48 B
wake                  30 B
wake_from_interrupt   72 B
profile_get           76 B
--------------------------
runtime wrappers     312 B
```

The audited 32-task metadata plus crash record leaves only 300 B for **all** code, constants, and ArghtOS-only MSP. Thus the current metadata representation plus required API already makes the 1.5 KiB target infeasible before scheduler policy, timer processing, context switching, or fault handling are counted.

# Compiler sweep

Current full-link text sizes:

```text
-Os                3304 B
-Oz                3052 B
-Oz + LTO          2760 B
```

LTO recovers hundreds of bytes, not the required thousands.

# Engineering conclusion

The size gap is architectural. Productive next reductions must change one or more of:

* per-task metadata representation;
* resident initialization/validation responsibilities;
* required runtime API/features;
* default 32-task capacity;
* profiling/crash richness;
* or the 1.5 KiB target itself.

Assembly micro-optimization and compiler flags remain useful later, but they cannot close the present 2582 B gap.

# Evidence source

Generated from the stabilized I0 `m23.elf` symbol table and `tools/footprint_report.py` after host tests, sanitizer tests, Cortex-M23 compile/link, HardFault capture, and idle/startup correctness fixes passed.
