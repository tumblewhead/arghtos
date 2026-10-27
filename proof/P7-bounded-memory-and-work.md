---
type: Proof
title: "P7 - Bounded resident footprint and scheduler work"
description: "All ArghtOS-resident SRAM and all scheduler execution paths are statically bounded by the build and configured task count."
tags: [scheduler, cortex-m23, proof, bounded-work, memory, critical-section, footprint]
status: draft
---

# Statement

For fixed build configuration and `N <= ARGHTOS_MAX_TASKS`:

1. the complete ArghtOS SRAM-resident footprint is finite and link-time bounded;
2. every scheduler operation terminates after bounded work;
3. every interrupt-masked critical section has a finite bound; and
4. no delayed periodic-release path performs work proportional to the number of missed periods.

# Resident-footprint bound

P0 defines the complete resident footprint, including ArghtOS/port code and read-only data.

The concrete metadata lower bound is:

```text
M_metadata(N) = 36*N + 20
```

because:

* C0 descriptor = 16 B/task;
* C8 control = 20 B/task;
* C9 global = 20 B.

At 32 tasks:

```text
M_metadata(32) = 1172 B
```

Optional crash capture adds 64 B.

All linked ArghtOS `.text`, `.rodata`, literals, `.data`, `.bss`, `.noinit`, required scheduler metadata, and MSP accounting remain part of the complete footprint.

No runtime operation changes any of those storage cardinalities.

# Soft-target consequence

The 1.5 KiB target is not implied by boundedness.

The proof establishes that the footprint is finite; it does not establish that it is <=1536 B.

That quantitative claim requires implementation/linker evidence.

# Work bound

No scheduler loop may depend on an unbounded history or runtime-produced collection. Startup-only validation may be O(N^2) when the bound is explicitly `N <= ARGHTOS_MAX_TASKS`; runtime scheduling/release paths remain O(N).

Permitted bounded scans include:

* at most `N` descriptor/control entries in timer release processing;
* at most `2N` inspections in the reference selector;
* at most `N` entries for per-task initialization checks;
* at most `N*(N-1)/2` pair comparisons for one-time stack-overlap validation;
* fixed register counts in context save/restore.

# Delayed periodic release

P4/C3 compute:

```text
releases_due = 1 + late/period
```

and advance the next release with arithmetic.

The scheduler does not loop once per missed period.

Therefore release-handler work remains O(N) even after a long bounded delay.

# Critical-section bound

Every interrupt-masked scheduler critical section SHALL:

1. contain no application callback;
2. contain no hardware wait;
3. contain no unbounded retry;
4. contain only fixed work or an explicitly N-bounded scan;
5. be measurable on target.

The maximum duration becomes a P6 timing input.

# Fault path bound

Crash capture:

* uses a fixed 64-byte record when enabled;
* performs no allocation;
* performs no formatted transport output;
* uses the C9 recursion guard;
* terminates in the platform fatal hook.

# Architecture/build tests

The project SHALL provide:

* linker-map attribution of all ArghtOS/port resident sections;
* an ArghtOS-footprint subtotal including code/constants/data/metadata and ArghtOS-only MSP requirement;
* a total-system SRAM report including the physical system MSP reservation, application task stacks/code, and driver state;
* configured task count and crash-record setting;
* loop-bound review or static checks;
* target timing of critical sections and dispatch paths;
* fault-injection tests;
* confirmation that no dynamic allocator is referenced.

# Dependencies

Depends on [P0](P0-static-configuration-validity.md) and [P4](P4-time-release-correctness.md).

Its execution-time bounds are inputs to P6.

# Status

The boundedness proof is architecturally consistent.

The <=1.5 KiB quantitative target remains open until code exists and a linker/MSP report is available.
