---
type: Proof
title: "P0 - Static configuration validity"
description: "The configured scheduler instance is finite, statically allocated, and internally well-formed before execution begins."
tags: [scheduler, cortex-m23, proof, configuration, static-memory]
generated:
  by: openai/gpt-5.6-sol
  at: 2026-10-27T20:08:00Z
status: draft
sources:
  - id: okf-v0.2
    title: "Open Knowledge Format v0.2"
    resource: https://github.com/GoogleCloudPlatform/open-knowledge-format/blob/main/SPEC.md
---

# Statement

For a build with compile-time constant `SCHED_MAX_TASKS = N`, every scheduler-owned object has a statically bounded cardinality and size, every task identifier is drawn from a finite set, and no scheduler operation requires dynamic allocation.

# Assumptions

1. `N >= 1`.
2. Every configured task has exactly one statically defined descriptor.
3. Every configured task stack is supplied by the application as a statically allocated region.
4. Scheduler-owned SRAM excludes application task stacks.
5. Task creation at runtime means activation of a predeclared task, not allocation of a new descriptor or stack.
6. No scheduler API calls `malloc`, `calloc`, `realloc`, `free`, or an equivalent allocator.

# Proof

Let the task identifier domain be `T = {0, 1, ..., N-1}`.

The scheduler state is composed only of:

* fixed kernel state whose size is independent of `N`;
* arrays or bitsets whose maximum length is a compile-time function of `N`; and
* per-task runtime state with exactly one slot per member of `T`.

Therefore the total scheduler-owned SRAM can be written as:

`M(N) = M_fixed + N * M_task + M_sets(N)`

where every term is known at link time.

Because the runtime cannot extend `T`, allocate another descriptor, or allocate another stack, execution cannot increase scheduler-owned storage beyond `M(N)`.

Thus the configured instance has finite, compile-time bounded scheduler state.

# Architecture test

The implementation SHALL provide build-time assertions or generated checks that establish:

1. the task table contains at most `SCHED_MAX_TASKS` entries;
2. every task ID is unique and in range;
3. every required stack pointer and size is non-null and valid for the selected port;
4. no scheduler translation unit references a dynamic allocation symbol;
5. scheduler-owned SRAM for the default `SCHED_MAX_TASKS=32` configuration is reported separately from task stacks; and
6. the default configuration SHOULD satisfy the soft target of at most 1.5 KiB scheduler-owned SRAM.

# Dependencies

This proof is foundational and is referenced by [P1](P1-task-state-invariants.md), [P2](P2-context-stack-integrity.md), and [P7](P7-bounded-memory-and-work.md).

# Concrete v1 instantiation

The concrete contracts now instantiate the mutable scheduler state as:

```text
M_task = 28 bytes
M_global = 12 bytes
M_ready_sets = 0 bytes
```

The reference selector scans the bounded task-control array and therefore requires no separate dynamic ready-queue storage.

For compile-time capacity `N = ARGHTOS_MAX_TASKS`:

```text
M_core(N) = 12 + 28*N
```

At the default `N=32`:

```text
M_core(32) = 908 bytes
```

An optional crash record adds 64 bytes. MSP reservation and any port-specific mutable bytes remain target-integration terms and are reported separately under C9.

# Open obligations

Final target qualification must instantiate:

* MSP reservation/high-water bound;
* any port-specific mutable bytes;
* linker/map evidence that immutable task descriptors reside outside mutable scheduler SRAM as intended.
