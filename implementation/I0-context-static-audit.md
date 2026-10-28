---
type: ImplementationAnalysis
title: "I0 - Cortex-M23 static context and MSP audit"
description: "Static disassembly and compiler stack-usage evidence for the Cortex-M23 reference port before hardware validation."
tags: [implementation, cortex-m23, context, msp, p2, audit]
status: current
---

# Scope

This is the static half of P2 verification. It uses the stabilized Cortex-M23 objects produced by the `-Oz` footprint build.

It does not replace target register-canary or stack-high-water testing.

# Context assembly findings

Disassembly confirms:

* `arghtos_port_service_call()` is exactly `SVC #0` followed by return;
* SVC handler reads the task's stacked R0/R1 as service ID/arg0, calls the portable kernel service, and writes the result back to stacked R0;
* PendSV saves R4-R11 below the existing hardware frame;
* high registers R8-R11 are moved through low registers for Armv8-M Baseline-compatible stores/loads;
* scheduler dispatch executes with PRIMASK set;
* selected R4-R11 and PSP are restored before exception return;
* HardFault captures EXC_RETURN, MSP, PSP, and chooses the basic frame from LR.SPSEL before entering the generic fatal path;
* the first-dispatch path records the pre-exception startup MSP and restores it before the first PSP exception return, so the startup hardware frame is not permanently stranded.

# Current assembly sizes

```text
arghtos_port_service_call     4 B
SVC_Handler                  22 B
PendSV_Handler              114 B
HardFault_Handler            28 B
-------------------------------
context/fault assembly      168 B nominal symbols
linked context object       172 B including literal/alignment bytes
```

# Compiler stack-usage evidence

Important portable/kernel frames:

```text
arghtos_init                  48 B
arghtos_start                 32 B
arghtos_kernel_service        40 B
process_time_locked           56 B
arghtos_select_next           48 B
arghtos_kernel_dispatch       24 B
arghtos_timer_interrupt       16 B
arghtos_fatal                 16 B
```

Important port C frames:

```text
arghtos_port_stack_valid          16 B
arghtos_port_build_initial_context 24 B
arghtos_m23_hardfault_entry       24 B
arghtos_port_init                  8 B
arghtos_port_fault_terminal        8 B
```

# Static ArghtOS-only MSP lower bound

The largest currently identified normal scheduler chain is the async sleep service:

```text
SVC handler explicit push        8 B
arghtos_kernel_service          40 B
process_time_locked             56 B
------------------------------------
normal-path static lower bound 104 B
```

Other representative normal chains are smaller:

```text
PendSV -> kernel_dispatch -> selector  about 72 B C stack
timer -> timer_interrupt -> time scan  about 72 B C stack
init -> initial-context builder        about 72 B C stack
```

This 104 B is used by the current footprint Makefile as `M23_ARGHTOS_MSP`.

It is a static lower bound, not a final physical MSP reservation. Target interrupt nesting and compiler/toolchain changes can increase the requirement.

# First-dispatch MSP reclamation

The first PendSV is entered from startup Thread mode using MSP. Returning directly to a task PSP without correction would leave the startup hardware frame occupying MSP permanently.

The stabilized assembly now:

1. detects the first/MSP-origin dispatch;
2. remembers the pre-exception MSP (`frame_base + 32` for the ABI-aligned startup path);
3. performs normal kernel selection;
4. restores the pre-exception MSP immediately before the first PSP exception return.

This adds a small amount of code but avoids a permanent 32-byte handler-stack penalty.

# Still required for P2

* hardware R4-R11 canary test across timer, yield, sleep, and interrupt-wake switches;
* first-context entry validation;
* SVC suspend/resume validation;
* PSP/MSP ownership observation on hardware;
* exception-alignment-padding case;
* idle/WFI race stress;
* target MSP/PSP high-water measurements;
* board timer/interrupt priority integration.

# Evidence status

The static context design is internally consistent with the current object code. P2 remains **not fully discharged** until hardware evidence exists.
