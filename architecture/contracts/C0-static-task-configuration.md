---
type: Contract
title: "C0 - Static task configuration"
description: "Concrete 16-byte immutable task descriptor and static scheduler configuration."
tags: [architecture, contract, task, static-memory]
status: draft
---

# Key goals

* Define every task and stack before `arghtos_start()`.
* Keep task descriptors immutable after initialization.
* Keep all task storage statically allocated.
* Make task ID equal to descriptor-table index.
* Minimize resident configuration footprint because all sections, including read-only data, reside in SRAM on the target.

# Public task entry type

```c
typedef void (*arghtos_task_fn_t)(void);
```

Periodic entries execute exactly one bounded job and return.

Asynchronous entries are persistent workers and must not return during normal operation.

# Concrete 16-byte task descriptor

```c
typedef struct {
    arghtos_task_fn_t entry;       /* 4 B: immutable entry point */
    void             *stack_base;  /* 4 B: lowest address of task stack */
    arghtos_time_t    period;      /* 4 B: 0 => async, nonzero => periodic */
    uint16_t          stack_size;  /* 2 B: bytes, maximum 65535 */
    uint8_t           async_priority;
    uint8_t           flags;       /* reserved, zero in v1 */
} arghtos_task_config_t;

_Static_assert(sizeof(arghtos_task_config_t) == 16u,
               "task descriptor resident-footprint contract changed");
```

Task class is derived rather than stored:

```text
period == 0  => asynchronous
period != 0  => periodic
```

Task ID is the zero-based descriptor-table index. No redundant task ID is stored.

# V1 periodic timing contract

V1 periodic tasks are synchronous rate-monotonic tasks:

* first release occurs at scheduler start;
* phase is zero;
* relative deadline equals period: `D_i = T_i`;
* shorter period means higher RMS priority.

Arbitrary phase offsets and `D != T` are not part of v1. Adding them requires revisiting P6 and the resident-footprint budget.

# V1 asynchronous priority

For asynchronous tasks:

```text
period == 0
1 <= async_priority <= ARGHTOS_ASYNC_PRIORITY_MAX <= 254
```

`0x00` and `0xFF` are reserved and invalid for async tasks.

For periodic tasks, `async_priority` must be zero.

# Kernel configuration passed to init

```c
typedef struct {
    const arghtos_task_config_t *tasks;
    struct arghtos_crash_record *crash_record; /* required v1 crash record */
    uint8_t                      task_count;
    uint8_t                      reserved[3];
} arghtos_config_t;

_Static_assert(sizeof(arghtos_config_t) == 12u,
               "init configuration ABI changed");
```

The `arghtos_config_t` object itself is needed only for the duration of `arghtos_init()`. The task descriptor table and task stacks remain valid for the scheduler lifetime.

C9 copies the persistent task-table pointer, required crash-record pointer, and task count into kernel-global state. This avoids retaining a second configuration object.

# Configuration API

```c
arghtos_result_t arghtos_init(const arghtos_config_t *config);
void arghtos_start(void);
```

`arghtos_init()` is called once before scheduling begins.

`arghtos_start()` is called once after successful initialization and does not return during normal operation.

# Concrete scheduler start transaction

Before C4 performs the first task handoff, `arghtos_start()` enters the C4 scheduler-serialization primitive and performs one bounded scan:

1. read `start_time = arghtos_port_time_now()`;
2. store C9 `start_time`;
3. for every periodic descriptor (`period != 0`):
   * DORMANT -> READY;
   * set `next_time = start_time + period`;
4. leave async tasks DORMANT;
5. set the C9 STARTED flag while serialization is still active;
6. arm the platform timer for the earliest periodic `next_time`, or disarm if no periodic task exists;
7. exit scheduler serialization;
8. call `arghtos_port_start()`.

Thus every synchronous periodic task has release 0 at the common scheduler start epoch and is eligible for the first dispatch. A timer or interrupt cannot observe a half-started scheduler state.

# Descriptor validity rules

For every descriptor:

* `entry != NULL`;
* the top-level `crash_record` pointer is non-NULL and points to one writable 64-byte C5 record;
* `stack_base != NULL`;
* `stack_size > 0` and the complete target-specific region is accepted by `arghtos_port_stack_valid(stack_base, stack_size)`; minimum context reserve and alignment are port properties, not generic scheduler constants;
* stack regions are pairwise non-overlapping;
* `flags == 0`.

For a periodic descriptor:

* `period > 0`;
* `period < ARGHTOS_TIME_HALF_RANGE`;
* `async_priority == 0`.

For an async descriptor:

* `period == 0`;
* `1 <= async_priority <= ARGHTOS_ASYNC_PRIORITY_MAX`;
* `0x00` and `0xFF` are reserved and illegal for async tasks.

# Initialization sequence

Before returning `ARGHTOS_OK`:

1. validate the top-level pointer/count;
2. validate every descriptor, including the pure `arghtos_port_stack_valid()` check;
3. perform pairwise stack-overlap checks;
4. call `arghtos_port_init()` only after the external configuration is known valid;
5. copy persistent configuration pointers/count into C9;
6. initialize C8 runtime state;
7. build every initial opaque task context through C4;
8. leave the scheduler timer disarmed until start.

This ordering prevents a bad application descriptor from causing persistent port/hardware side effects before configuration validation completes.

On success:

* all async tasks begin DORMANT;
* all periodic tasks begin DORMANT and become due at scheduler start;
* no task has executed.

# Resident-memory accounting

The descriptor table is read-only by contract but is physically SRAM-resident and counts against the ArghtOS resident-footprint budget.

At 24 configured tasks:

```text
24 * 16 B = 384 B
```

Application task stacks are excluded from the ArghtOS overhead target by explicit requirement, but remain part of total device SRAM consumption.

Application task bodies are likewise not ArghtOS overhead; ArghtOS kernel/port code is.

The required 64-byte crash record is ArghtOS overhead.

# Dependencies

[C7](C7-common-types-abi.md), [C9](C9-kernel-global-state.md), [P0](../../proof/P0-static-configuration-validity.md), [P2](../../proof/P2-context-stack-integrity.md), [P6](../../proof/P6-rms-schedulability.md).

# Dangerous aspects

* Treating read-only data as free because it is `const`.
* Overlapping stacks.
* Stack regions that pass generic scalar checks but fail the selected port's context/alignment requirements.
* Reintroducing phase/deadline/budget fields without accounting their per-task SRAM cost.
* Treating task activation as allocation.

# Boundary specification

C0 defines immutable application-supplied scheduler metadata. It is read-only after initialization but is resident SRAM and is included in the scheduler-footprint accounting.

# Minimum-footprint generated profile

The generic 16-byte descriptor above remains the public/runtime-config ABI.

The active R2c minimum-footprint implementation uses generated build-time task
slots instead of retaining a separate generic descriptor array. In that profile,
configuration validity, stack layout, and task count are build/generator
evidence.

The R2c schedule encoding is:

```text
high bit clear  => periodic; low 31 bits are period
high bit set    => async; low byte is priority
```

The generated stack top is stored temporarily in the task slot's context field
and is overwritten by the target context token during initialization.

This specialization does not change task IDs, RMS ordering, async priority
semantics, or lifecycle behavior.

# R3c minimum-footprint production profile

The generic API above retains runtime lifecycle misuse checks.

The R3c minimum-footprint production profile instead treats these as build/application preconditions:

* `arghtos_init()` is called exactly once;
* `arghtos_start()` is called exactly once after successful init;
* generated task configuration and stack layout are valid by construction.

R3c still checks port initialization failure and initial-context construction failure.

This specialization does not change behavior for valid startup sequences.

# R5l generated prebuilt-context refinement

The trusted generated minimum-size profile may supply each configured task with
an already-built initial opaque target context instead of asking the portable
kernel to construct that context at runtime.

This refinement is allowed only when build/generator evidence establishes:

* the task stack region and alignment satisfy the selected port contract;
* the saved context token points inside that task's own stack;
* the prebuilt frame is byte-equivalent to the frame that the selected port's
  ordinary initial-context builder would produce for the same trampoline,
  return guard, and task entry;
* all mutable scheduler-owned runtime fields begin in their documented
  initialization state;
* unused generated slots, if any, begin zero/inactive;
* the application startup mechanism materializes the prebuilt frame before
  `arghtos_init()` executes.

For Cortex-M23 R5l, the application-owned stack RAM size is unchanged. The
trade is that the generated initial frame is load-initialized rather than
`NOLOAD`, increasing load-image/startup initialization bytes.

The public runtime-config ABI and generic/debug initialization path are
unchanged.

# R5m pristine generated startup state

The trusted generated prebuilt-context profile may also treat initial mutable
scheduler state as build/startup evidence rather than rewrite it in
`arghtos_init()`.

This refinement requires:

* `arghtos_init()` is called exactly once;
* every active generated context token is valid before the call;
* task next-time/age storage begins zero;
* packed task status begins DORMANT with no pending wake or missed releases;
* unused generated task slots, if any, begin zero/inactive;
* C4 `arghtos_port_init()` succeeds before the scheduler is considered
  initialized;
* application code does not invoke scheduler APIs after a failed init.

Under those premises, the R5m minimum profile does not rescan or clear generated
task state at runtime. It still propagates port initialization failure and
sets the live kernel current-task/initialized state after port initialization
succeeds.

The generic/debug initialization path continues to validate and initialize
runtime state explicitly.

# R5p generated class/order refinement

The trusted generated minimum-size profile may choose the generated descriptor
order so task class and RMS order are encoded by task ID.

For that profile the generator emits one constant periodic count `P` and
establishes:

```text
task IDs [0, P)  = periodic tasks
task IDs [P, N)  = async tasks
```

The periodic prefix is sorted by nondecreasing period. For equal periods, the
generator assigns increasing task IDs in the deterministic tie order used by
C6.

Task ID remains exactly the zero-based index of the generated task table. Any
application-facing generated task-ID constants refer to this final generated
index. The generic/runtime-config profile retains its existing descriptor
ordering and does not depend on this refinement.

The refinement therefore changes representation/order only in the trusted
generated profile; it does not change RMS priority semantics, async priority
semantics, lifecycle behavior, or the public task-ID type.

# R5s generated crash startup image

The trusted generated minimum-size profile may include C5's 64-byte crash
record in the generated startup image with the invariant header and zero
payload required by C5 R5s.

Build/generator evidence must establish that this image is materialized before
`arghtos_init()`. This is the same class of startup-image premise used by the
R5l/R5m generated task context/state refinements.

The crash record remains scheduler-owned writable storage and its full 64 bytes
remain part of the ArghtOS resident-footprint accounting.

# R6k preseeded periodic release-zero state

The trusted generated minimum-size profile may refine R5m pristine startup by
preseeding the packed state of every generated periodic task to READY while
leaving every generated async task DORMANT.

This is permitted only when the profile also proves that no supported scheduler
observer can execute before C9 STARTED is committed:

* R5n prevents dispatch before STARTED;
* R6b prevents scheduler-timer entry before STARTED;
* production interrupt-wake/service preconditions exclude lifecycle mutation
  before scheduler start;
* no public diagnostic state query is present in the no-profiling minimum
  profile.

The periodic READY encoding before STARTED is therefore an internal generated
representation of the synchronous release-zero transaction, not an early task
release visible to supported execution.

At `arghtos_start()`, the kernel still reads the common start epoch, computes
each periodic `next_time = start + period`, commits STARTED, arms the generated
first timer, and enters the port start path. The only removed work is rewriting
READY into metadata that already contains READY.

Generic/debug and ordinary R5m profiles retain DORMANT periodic metadata before
start and perform DORMANT -> READY at runtime.

# R6l pre-serialized start transaction

R6l does not remove serialization from the C0 start transaction. It moves
ownership of that serialization across the API boundary.

For the trusted generated profile, the caller enters `arghtos_start()` with
the C4 scheduler serialization state already active. The start transaction
still executes atomically:

1. read the common start epoch;
2. materialize runtime periodic deadlines;
3. commit STARTED;
4. arm the first scheduler timer;
5. transfer permanently to `arghtos_port_start()`.

Because interrupts remain masked through step 4, no timer, wake, SVC, or
PendSV path can observe a partially started scheduler. The port start path owns
the eventual transition back to normal interrupt eligibility.

Generic profiles continue to enter and exit serialization inside
`arghtos_start()`.

# R6o generated kernel-state seed

The trusted generated image may seed the minimum C9 kernel state with
`current_task=INVALID` before `arghtos_init()`.

This seed is static configuration evidence, analogous to other accepted
generated initial-state materialization. The runtime initializer still owns the
port initialization result and INITIALIZED flag commit.

After successful `arghtos_init()`, the observable kernel state is identical to
the ordinary minimum profile.

# R6q preseeded periodic deadline offsets

The trusted generated minimum-size profile may preload each periodic task's
runtime `next_time` storage with its generated period before scheduler start.

Before STARTED, that word is interpreted only as a relative offset from the
future common start epoch. R5n/R6b and the production lifecycle preconditions
already prove that no supported scheduler observer consumes the field as an
absolute deadline before start.

At `arghtos_start()`, the runtime transaction converts each offset in place:

```text
next_time += start_epoch
```

which yields exactly the ordinary first future release deadline:

```text
start_epoch + period
```

The generated schedule table remains unchanged and is still authoritative for
subsequent periodic catch-up. Generic profiles retain the ordinary
`next_time = start + period` materialization.

# R6aj generated direct async initial context

The trusted R5l/R5p/R5ab generated profile may specialize the prebuilt initial
context by task class.

For periodic IDs in \`[0,P)\`, the generated frame retains the ordinary
periodic task-start trampoline and passes the immutable application entry in
the initial argument register.

For the sole async ID \`P\`, the generated frame may instead place the
application entry directly in stacked PC and place the async-return fatal guard
in stacked LR.

This discharges task class at generation time rather than re-reading the raw
R5z schedule byte at runtime.

The generated frame still has exactly the same Cortex-M23 saved-context size,
alignment, stack ownership, and saved PSP token.

# R6ao periodic-only raw schedule table

The trusted generated R5z/R5ab/R6aj minimum profile may omit the raw schedule
byte for the sole asynchronous task.

R5p and R5ab establish:

~~~text
periodic IDs = [0, P)
async IDs    = {P}
N            = P + 1
~~~

R5z stores raw periodic periods in the periodic prefix and raw async priority
in the async suffix.

In the accepted single-async profile, the sole async priority has no remaining
runtime consumer:

* residual-capacity selection has only one async candidate and therefore does
  not compare async priorities;
* R6aj/R6ak discharge task class in the generated initial context and do not
  read the async schedule byte;
* periodic start/time processing accesses schedule values only for IDs in
  \`[0,P)\`.

The generated schedule8 table may therefore contain exactly P bytes, one period
for each periodic task. Generic, multi-async, and runtime-classification
profiles retain the full N-entry schedule table.

# R6ap omitted unit-period schedule entry

The trusted generated profile may omit task 0's schedule byte when all of the
following already hold:

* R5p generated ordering;
* R6ao periodic-only schedule8 storage;
* R6t proves task 0 has period exactly one scheduler tick;
* the remaining periodic task IDs are 1 through P-1.

The generated schedule object then stores only periods for periodic IDs 1..P-1:

~~~text
schedule8[0] = period(task 1)
...
schedule8[P-2] = period(task P-1)
~~~

Task 0's period is supplied by the R6t unit-period contract rather than stored
again in generated data.

Generic and non-unit-period profiles retain the ordinary periodic-only table.

# R6ar nibble-packed periodic deltas

The trusted generated profile may replace R6ap's byte-per-period table with
4-bit deltas between successive periodic periods when the generator proves:

* task 0 period is one tick;
* periodic IDs are sorted by nondecreasing period;
* for every periodic ID i in [1,P), \`period[i] - period[i-1] <= 15\`.

The generator stores the P-1 unsigned deltas in ID order, low nibble first:

~~~text
byte 0: delta(1) | delta(2)<<4
byte 1: delta(3) | delta(4)<<4
...
~~~

A zero delta is valid and represents equal-period RMS peers. The generated
task-ID order remains the equal-period tie break.

For P=23, the schedule object contains 22 deltas = 11 bytes.
