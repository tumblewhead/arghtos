---
type: ArchitectureAudit
title: "A1 - Pre-module consistency audit"
description: "Cross-check of requirements-derived proofs, architecture, and concrete contracts before code-module design."
tags: [architecture, audit, consistency, proof, contract, footprint]
status: current
---

# Audit result

The proof/architecture/contract set is internally consistent enough to enter code-module design.

This does **not** mean all proofs are discharged.

The 1.5 KiB soft footprint target has now been tested by I0 and is not met: the linked baseline is 4106 B before MSP.

# Scope checked

* P0-P8 proof obligations.
* A0 architecture and T0/S0 integration views.
* C0-C10 concrete contracts and contract matrix.
* B0-B6 block descriptions.
* M0-M3 integrated submodules.
* Cortex-M23 context assumptions relevant to PSP/MSP switching.

# Corrections made during audit

## Resident SRAM accounting

Old assumption:

```text
mutable SRAM is the scheduler memory target
.text / .rodata / const configuration are effectively free
```

Corrected model:

```text
all ArghtOS-resident SRAM counts:
  kernel/port .text
  .rodata / literal pools / jump tables
  .data / .bss / .noinit
  required C0 descriptors
  C8/C9 runtime state
  required crash record
  ArghtOS-only MSP requirement
```

S0 separately reports total MCU SRAM including application task bodies/stacks and the physical MSP reservation.

## Metadata compression

C0 descriptor reduced from 32 B to 16 B by freezing v1 periodic semantics to:

* synchronous release;
* phase 0;
* D=T;
* class derived from period==0;
* no per-task diagnostic budget field.

C8 runtime record reduced from 28 B to 20 B by:

* removing non-required dispatch/preemption counters;
* moving current RUNNING start time to one C9 global;
* replacing time-based async aging with one-byte opportunity age credit.

C9 global state is 20 B.

Fully populated default 32-task metadata:

```text
C0 descriptors   512 B
C8 controls      640 B
C9 global         20 B
----------------------
                 1172 B
```

The required 64-byte crash record raises the pre-code resident lower bound to 1236 B.

## Capacity versus configured task count

C8 controls are allocated at compile-time capacity:

```text
20 * ARGHTOS_MAX_TASKS
```

Unused runtime capacity does not disappear.

C0 descriptors scale with actual configured task count.

## Wake/wait proof

P3 now matches C1/C2:

* wake while RUNNING sets one pending-wake bit;
* wait consumes that bit instead of blocking;
* wake after WAITING commit performs WAITING -> READY;
* repeated wakes collapse deliberately.

## Priority isolation

P5 now proves the actual class-first selector:

```text
periodic READY exists => select periodic
else => evaluate async
```

No numeric periodic/async priority boundary exists.

## Async starvation avoidance

Time-based aging was removed because a READY timestamp eventually wraps.

C6 now uses dispatch-opportunity age credit.

P8 proves eventual selection for continuously READY async tasks when async dispatch opportunities continue.

## Timer catch-up

P4/C3 now prohibit one loop per missed period.

Delayed periodic releases use:

```text
releases_due = 1 + late/period
```

and advance the release epoch arithmetically.

Runtime release processing remains O(N).

## Scheduler start

C0/T0 now explicitly:

1. capture one common start epoch;
2. release all synchronous periodic tasks;
3. set each next release to start+period;
4. arm the earliest future release;
5. enter first dispatch.

## Stack context reserve

The deliberately synthesized initial context is 64 B.

Runtime exception stacking can add one 4-byte alignment word, so each task stack must reserve at least 68 B for scheduler context overhead before application stack demand.

## Startup complexity

Pairwise stack-overlap validation is explicitly bounded O(N^2) at initialization.

Runtime scheduling/release paths remain O(N).

## Crash-record visibility

The crash-record type is part of the public header contract because C0 permits application-provided crash storage.

# Current proof status

| Proof | Architecture consistency | Remaining implementation evidence |
|---|---|---|
| P0 static/resource bound | consistent | linker map, code/const size, MSP |
| P1 state invariants | consistent | host transition tests |
| P2 context/stack | consistent | assembly review, register-canary hardware test, stack qualification |
| P3 wake/wait race | consistent | interleaving tests, critical-section validation |
| P4 time/release | consistent | arithmetic tests, timer service bound |
| P5 priority isolation | consistent | selector property tests |
| P6 RMS schedulability | conditional | task WCETs, release jitter, kernel/handler timing |
| P7 bounded work/footprint | consistent | linker map and target timing measurements |
| P8 async starvation avoidance | consistent | selector liveness/property tests |

# Footprint risk

Soft target:

```text
F_arghtos(32 tasks) <= 1536 B
```

Known metadata:

```text
1236 B including required crash record
```

Remaining before the target:

```text
300 B
```

That remainder must contain ArghtOS/port code, constants/literals, ArghtOS-only MSP requirement, and any other required resident bytes.

I0 has resolved this risk into a measured failure: the current implementation is 4012 B before MSP, 2570 B above the 1536 B soft target. Adding the current 104 B static ArghtOS-only MSP lower bound raises the implementation lower bound to about 4210 B. Retaining the target now requires architecture/requirement reduction, not merely compiler tuning.

# Gate for next stage

Code-module design may proceed because the behavioral contracts and proofs no longer contradict one another.

However, module design SHALL include:

* per-module resident-code/data budgets;
* a no-hidden-table rule;
* explicit attribution of port and inline/header-generated code;
* linker-map footprint tests from the first compilable skeleton;
* the required crash record included in every v1 footprint profile.

Linked I0 evidence now exists and shows the target is not met. No subsequent artifact may claim otherwise without newer linker/MSP evidence.
