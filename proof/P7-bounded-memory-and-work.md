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

Let `C=ARGHTOS_MAX_TASKS` and `N=task_count <= C`.

The concrete metadata lower bound is:

```text
M_metadata(C,N) = 20*C + 16*N + 20
```

because:

* C0 descriptor = 16 B/configured task;
* C8 control array = 20 B/capacity slot;
* C9 global = 20 B.

At 32 tasks:

```text
M_metadata(32) = 1172 B
```

The required crash record adds 64 B, making the 32-task pre-code resident lower bound 1236 B.

All linked ArghtOS `.text`, `.rodata`, literals, `.data`, `.bss`, `.noinit`, required scheduler metadata, and MSP accounting remain part of the complete footprint.

No runtime operation changes any of those storage cardinalities.

# Soft-target consequence

The 1.5 KiB target is not implied by boundedness.

The proof establishes that the footprint is finite; it does not establish that it is <=1536 B.

I0 now supplies implementation/linker evidence, and the quantitative claim fails for the current 32-task v1 implementation.

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

* uses the required fixed 64-byte record;
* performs no allocation;
* performs no formatted transport output;
* uses the C9 recursion guard;
* terminates in the platform fatal hook.

# Architecture/build tests

The project SHALL provide:

* linker-map attribution of all ArghtOS/port resident sections;
* an ArghtOS-footprint subtotal including code/constants/data/metadata and ArghtOS-only MSP requirement;
* a total-system SRAM report including the physical system MSP reservation, application task stacks/code, and driver state;
* configured task count and confirmation of the required crash record;
* loop-bound review or static checks;
* target timing of critical sections and dispatch paths;
* fault-injection tests;
* confirmation that no dynamic allocator is referenced.

# Dependencies

Depends on [P0](P0-static-configuration-validity.md) and [P4](P4-time-release-correctness.md).

Its execution-time bounds are inputs to P6.

# Status

The bounded-work and bounded-storage claims remain architecturally consistent and are supported by the current host implementation.

The quantitative footprint result is now known:

```text
I0 linked ArghtOS resident footprint before MSP = 4122 B
soft target                                     = 1536 B
delta                                           = +2586 B
```

See [I0 footprint skeleton](../implementation/I0-footprint-skeleton.md).

Static compiler/assembly analysis now supplies an approximately 104 B ArghtOS-only MSP lower bound, producing a current combined lower bound of about 4226 B. Hardware high-water validation can only maintain or increase the required physical reservation. The 1.5 KiB target therefore fails at I0 even though boundedness itself is preserved.

# Current 24-task R2c implementation evidence

The original 32-task I0 evidence remains above as the historical baseline.

The current default capacity is 24 tasks. The R2c minimum-footprint profile
combines generated configuration and runtime state into one 24-byte slot per
task.

Measured CI result:

```text
R2c complete resident footprint = 3376 B
soft target                     = 1536 B
excess                          = 1840 B
```

The R2c fixed slot/storage terms are bounded:

```text
unified task slots      24 * 24 B = 576 B
static C9                            12 B
crash record                         64 B
linker metadata                       16 B
ArghtOS-only MSP                     104 B
```

Runtime scheduler work remains bounded by the same task-count limits:

* release processing scans at most 24 task slots;
* selector inspects at most 48 task slots;
* no catch-up loop scales with missed releases;
* no runtime allocation exists.

R2c therefore preserves P7 boundedness while changing the concrete storage
representation.

Evidence: CI run 40649991293 and
[I1 footprint reduction](../implementation/I1-footprint-reduction.md).

The quantitative 1.5 KiB target still fails, but the failure is now dominated
by executable code rather than by the static metadata lower bound.
# R4a minimum-size profiling evidence

Runtime profiling may now be omitted in the minimum-size production build.

This changes the fixed build-time resident set but does not change P7's
boundedness argument: both configurations remain statically bounded, contain no
dynamic allocation, and retain the same bounded scheduler loops.

Measured 24-task evidence:

```text
R3c profiling enabled   3072 B
R4a profiling disabled  2845 B
soft target             1536 B
remaining excess        1309 B
```

R4a removes profiling-only counters, `run_start`, snapshot code, and execution
accounting. It does not remove crash state, missed-release evidence, or async
age credit.
