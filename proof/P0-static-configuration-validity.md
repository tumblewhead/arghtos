---
type: Proof
title: "P0 - Static configuration and resident-footprint boundedness"
description: "The configured scheduler instance has finite static resources, no dynamic allocation, and a link-time bounded SRAM-resident footprint."
tags: [scheduler, cortex-m23, proof, configuration, static-memory, footprint]
status: draft
---

# Statement

For compile-time capacity `C = ARGHTOS_MAX_TASKS` and configured task count `N <= C`:

1. task identity is finite;
2. no runtime operation allocates scheduler storage;
3. all ArghtOS code/data/configuration has finite link-time size; and
4. the complete ArghtOS resident SRAM footprint is bounded for the lifetime of the program.

# Assumptions

1. `1 <= N <= ARGHTOS_MAX_TASKS <= 254`.
2. Every task has exactly one C0 descriptor.
3. Task stacks are statically supplied and excluded from the ArghtOS overhead target by explicit requirement.
4. All ArghtOS code and data, including read-only sections, execute/reside from SRAM.
5. No scheduler API invokes a dynamic allocator.
6. No runtime operation creates a new task descriptor or stack.

# Proof

Let:

* `M_cfg(N) = 16N` bytes for C0 task descriptors;
* `M_ctl(C) = 20C` bytes for the statically allocated C8 control array;
* `M_global = 20` bytes for C9 global state;
* `M_crash = 64` bytes for the required v1 crash record;
* `M_code` be linked ArghtOS/port executable bytes resident in SRAM;
* `M_const` be linked ArghtOS/port read-only constants/literals resident in SRAM;
* `M_other` be any additional linked ArghtOS resident data not already represented above;
* `M_MSP` be the reserved MSP contribution required by the integration.

Every term is finite at link/integration time.

Therefore:

```text
F_arghtos(C,N) =
    M_cfg(N)
  + M_ctl(C)
  + M_global
  + M_crash
  + M_code
  + M_const
  + M_other
  + M_MSP_arghtos
```

is finite.

Because runtime execution changes none of these storage cardinalities and performs no allocation, the resident footprint cannot grow beyond the linked/reserved amount.

# Concrete metadata lower bound

At `N=32`:

```text
C0 descriptors = 512 B
C8 controls    = 640 B
C9 global      =  20 B
----------------------
metadata       = 1172 B
```

With the required crash record:

```text
pre-code resident lower bound = 1236 B
```

This is a lower bound before ArghtOS `.text`, `.rodata`, literals, other sections, and MSP.

# Soft-target status

The requirement:

```text
F_arghtos(32) <= 1536 B
```

is a **soft target and is disproven by the I0 implementation baseline**.

The first linked Cortex-M23 I0 build reports:

```text
ArghtOS resident before MSP = 4122 B
soft target                 = 1536 B
excess before MSP           = 2586 B
```

The report already includes retained portable kernel code/data, Cortex-M23 port C, context assembly, board-adapter bytes, 32 task descriptors, the required crash record, and linker-resident metadata. The ArghtOS-only MSP term is still zero in this baseline, so 4012 B is an optimistic lower bound.

Therefore P0 proves **boundedness**, but the v1 32-task implementation does not satisfy the 1.5 KiB quantitative target.

# Architecture test

A build claiming a footprint result SHALL provide a linker/map-derived report that:

1. identifies configured `N`;
2. includes ArghtOS/port `.text`;
3. includes ArghtOS/port `.rodata`, literal pools, and jump tables;
4. includes `.data`, `.bss`, `.noinit`, and C0 descriptors;
5. includes the required 64-byte crash record;
6. reports the MSP reservation used by the integration;
7. excludes application task stacks only from the ArghtOS-overhead subtotal, while reporting them in the S0 system total;
8. proves no dynamic allocator reference exists in ArghtOS.

# Dependencies

P1, P2, and P7 depend on this finite static resource model.

# Implementation evidence and remaining obligations

I0 evidence is recorded in [../implementation/I0-footprint-skeleton.md](../implementation/I0-footprint-skeleton.md).

Now known:

* linked code/data/configuration lower bound: 4012 B before MSP;
* the 1536 B soft target is not met.

Still open:

* target ArghtOS-only MSP requirement/high-water bound;
* hardware-validation changes that may alter port size;
* future architecture/requirement reductions if the 1.5 KiB goal is retained.


# Current implementation lower bound including static MSP evidence

The stabilized Clang `-Oz` I0 stack-usage evidence identifies a normal SVC sleep path of approximately 104 B of ArghtOS-only MSP.

Thus:

```text
F_arghtos >= 4106 + 104 = 4226 B
```

before any physical-system MSP growth due to application/device handler nesting.

# Current 24-task I1 compact candidate

The 32-task I0 result above is retained as historical implementation evidence.

The current requirement default on the I1 experiment branch is now:

```text
ARGHTOS_MAX_TASKS = 24
```

The active minimum-footprint implementation candidate is R2c.

R2c uses generated unified task slots rather than a separate generic C0
descriptor array plus C8 control array.

For the R2c profile:

```text
24 unified task slots    24 * 24 B = 576 B
static C9                              12 B
required crash record                  64 B
linker metadata                        16 B
ArghtOS-only MSP                      104 B
-------------------------------------------
resident state/metadata + MSP         772 B
```

The complete measured Cortex-M23 resident footprint is:

```text
F_arghtos_R2c(24) = 3376 B
soft target        = 1536 B
excess             = 1840 B
```

Evidence:

* CI run 40649991293;
* [I1 footprint reduction](../implementation/I1-footprint-reduction.md).

Therefore P0 continues to prove static boundedness, while the current 24-task
R2c candidate still does not satisfy the quantitative soft target.

The generic formula based on separate C0/C8 structures remains valid for the
compatibility profile. The generated R2c profile is a specialized
implementation of the same static-resource principle and has its own linked
measurement above.

# R6p generated initialization refinement

For the selected generated Cortex-M23 profile, C4 establishes that
`arghtos_port_init()` is bounded and infallible.

R6p therefore moves the invariant post-init C9 values
`current_task=INVALID` and `flags=INITIALIZED` into the generated image.
The public init call still executes the port initialization exactly once.

Because the selected port cannot fail, the state observed after the call and
the returned result are identical to the ordinary successful initialization
path. No storage cardinality changes and no new runtime resource is introduced.
