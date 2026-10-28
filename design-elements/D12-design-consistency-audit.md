---
type: DesignAudit
title: "D12 - Code-module design consistency audit"
description: "Cross-check of D00-D11 against C0-C10 and P0-P8 before implementation."
tags: [design, audit, consistency, implementation-gate]
status: current
---

# Audit verdict

The code-module design is internally consistent enough to proceed to implementation.

This is a **design GO**, not a claim that:

* the 1.5 KiB resident-footprint target is achievable;
* P2 target context switching is proven;
* P6 schedulability is discharged;
* target timing/MSP bounds are known.

# Module inventory

| Element | Responsibility | Physical default |
|---|---|---|
| D01 | public ABI/API | `arghtos.h` + public functions |
| D02 | config/state/init/start | `arghtos_kernel.c` |
| D03 | lifecycle/wake/SVC service logic | `arghtos_kernel.c` |
| D04 | RMS/async selector | `arghtos_kernel.c` |
| D05 | timer/release/sleep | `arghtos_kernel.c` |
| D06 | Cortex-M23 mechanism | `arghtos_port.c` + `arghtos_context.S` |
| D07 | profiling/crash/fatal | `arghtos_kernel.c` |
| D08 | task trampoline | `arghtos_kernel.c` |
| D09 | portable integration/dispatch | `arghtos_kernel.c` |
| D10 | build/linker/footprint | build tooling |
| D11 | proof-oriented tests | host + target test tree |

# Key consistency decisions

## One portable mutable authority

Only:

```text
controls[ARGHTOS_MAX_TASKS]
kernel
```

hold persistent portable kernel state.

No module design introduces a hidden ready queue, timer queue, priority array, or per-task side table.

## One task handoff

Only D06 PendSV changes the executing PSP.

D03/D05 request dispatch but do not switch.

D04 selects but does not switch.

D09 integrates state/accounting and returns one saved PSP to D06.

## Dispatch serialization owner

D06 owns PRIMASK serialization around D09 dispatch.

D09 assumes dispatch is already serialized.

This prevents nested critical-section ambiguity and allows the no-winner decision to flow directly into the race-free idle protocol.

## Race-free idle

The selector's no-READY result is obtained while PRIMASK masks scheduler-relevant handlers.

WFI is entered without unmasking in between.

A qualifying pending interrupt is a WFI wake source under Armv8-M even with PRIMASK set; the port then restores normal masking so the handler runs before the next masked readiness check.

This closes the classic check-then-WFI race.

## Privileged task trust boundary

V1 tasks are privileged.

They must not directly modify scheduler-owned control state such as PRIMASK, PSP/MSP selection, PendSV/SVC configuration, or scheduler exception priorities.

This is an explicit trust assumption, not containment.

## Lifecycle ownership

D03 owns all normal lifecycle transitions.

Wake while RUNNING uses one pending bit.

Wake while SLEEPING preserves the timer-only sleep and sets the pending bit.

Periodic return goes DORMANT.

Async return goes fatal.

## Async aging

Only D04 increments age credit.

It does so only on actual async dispatch opportunities.

A redundant wake of READY work does not reset credit.

The selector must be invoked exactly once per dispatch decision because it is intentionally mutating.

## Time ownership

Only D05 interprets C8 `next_time`.

Periodic catch-up is arithmetic, not iterative per missed release.

A new sleep deadline causes one bounded timer-rearm scan because no global armed-deadline field exists.

## Periodic start

D02 releases all synchronous periodic tasks before first selection and establishes one shared start epoch.

## CPU-time accounting

D07/D09 account each completed RUNNING residency exactly once.

The current incomplete residency is not included in `profile_get()`.

## Trampoline restart

Periodic application call frames fully unwind before TASK_RETURN blocks the trampoline.

The next release resumes the trampoline and makes a fresh application call.

No task context is rebuilt every period.

# Resident-footprint design status

At default capacity/configuration 32:

```text
C0 descriptors      512 B
C8 controls         640 B
C9 state             20 B
required crash       64 B
--------------------------
pre-code total     1236 B

soft target        1536 B
residual            300 B
```

D10 reserves that 300 B as:

```text
252 B code + constants
 48 B ArghtOS-only MSP
-----------------------
300 B
```

These are red-line measurement envelopes, not predictions.

The design does not currently demonstrate they are feasible.

# First implementation checkpoint

Before implementing all behavior, create the smallest compilable Cortex-M23 skeleton containing:

* public headers/types;
* C0/C8/C9 objects;
* required crash record type;
* public symbol stubs;
* D06 exception/port stubs sufficient to link;
* task trampoline symbols.

Immediately capture:

* ELF;
* map;
* section sizes;
* symbol sizes;
* disassembly;
* stack-usage artifacts.

If the skeleton already consumes most or all of the 300 B residual, stop and revisit the footprint target/metadata representation before implementing the full scheduler.

# Open implementation choices that do not change contracts

These may be resolved by measurement:

* exact Thumb instruction sequence for R4-R11 save/restore;
* whether SVC wrapper bodies are C or assembly;
* whether some tiny private helpers are inlined/folded;
* whether LTO reduces or increases resident size;
* exact board timer source;
* physical split of portable C after size measurement.

# Decisions that require contract/proof review if changed

* adding phase or D!=T periodic tasks;
* adding event counts/queues to wake;
* waking SLEEPING tasks early;
* changing class-first periodic isolation;
* changing opportunity-based async aging;
* adding another context-switch path;
* dynamic task creation;
* adding per-task metadata;
* making crash logging optional;
* excluding text/rodata/MSP from footprint accounting;
* allowing periodic wait/sleep self-suspension.

# Dangerous implementation-stage traps testers should watch globally

## State-field writes outside their owner

A shortcut in port or timer code that writes lifecycle state can create a second transition semantics.

**Audit:** grep/write-access review of `controls[].state`.

## Hidden compiler artifacts

Jump tables, veneers, outlined helpers, unwind data, and assertion strings can consume the 300 B residual without appearing in C-level design.

**Audit:** symbol/map/disassembly diff from the first skeleton onward.

## Host tests accidentally validating a different algorithm

If the host model reimplements policy/time logic instead of compiling the production C, it can prove the model while production code differs.

**Audit:** host tests should link the real portable kernel wherever possible; reference models compare against it rather than substitute for it.

## Hardware test exercises only happy exception alignment

Context tests that never interrupt a 4-byte-aligned PSP miss the 68-byte runtime context case.

**Audit:** include deliberate alignment-padding scenarios.

## Timing tests alter timing

Heavy instrumentation inside critical/PendSV paths can invalidate measured overhead.

**Audit:** use minimally intrusive cycle counters/GPIO or measured instrumentation overhead and retain uninstrumented bounds where possible.

# Gate

The next stage may implement D01-D09 and the D10/D11 support tooling.

Implementation should proceed **footprint-first**, with the linkable skeleton checkpoint before deep feature coding.
