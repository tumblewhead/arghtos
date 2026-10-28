---
type: CodeModuleDesign
title: "D07 - Runtime profiling, crash capture, and fatal path"
description: "Design of per-task CPU accounting, profile snapshots, missed-release evidence, 64-byte crash capture, and recursion-safe fatal termination."
tags: [design, diagnostics, profiling, fault, crash]
status: draft
---

# Goal

Provide the required runtime evidence and crash logging with bounded resident code/data, no transport dependency, and no hidden per-task state beyond C8.

# Logical module

```text
DIAGNOSTICS_FAULT
```

# Physical implementation

Portable logic in:

```text
src/arghtos_kernel.c
```

Architecture register/frame extraction is entirely owned by the selected port through `arghtos_port_crash_fill()`.

# Owned persistent storage

D07 owns no private persistent array.

It consumes:

* C8 profiling fields;
* C9 `run_start`, `current_task`, and fault-active bit;
* required application-provided 64-byte crash record.

# Public interface

```c
arghtos_result_t arghtos_profile_get(
    arghtos_task_id_t id,
    arghtos_task_profile_t *out);
```

# Private interfaces

Proposed logical helpers:

```c
static void profile_on_dispatch(
    arghtos_task_id_t id,
    arghtos_time_t now);

static void profile_on_deschedule(
    arghtos_task_id_t id,
    arghtos_time_t now);

static void missed_release_add(
    arghtos_task_id_t id,
    uint32_t count);

_Noreturn void arghtos_fatal(
    arghtos_fault_reason_t reason,
    const void *port_fault_context);
```

The first two helpers may be folded directly into D09 dispatch if that is smaller.

# CPU accounting semantics

On task dispatch:

```text
kernel.run_start = now
```

On deschedule:

```text
elapsed = now - kernel.run_start
c.total_cpu_ticks += elapsed
c.max_run_ticks = max(c.max_run_ticks, elapsed)
```

The accounting point happens exactly once per completed RUNNING residency.

A same-task "dispatch" caused by reconsideration still closes one residency and starts another if D09 actually closes and restarts the residency through the selected port's dispatch path.

# Profile snapshot semantics

`arghtos_profile_get()` returns the committed C8 values.

If the queried task is currently RUNNING, the unfinished current residency is **not** added speculatively to the snapshot in v1.

Reason:

* avoids a second live-accounting path;
* keeps C5 semantics equal to C8 committed state;
* reduces code.

The next deschedule commits that residency.

This is a deliberate snapshot semantic and testers should not expect instantaneous CPU time.

# Missed release evidence

D05 updates `missed_release_count` through a saturating helper.

D07 exposes it but does not infer root cause.

A missed release can mean:

* timer service was delayed across several epochs;
* prior job remained active;
* both.

# Crash record write protocol

Reference order:

```text
record.commit_magic = 0

record.magic = ARGH
record.version = 1
record.size = 64
record.reason = reason
record.task_id / flags
record.timestamp
zero record.port_format / port_words / port_state[]
arghtos_port_crash_fill(record, port_fault_context)

compiler barrier / required ordering primitive
record.commit_magic = CRAS
```

The commit marker is the final validity write.

No CRC is required in v1.

# Fatal recursion guard

First action:

```text
if KERNEL_FAULT_ACTIVE already set:
    port_fault_terminal()

set KERNEL_FAULT_ACTIVE
```

Then normal capture proceeds.

The recursion guard must not itself depend on complex helper code.

# Task-attributed fault

If:

* `current_task != INVALID`; and
* reason is attributable to the current task,

then:

```text
controls[current].state = FAULTED
```

before crash snapshot.

Kernel-global corruption faults do not assign FAULTED arbitrarily.

# Port-fault context opacity

`port_fault_context` is never dereferenced by D07.

For a CPU/architecture exception, the selected port passes whatever private context pointer it needs and later interprets it inside `arghtos_port_crash_fill()`.

For a kernel-detected invariant fault, `port_fault_context` may be NULL. D07 zeroes the generic port payload before calling the port, so a port that has nothing useful to add can leave `port_words=0` and clear PORT_STATE_VALID.

The portable crash path therefore contains no architecture register names.

# Fatal terminal hook

```c
_Noreturn void arghtos_port_fault_terminal(void);
```

The hook may reset or halt.

It must not:

* call scheduler APIs;
* format strings;
* depend on the current task's target-specific execution stack/context;
* allocate;
* return.

# Key goals

* One accounting path.
* One crash format.
* Commit-last validity.
* No logging transport.
* No strings.
* No per-task diagnostic side array.

# Dependencies

* C5, C8, C9.
* P6, P7.
* D05, D06, D09.

# Dangerous code sections and tester guidance

## 1. Double-accounting one RUNNING residency

**Failure example:** D03 yield path accounts CPU time and D09 dispatch accounts it again.

CPU time becomes roughly 2x actual.

**Tester should:**

* run deterministic fake-clock traces for yield, wait, sleep, preemption, same-task reselect;
* assert each interval appears exactly once;
* compare total against a golden event timeline.

## 2. Snapshot reads torn fields

On a 32-bit target each 32-bit field is naturally aligned, but a snapshot across several fields can still mix before/after states if preempted.

**Tester should:**

* force timer/wake dispatch between every field copy in a host interleaving model;
* verify D07 uses one bounded critical section around the full snapshot;
* assert snapshot is from one logical scheduler instant.

## 3. Crash commit marker written too early

**Bad implementation:** set `commit_magic` before register fields.

Reset/power loss during later writes produces a record that looks valid but is half old.

**Tester should:**

* fault-inject termination after every crash-record store;
* only the fully completed sequence may pass validity checks;
* prefill crash record with a distinct old pattern to expose stale fields.

## 4. Fatal recursion consumes the port's kernel/handler stack until overflow

**Failure scenario:** crash capture faults because crash storage is invalid; HardFault re-enters capture; repeat.

**Tester should:**

* force a fault inside the capture path in a controlled target test;
* verify second entry skips normal capture and reaches terminal hook;
* measure the selected port's kernel/handler stack high water during recursion testing.

## 5. Frame pointer assumed valid for kernel invariant fault

**Bad implementation:** portable D07 casts `port_fault_context` to a Cortex frame and reads PC/LR directly.

**Tester should:**

* invoke every kernel-detected fault with NULL port context in the host model;
* verify generic port payload starts zero and only the fake port may populate it;
* use memory sanitizers.

## 6. Running task profile expected to be "live"

A tester could mistakenly report a bug because total_cpu_ticks does not include the currently executing partial residency.

**Tester should:**

* explicitly test documented committed-snapshot semantics;
* query a RUNNING task before deschedule, then after yield;
* verify only the latter includes the interval.

## 7. Saturating missed-release count wraps

**Bad implementation:** `uint8_t += count`.

**Tester should:**

* start at 254/255;
* add 1,2,255,256,large values;
* assert result never decreases.

## 8. Human-readable fault text sneaks into core

A convenient `switch(reason) -> "bad dispatch"` table can exceed the code/data budget.

**Tester should:**

* scan the linked image for fault strings;
* minimum-footprint profile should contain numeric reason codes only.

# Test seam

Host tests can provide:

* deterministic fake clock;
* synthetic opaque port-fault context owned by the fake port;
* crash record initialized to a poison pattern;
* terminal hook replaced by longjmp/test abort.

Each target port must verify that its own documented `port_state[]` format captures the intended architecture state.

# Boundary specification

D07 owns architecture-neutral evidence capture and profiling presentation. Target register/frame extraction belongs to the selected port. D07 does not decide scheduling, recovery policy beyond terminal handoff, or telemetry transport.
