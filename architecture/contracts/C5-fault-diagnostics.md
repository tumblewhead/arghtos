---
type: Contract
title: "C5 - Fault, crash record, and runtime profiling"
description: "Concrete fixed-size crash record, diagnostic snapshot API, and lightweight runtime accounting."
tags: [architecture, contract, fault, diagnostics, profiling]
status: draft
---

# Key goals

* Capture fatal evidence without dynamic allocation or transport dependencies.
* Provide the required runtime profiling: cumulative CPU time and observed maximum contiguous run.
* Keep diagnostic storage bounded and SRAM-visible.
* Distinguish observed runtime from proven WCET.

# Public profiling snapshot

```c
typedef struct {
    uint32_t total_cpu_ticks;
    uint32_t max_run_ticks;

    uint8_t  missed_release_count;
    uint8_t  age_credit;
    uint8_t  state;
    uint8_t  flags;
} arghtos_task_profile_t;

_Static_assert(sizeof(arghtos_task_profile_t) == 12u,
               "profile ABI changed");
```

# Public profiling API

```c
arghtos_result_t arghtos_profile_get(
    arghtos_task_id_t task_id,
    arghtos_task_profile_t *out);
```

Call context: TASK only.

The snapshot is copied atomically using one bounded critical section.

# Runtime accounting

C9 owns one global `run_start` timestamp because at most one task is RUNNING.

On dispatch:

```text
kernel.run_start = now
```

On deschedule:

```text
elapsed = now - kernel.run_start
task.total_cpu_ticks += elapsed
task.max_run_ticks = max(task.max_run_ticks, elapsed)
```

`total_cpu_ticks` wraps modulo 2^32.

`max_run_ticks` is an observed maximum contiguous scheduler run. It is not a proof of job WCET.

The profiling contract requires each uninterrupted RUNNING residency to be shorter than one full 2^32-tick clock cycle. Periodic tasks satisfy a much tighter bound through P6; async workers are expected to yield, wait, sleep, or be preempted before a full clock cycle elapses.

# Periodic missed-release count

When a periodic release is missed while the prior job remains active, `missed_release_count` is increased by the number of missed releases, saturating at `UINT8_MAX`.

The count may therefore advance by more than one in one delayed timer service.

# Async age credit

The snapshot exposes C8 `age_credit` only as diagnostic state. C5 does not interpret it as time.

# Generic port crash payload

The public crash record is architecture-neutral. The kernel does not expose or interpret a Cortex exception-frame type.

Crash flags are generic:

```c
#define ARGHTOS_CRASH_F_TASK_VALID       (1u << 0)
#define ARGHTOS_CRASH_F_PORT_STATE_VALID (1u << 1)
#define ARGHTOS_CRASH_F_RESERVED         (0xFCu)
```

The target port owns the meaning of `port_format`, `port_words`, and `port_state[]` below.

# Fault reasons

```c
typedef uint16_t arghtos_fault_reason_t;

enum {
    ARGHTOS_FAULT_NONE             = 0x0000u,
    ARGHTOS_FAULT_KERNEL_INVARIANT = 0x0001u,
    ARGHTOS_FAULT_BAD_SERVICE      = 0x0002u,
    ARGHTOS_FAULT_CONTEXT_CORRUPT  = 0x0003u,
    ARGHTOS_FAULT_ASYNC_RETURNED   = 0x0004u,
    ARGHTOS_FAULT_STACK_RANGE      = 0x0005u,
    ARGHTOS_FAULT_BAD_DISPATCH     = 0x0006u,
    ARGHTOS_FAULT_CPU_EXCEPTION    = 0x0100u
};
```

# Required architecture-neutral 64-byte crash record

```c
#define ARGHTOS_CRASH_MAGIC        0x41524748u
#define ARGHTOS_CRASH_COMMIT_MAGIC 0x43524153u
#define ARGHTOS_CRASH_VERSION      1u

typedef struct arghtos_crash_record {
    uint32_t magic;
    uint16_t version;
    uint16_t size;

    uint16_t reason;
    uint8_t  task_id;
    uint8_t  flags;

    uint32_t timestamp;

    uint16_t port_format;
    uint8_t  port_words;
    uint8_t  reserved;

    uint32_t port_state[10];

    uint32_t commit_magic;
} arghtos_crash_record_t;

_Static_assert(sizeof(arghtos_crash_record_t) == 64u,
               "crash record ABI changed");
```

`port_words` is in the range 0..10. The portable kernel zeros the port payload before asking the selected port to fill it. A target port may define one or more `port_format` identifiers in its own port documentation.

# Crash commit protocol

`commit_magic` is cleared first and written valid last.

Before the final commit-marker write, portable C executes a C11 release fence. The Cortex-M23 reference compiler lowers this to a DMB, ensuring prior crash-record stores are ordered before the validity marker.

A record is valid only if magic, version, size, and commit marker all match.

# Fatal kernel entry

Portable kernel faults use:

```c
_Noreturn void arghtos_fatal(
    arghtos_fault_reason_t reason,
    const void *port_fault_context);
```

The `port_fault_context` pointer is opaque to portable code. It may be NULL for a kernel-detected invariant fault.

The fatal path:

1. sets the C9 fault-active guard;
2. prevents normal scheduler mutation;
3. if the fault is attributable to the current task, marks that task FAULTED;
4. writes generic crash header/task/time fields;
5. zeros the generic port payload;
6. calls `arghtos_port_crash_fill(record, port_fault_context)`;
7. writes the crash commit marker last;
8. calls the terminal port hook.

The portable kernel never dereferences `port_fault_context`.

# Terminal hook

```c
_Noreturn void arghtos_port_fault_terminal(void);
```

It must not return, allocate, call normal scheduler APIs, or depend on the failed task stack.

# Resident-memory accounting

The required 64-byte crash record counts against the resident SRAM budget.

Code implementing diagnostics, fault capture, constants, and literal data also counts because ArghtOS code is SRAM-resident.

# Dependencies

[C0](C0-static-task-configuration.md), [C7](C7-common-types-abi.md), [C8](C8-task-control-record.md), [C9](C9-kernel-global-state.md), [P6](../../proof/P6-rms-schedulability.md), [P7](../../proof/P7-bounded-memory-and-work.md).

# Dangerous aspects

* Treating observed max runtime as WCET.
* Fault recursion.
* Calling transport/formatting code from the fatal path.
* Forgetting crash/fault code size in the resident-footprint total.

# Boundary specification

C5 captures architecture-neutral evidence. Target register/frame extraction belongs entirely to the selected port. Export, formatting, telemetry, and post-reset policy are external.

# R3 minimum-footprint production profile

The generic/debug profile validates profile call context, output pointer, and task ID.

The R3 production profile assumes:

* `arghtos_profile_get()` is called from task context;
* `out` is non-NULL;
* `task_id` identifies a configured task.

The atomic snapshot operation and returned field semantics are unchanged for valid calls.

Fatal-path integrity checks are not removed by R3.
# Minimum-size production profiling option

Runtime profiling is **optional** in the minimum-size production profile.

The generic/debug profile and ordinary production profiles retain the public
profiling ABI and accounting described above. A minimum-size production build
may define:

```c
ARGHTOS_RUNTIME_PROFILING=0
```

In that profile only:

* `arghtos_profile_get()` is not part of the linked public surface;
* `total_cpu_ticks` and `max_run_ticks` are not retained per task;
* the global profiling `run_start` timestamp is not retained;
* dispatch does not perform execution-time accounting.

This option does **not** remove or weaken:

* the required 64-byte crash record;
* fatal-path capture;
* periodic missed-release evidence;
* async age credit;
* scheduler timing or release behavior.

Applications selecting the minimum-size profile must not reference
`arghtos_profile_get()`. The public header gates that declaration with
`ARGHTOS_RUNTIME_PROFILING`.

Measured R4a evidence at 24 tasks:

```text
profiling enabled R3c   3072 B
profiling disabled R4a  2845 B
saving                   227 B
```

The option is a footprint configuration, not a change to scheduler semantics.

# R5s generated crash-record preseed refinement

The trusted generated minimum-size profile may materialize the required
64-byte crash record with invariant fields already initialized before
`arghtos_init()`:

```text
magic        = ARGHTOS_CRASH_MAGIC
version      = ARGHTOS_CRASH_VERSION
size         = sizeof(arghtos_crash_record_t)
task_id      = ARGHTOS_TASK_ID_INVALID
flags        = 0
port_format  = 0
port_words   = 0
reserved     = 0
port_state[] = 0
commit_magic = 0
```

The record remains writable ArghtOS-resident storage and still counts in full
against the resident-footprint budget.

This refinement is permitted only when the generated startup mechanism
materializes that image before scheduler initialization. The fatal path still:

1. clears `commit_magic` before dynamic capture;
2. writes dynamic reason, timestamp, task attribution, and port payload;
3. bounds/marks the port payload exactly as the ordinary path does;
4. executes the release fence;
5. writes `ARGHTOS_CRASH_COMMIT_MAGIC` last.

A successful fatal capture never returns. A recursive fatal reaches the
terminal hook before beginning another capture. Therefore one runtime does not
perform a second successful capture that would require re-zeroing the preseeded
payload or invariant fields.

Generic/debug and non-preseeded profiles retain explicit full record
initialization in `arghtos_fatal()`. Post-reset export/persistence policy
remains outside C5.


# R5w trusted generated crash-port refinement

The trusted generated minimum-size profile may further specialize C5 when the
selected port contract proves that every successful
`arghtos_port_crash_fill()` invocation writes a valid nonzero payload count:

```text
1 <= port_words <= 10
```

The reference host and Cortex-M23 ports satisfy this premise.

When combined with R5s generated crash preseed and R5n current-task invariants,
the generated startup image may preseed:

```text
flags = ARGHTOS_CRASH_F_PORT_STATE_VALID
commit_magic = 0
```

For this profile only:

* the first fatal capture need not clear `commit_magic` again because the
  startup image guarantees it is already invalid and recursive fatal exits
  before recapture;
* the portable fatal path need not clamp `port_words` or test it for zero;
* task attribution adds `ARGHTOS_CRASH_F_TASK_VALID` without clearing the
  preseeded port-state-valid bit;
* `current_task` may be tested against `ARGHTOS_TASK_ID_INVALID` rather than
  range-validated because R5n/P1 prove it is either INVALID or configured.

The required 64-byte record, dynamic reason/timestamp/task ID, port payload,
release fence, and commit-last protocol are unchanged.

Generic/debug and non-R5w profiles retain the defensive clamp, zero-payload
handling, and full task-ID validation.


# R5ac terminal fault-state elision

The trusted generated minimum-size profile may omit the final task-control
mutation that marks the current task `FAULTED` inside `arghtos_fatal()`.

This specialization is permitted only when the accepted R5n generated
lifecycle/dispatch proof is active and the terminal hook remains non-returning.

The reasoning is:

* `arghtos_fatal()` does not return to scheduler execution;
* `arghtos_port_fault_terminal()` does not return;
* no subsequent scheduler transition can observe the task-control state;
* the crash record still captures the fault reason and current task ID;
* recursive fatal protection and commit-last crash integrity are unchanged.

Therefore the `FAULTED` state write is redundant for valid execution of the
terminal minimum profile. Generic/debug and other profiles retain task-state
mutation for diagnostic visibility.

# R6g proven current-task domain in fatal capture

The trusted generated minimum-size profile may consume the R5n dispatcher
invariant that C9 `current_task` is always either:

* `ARGHTOS_TASK_ID_INVALID`; or
* the ID of a configured task previously selected by the dispatcher.

For that profile, fatal capture may test task attribution with
`current_task != ARGHTOS_TASK_ID_INVALID` instead of repeating an
`id < task_count` validation.

This refinement changes no crash-record field or commit rule. It only removes a
runtime range check whose stronger domain property is already proved by R5n.
The generic/debug profile retains ordinary task-ID validation.

# R6m commit-word recursion guard

The trusted generated minimum-size profile may use the crash record's
`commit_magic` word as both the capture-validity marker and the recursive-fatal
guard.

This specialization requires R5s generated startup, which materializes
`commit_magic = 0` before `arghtos_init()`, and a static valid crash-record
address.

For this profile, the fatal entry protocol is:

```text
if commit_magic != 0:
    terminal immediately

commit_magic = CAPTURE_IN_PROGRESS   # nonzero and != ARGHTOS_CRASH_COMMIT_MAGIC
write dynamic crash fields
fill port payload
release fence
commit_magic = ARGHTOS_CRASH_COMMIT_MAGIC
terminal
```

A record is valid only when the commit word equals
`ARGHTOS_CRASH_COMMIT_MAGIC`. Therefore the in-progress value invalidates the
record before dynamic capture exactly as required by the commit-last protocol.

Because a successful fatal path never returns, any second fatal entry in the
same runtime is recursive. It observes either the in-progress marker or the
final valid commit marker, both nonzero, and transfers directly to the terminal
hook without recapturing.

The generated minimum profile may therefore omit the separate C9
`ARGHTOS_KF_FAULT_ACTIVE` test/store. Generic/debug profiles retain the C9
guard and the ordinary zero-before-capture protocol.

Post-reset crash export/persistence remains outside C5; R6m inherits R5s's
requirement that generated startup rematerializes the initial zero commit word.

# R6x bounded iterative missed-release saturation

The trusted compact minimum profile may update the packed four-bit periodic
missed-release counter with a bounded iterative saturator instead of
room/subtraction arithmetic.

The packed status high nibble is the missed count and the low nibble contains
state/wake bits. Starting from the current status, each loop iteration adds
`0x10`, preserving the low nibble, until either:

* the requested missed-release increment is exhausted; or
* the high nibble reaches `0xF`.

The result is exactly:

```text
min(15, old_missed + count)
```

and therefore preserves the C5 saturating diagnostic contract.

The loop executes at most `15 - old_missed <= 15` iterations, regardless of
the delayed-release count.

# R6y packed missed-release add/clamp

The trusted compact minimum profile may compute the four-bit missed-release
counter as an add-then-clamp operation:

```text
total = old_missed + count
new_missed = min(total, 15)
```

The packed low nibble containing task state/wake information is preserved by the
existing missed-count setter.

For every admitted delayed-release update, `count` is derived from the
wrap-safe due interval and is at most `0x80000000`. Since
`old_missed <= 15`, the 32-bit addition cannot overflow.

This representation is exactly equivalent to the existing room/subtraction
saturator and retains the same C5 diagnostic semantics.

# R6z trusted selected-port crash payload

When C4 proves the selected crash filler always returns
`1 <= port_words <= 10`, the generated crash-record image may preseed:

```text
flags |= ARGHTOS_CRASH_F_PORT_STATE_VALID
```

The portable fatal path may then omit the post-fill clamp and zero-payload test.
Task attribution ORs TASK_VALID into the preseeded flags rather than replacing
them.

R6z composes with the R6m commit-word recursion guard. The record remains
invalid while capture is in progress and is committed valid only after the port
payload has been written and the release fence executes.

Generic/debug and unproven ports retain the clamp, zero-payload handling, and
runtime PORT_STATE_VALID derivation.
