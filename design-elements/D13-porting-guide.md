---
type: PortingGuide
title: "D13 - ArghtOS porting guide"
description: "Step-by-step guide for implementing a new CPU/MCU port without modifying the portable scheduler core."
tags: [design, porting, portability, target, integration]
status: current
---

# Goal

A new target must be supportable without editing:

```text
src/arghtos_kernel.c
src/arghtos_internal.h
include/arghtos/arghtos.h
```

The target-specific implementation lives under:

```text
port/<target>/
```

and implements the architecture-neutral contract declared by:

```text
include/arghtos/arghtos_port.h
```

The target may call back into the portable kernel only through:

```text
src/arghtos_kernel_port.h
```

If a new target requires a target-specific change in `arghtos_kernel.c`, stop and treat that as a port-ABI design problem rather than adding a target `#ifdef`.

# File template for a new port

Recommended minimum:

```text
port/<target>/
├── arghtos_port.c
├── arghtos_context.S          # optional if C/intrinsics are sufficient
├── arghtos_port_private.h
└── README.md                  # target-specific integration notes
```

A target may split timer/board glue into another file if necessary, but all nonportable code stays inside the target directory.

# The two interfaces a porter must understand

## 1. arghtos_port.h: kernel -> port

The portable kernel calls the selected port.

The target must implement:

```c
typedef uintptr_t arghtos_context_t;
typedef uintptr_t arghtos_irq_key_t;

arghtos_result_t arghtos_port_init(void);
bool arghtos_port_stack_valid(void *stack_base, uint32_t stack_size);

arghtos_time_t arghtos_port_time_now(void);
void arghtos_port_timer_arm(arghtos_time_t absolute_deadline);
void arghtos_port_timer_disarm(void);

arghtos_irq_key_t arghtos_port_critical_enter(void);
void arghtos_port_critical_exit(arghtos_irq_key_t key);

bool arghtos_port_is_task_context(void);
bool arghtos_port_is_handler_context(void);

arghtos_result_t arghtos_port_service_call(
    uint8_t service_id,
    uint32_t arg0);

void arghtos_port_request_dispatch(void);
void arghtos_port_start(void);
void arghtos_port_idle_wait(void);

arghtos_context_t arghtos_port_build_initial_context(
    void *stack_base,
    uint32_t stack_size,
    void (*thread_start)(void),
    void (*return_guard)(void));

void arghtos_port_crash_fill(
    arghtos_crash_record_t *record,
    const void *port_fault_context);

_Noreturn void arghtos_port_fault_terminal(void);
```

## 2. arghtos_kernel_port.h: port -> kernel

The selected port may call only:

```c
arghtos_context_t arghtos_kernel_dispatch(
    arghtos_context_t outgoing_context);

arghtos_result_t arghtos_kernel_service(
    uint8_t service_id,
    uint32_t arg0);

void arghtos_timer_interrupt(void);

_Noreturn void arghtos_fatal(
    arghtos_fault_reason_t reason,
    const void *port_fault_context);
```

A port must not reach into `controls[]`, C9 kernel state, lifecycle helpers, or selector internals.

# Context-token contract

`arghtos_context_t` is opaque to the kernel.

Required semantics:

```text
ARGHTOS_CONTEXT_NONE == 0

nonzero token:
    identifies enough saved execution state for the port to resume one task

kernel:
    stores token
    compares only to zero
    returns token to port
    never decodes it
```

Examples:

* Cortex-M23 reference port: token is saved PSP.
* A different CPU might use a pointer to a saved register frame.
* A host test port may use a small synthetic integer.

The token must fit in `uintptr_t`.

# Step 1: define task execution mode

Document:

* how a task executes;
* what stack pointer/context object represents it;
* whether tasks are privileged/unprivileged;
* which execution mode kernel/interrupt code uses;
* what architectural state must survive preemption.

The port must preserve all state required by the target ABI for arbitrary C task code.

# Step 2: define context save/restore

Identify:

* hardware-automatically saved registers;
* software-saved registers;
* alignment/padding rules;
* exception/trap return metadata;
* optional FP/vector state if present.

Then define the opaque context token returned to the kernel.

Port-specific context details belong only in the target README/private header/assembly.

# Step 3: implement initial-context construction

`arghtos_port_build_initial_context()` must construct a dormant task context equivalent to a task that can be restored by the normal context-restore path.

It must arrange:

```text
first execution -> thread_start
unexpected return -> return_guard
```

The initial path and resumed path should share as much restore code as possible.

# Step 4: implement task service calls

Portable APIs call:

```text
arghtos_port_service_call(service_id,arg0)
```

The port chooses the mechanism.

Examples:

* Cortex-M23: fixed SVC trap.
* RISC-V: ECALL.
* another architecture: software interrupt/trap.
* host fake port: direct C call into `arghtos_kernel_service()`.

Requirements:

* service ID and arg0 arrive unchanged at `arghtos_kernel_service()`;
* result returns to the calling task;
* if service pends a dispatch, the target may switch tasks before the caller resumes;
* portable code does not know the mechanism.

# Step 5: implement dispatch request and context handoff

`arghtos_port_request_dispatch()` marks that scheduling must be reconsidered.

The actual task handoff must converge on exactly one port path.

That path:

1. serializes scheduler-state mutation;
2. saves outgoing target context if one exists;
3. calls `arghtos_kernel_dispatch(outgoing_context)`;
4. restores the returned context if nonzero;
5. enters the target's race-free idle protocol if zero.

The port must never implement scheduling policy itself.

# Step 6: implement scheduler serialization

`arghtos_port_critical_enter/exit()` must serialize every interrupt/handler context permitted to mutate ArghtOS state.

The returned key must restore the exact previous masking/locking state so nesting is safe.

Questions a porter must answer:

* Which interrupts can call `arghtos_wake_from_interrupt()`?
* Can any of them preempt a scheduler critical section?
* What architecture primitive blocks exactly those contexts?
* Does the primitive work correctly when already active/nested?

# Step 7: implement race-free idle

The target must not use a naive:

```text
if no work:
    enable interrupts
    sleep
```

Port-specific idle design must prove that an event cannot become READY in the gap between the last no-work decision and sleep.

Document the architecture-specific rule used to close that race.

For Cortex-M23, the reference port uses PRIMASK plus WFI wake semantics.

# Step 8: implement time

The port must present a scheduler-visible modulo-2^32 monotonic clock:

```c
arghtos_time_t arghtos_port_time_now(void);
```

and one-shot absolute timer operations.

If hardware time is narrower, the port extends it.

The kernel must not know timer register width, prescalers, compare channels, vendor IRQ names, or oscillator details.

Port documentation must state:

* `ARGHTOS_TIME_HZ`;
* maximum timer delivery jitter;
* physical timer width;
* how long horizons are bridged if needed;
* clock-change behavior.

# Step 9: implement crash payload

The portable crash record reserves:

```text
port_format
port_words
port_state[10]
```

The target defines its own payload format.

Port rules:

* `port_words <= 10`;
* generic header fields are not modified;
* commit marker is not modified;
* architecture register extraction stays entirely inside the port;
* a NULL `port_fault_context` must be handled safely.

The target README must document every word in its payload.

# Step 10: implement terminal fault behavior

`arghtos_port_fault_terminal()` must not return.

Typical choices:

* reset;
* halt;
* watchdog-triggered reset;
* board-defined fatal loop.

It must not invoke normal scheduler APIs or depend on a task stack.

# Step 11: implement context predicates

`arghtos_port_is_task_context()` and `arghtos_port_is_handler_context()` support API misuse checks.

Document exactly which execution contexts return true.

A target with no meaningful handler/task distinction may still implement the abstraction, but it must preserve the public API legality rules.

# Step 12: validate stack requirements

`arghtos_port_stack_valid()` checks target-specific minimum/alignment requirements.

The port documentation must state:

* fixed scheduler context reserve;
* alignment;
* any red-zone or ABI special rules;
* whether exception entry can add padding;
* whether optional target state changes stack demand.

# Required new-port tests

A port is not accepted until these pass.

## Context preservation

* unique canaries in every callee-saved register;
* repeated forced preemption;
* voluntary yield;
* sleep/resume;
* interrupt wake;
* same-task reselect.

## Initial context

* first entry reaches `thread_start`;
* return guard is valid;
* stack alignment is valid;
* initial register state matches documented values.

## Service call

For every private service ID:

* ID arrives exactly;
* arg0 arrives exactly;
* result returns exactly;
* dispatch-capable service may suspend and later resume caller.

Unknown IDs must reach the kernel's BAD_SERVICE fatal path.

## Serialization

* nested enter/exit restores original state;
* scheduler-mutating interrupt cannot run inside serialized section;
* no early unmask.

## Idle race

Inject an interrupt/event:

* before no-work decision;
* immediately after decision;
* just before sleep instruction;
* while sleeping.

READY work must never be stranded while the CPU sleeps.

## Time

* wraparound;
* earliest deadline;
* timer disarm/rearm;
* delivery jitter measurement;
* long-horizon behavior.

## Crash capture

* NULL port context;
* architecture exception context;
* every payload word verified;
* recursive fault;
* commit-last validity.

## Portability compile test

Compile unchanged:

```text
src/arghtos_kernel.c
```

against:

* host port;
* new target port.

No target preprocessor branch is allowed in portable source.

# Required new-port documentation

Every `port/<target>/README.md` should include:

1. target architecture/compiler assumptions;
2. task/kernel execution modes;
3. context-token representation;
4. context frame layout;
5. minimum task stack reserve;
6. serialization primitive;
7. service-call mechanism;
8. dispatch mechanism;
9. idle race solution;
10. clock/timer source and jitter;
11. crash payload format;
12. exception/interrupt priority rules;
13. ArghtOS-only MSP/kernel-stack requirement;
14. known limitations;
15. verification evidence and test results;
16. resident footprint attributable to the port.

# Cortex-M23 reference port

The reference implementation demonstrates the pattern:

```text
generic concept        Cortex-M23 realization
------------------------------------------------
context token          saved PSP
service call           SVC #0, ID in R0, arg in R1
dispatch request       pend PendSV
context handoff        PendSV
serialization          PRIMASK
kernel stack           MSP
task stack             PSP
idle                   masked race-free WFI protocol
crash payload          MSP/PSP/EXC_RETURN + basic frame subset
time                   board-selected monotonic timer adapter
```

This mapping is documentation for the reference port, not portable-kernel behavior.

# Dangerous porting mistakes

## 1. Adding target #ifdefs to arghtos_kernel.c

This makes every future port a combinatorial maintenance problem.

**Review rule:** reject target branches in portable kernel; extend the abstract port ABI if genuinely necessary.

## 2. Port reads/writes C8 directly

A port may be tempted to inspect task state to optimize context switching.

That creates two owners for scheduler semantics.

**Test/review:** only kernel callbacks may decide state or winner; port treats context token as execution mechanism only.

## 3. Port duplicates the timer policy

A target timer ISR that directly decides which task is due duplicates D05.

**Correct pattern:** target timer event calls `arghtos_timer_interrupt()`.

## 4. Port returns architecture pointers to public application code

Context/frame types are private to the port.

Public ArghtOS API remains architecture-neutral.

## 5. New port silently increases generic structure sizes

A context token wider than the Cortex reference changes C8 size and footprint.

That may be acceptable for another target, but the port must report its own structure sizes and cannot claim the Cortex-M23 1.5 KiB footprint result.

## 6. Host port cheats around the ABI

If the host port calls private lifecycle helpers directly instead of `arghtos_kernel_service()`, tests can pass while a real port fails.

**Rule:** host port must exercise the same abstract port ABI as hardware ports.

# Port-completion gate

A port is considered complete when:

* portable kernel compiles unchanged;
* all abstract port symbols are implemented exactly once;
* no unresolved target assumptions remain in portable code;
* context, service, timer, idle, crash, and serialization tests pass;
* target README is complete;
* target footprint and stack evidence are recorded.
