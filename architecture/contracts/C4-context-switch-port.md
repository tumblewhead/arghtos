---
type: Contract
title: "C4 - Cortex-M23 execution and context port"
description: "Concrete MSP/PSP, SVC, PendSV, critical-section, initial-frame, and dispatch boundary."
tags: [architecture, contract, cortex-m23, context, psp, msp, pendsv, svc]
status: draft
---

# Key goals

* Keep all processor-specific mechanics behind one port.
* Run tasks in Thread mode on PSP.
* Run kernel/exception work on MSP.
* Make PendSV the only task-context handoff point.
* Define the exact saved task frame used by initialization and switching.
* Give C1/C2 one bounded atomicity primitive.

# Port-provided functions

```c
arghtos_result_t arghtos_port_init(void);
bool arghtos_port_stack_valid(void *stack_base, uint32_t stack_size);

arghtos_irq_key_t arghtos_port_critical_enter(void);
void arghtos_port_critical_exit(arghtos_irq_key_t key);

bool arghtos_port_is_task_context(void);
bool arghtos_port_is_handler_context(void);

void arghtos_port_request_dispatch(void);
void arghtos_port_start(void);
void arghtos_port_idle_wait(void);

uintptr_t arghtos_port_build_initial_psp(
    void *stack_base,
    uint32_t stack_size,
    arghtos_task_fn_t entry,
    void (*return_trampoline)(void));
```

The port also owns the vector-table implementations for SVC and PendSV.

# Port initialization

`arghtos_port_init()` is called exactly once by `arghtos_init()` before initial task frames are built.

On `ARGHTOS_OK`, it guarantees:

* the scheduler-visible timebase is ready;
* the scheduler timer source is disarmed;
* PendSV is configured at the least urgent scheduler-relevant exception priority;
* SVC/PendSV integration is usable;
* no scheduler task has been started.

It may return `ARGHTOS_ERR_BAD_CONFIG` if the target integration cannot satisfy the contract.

# Stack validation

`arghtos_port_stack_valid(base,size)` is a pure/bounded configuration check used by C0.

It returns true only if the region can hold a valid initial task frame under the target ABI, including required alignment and the 64-byte minimum saved frame.

It does not inspect overlap with other task stacks; C0 owns pairwise overlap validation.


# Critical section contract

`arghtos_port_critical_enter()` returns the prior mask state and prevents every maskable handler that is permitted to call ArghtOS from entering scheduler-state mutation.

Nested use must restore the exact prior state:

```c
arghtos_irq_key_t key = arghtos_port_critical_enter();
/* bounded scheduler mutation */
arghtos_port_critical_exit(key);
```

NMI and fatal fault exceptions are not masked and may not call normal scheduler APIs.

Critical sections must not contain:

* application callbacks;
* busy-waits for hardware;
* unbounded loops;
* formatted logging;
* sleep instructions.

# Context-query contract

`arghtos_port_is_task_context()` is true only when execution is Thread mode on a scheduler task PSP after scheduler start.

`arghtos_port_is_handler_context()` is true for supported maskable Handler-mode execution.

They are mutually exclusive for valid normal API calls.

# Dispatch request

`arghtos_port_request_dispatch()` makes PendSV pending.

It does not directly transfer execution and is safe to call repeatedly. Multiple requests collapse naturally into one pending PendSV.

PendSV must execute at the least urgent scheduler-relevant exception priority so device/timer handlers can complete before dispatch selection.

# Kernel dispatch callback

PendSV calls the C kernel through:

```c
uintptr_t arghtos_kernel_dispatch(uintptr_t outgoing_saved_psp);
```

Argument semantics:

* nonzero: saved PSP of the task that was executing before this dispatch;
* zero: no new outgoing context to save, used for first dispatch and re-selection after idle.

Return semantics:

* nonzero: saved PSP of the task selected to run;
* zero: no task is READY.

If zero is returned, the port calls `arghtos_port_idle_wait()` and then calls `arghtos_kernel_dispatch(0)` again.

```mermaid
flowchart TD
  P["PendSV on MSP"] --> SAVE["save outgoing software context"]
  SAVE --> KD["kernel_dispatch(saved_psp)"]
  KD --> Q{"incoming PSP?"}
  Q -- yes --> REST["restore incoming context"]
  REST --> RET["exception return to Thread/PSP"]
  Q -- no --> IDLE["port_idle_wait"]
  IDLE --> KD
```

# Idle wait

`arghtos_port_idle_wait()`:

* executes only with no READY task selected;
* uses no task stack;
* performs no scheduler-state mutation;
* returns after a scheduler-relevant interrupt/event gives the kernel reason to re-evaluate readiness;
* should use the lowest-cost architecture-supported wait mechanism, normally WFI.

The exact low-power policy remains a port decision.

# SVC ABI

Task-context ownership-changing calls use SVC.

The v1 service numbers are:

```c
typedef uint8_t arghtos_svc_id_t;

enum {
    ARGHTOS_SVC_YIELD       = 0u,
    ARGHTOS_SVC_WAIT        = 1u,
    ARGHTOS_SVC_SLEEP_UNTIL = 2u,
    ARGHTOS_SVC_WAKE        = 3u,
    ARGHTOS_SVC_TASK_RETURN = 4u
};
```

For services with one scalar argument, the argument is passed in R0 according to the Cortex-M C ABI. The result code is returned in R0 when the calling task later resumes.

The SVC handler validates the service number and forwards it to the kernel API gateway. It never selects or restores a task directly; it requests PendSV when a dispatch decision is needed.

# Saved task frame

Cortex-M23 basic exception entry provides the hardware frame:

```text
higher address
+-----------------+
| xPSR            |
| PC              |
| LR              |
| R12             |
| R3              |
| R2              |
| R1              |
| R0              |
+-----------------+  <- PSP after hardware stacking
```

The PendSV port additionally saves R4-R11 below that frame:

```text
higher address
| hardware frame  |  8 words
+-----------------+
| R11             |
| R10             |
| R9              |
| R8              |
| R7              |
| R6              |
| R5              |
| R4              |
+-----------------+  <- saved_psp
lower address
```

A complete dormant saved context therefore occupies 16 32-bit words, 64 bytes, before any ordinary task stack use.

# Initial context

`arghtos_port_build_initial_psp()`:

1. computes the highest 8-byte-aligned address within the provided stack;
2. verifies at least 64 bytes are available for the initial saved frame;
3. creates a frame equivalent to a preempted task;
4. sets R4-R12 and R0-R3 to zero;
5. sets stacked LR to `return_trampoline`;
6. sets stacked PC to `entry`;
7. sets stacked xPSR with the Thumb state bit set;
8. returns the saved PSP pointing at R4.

No task entry argument exists in v1.

# First start

`arghtos_port_start()`:

* is called once after successful C0 initialization;
* switches kernel execution into the dispatch mechanism;
* invokes `arghtos_kernel_dispatch(0)`;
* never returns during normal operation.

# Privilege model

All v1 tasks run privileged in Thread mode.

ArghtOS v1 does not depend on an MPU, MMU, or TrustZone security state separation. Unprivileged task execution is outside the v1 contract.

# PSP/MSP ownership


| Situation | Stack |
|---|---|
| application startup before scheduler start | MSP |
| task Thread mode | PSP |
| SVC | MSP |
| PendSV | MSP |
| timer/device handlers | MSP |
| fatal handlers | MSP |
| idle wait in scheduler | MSP / port-owned kernel execution |

# Failure behavior

Invalid context-frame metadata, impossible PSP alignment/range in diagnostic builds, unknown SVC number, or impossible dispatch return enters C5 fatal handling.

# Dependencies

[C7](C7-common-types-abi.md), [C5](C5-fault-diagnostics.md), [P2](../../proof/P2-context-stack-integrity.md), [P7](../../proof/P7-bounded-memory-and-work.md).

# Dangerous aspects

* Save/restore asymmetry.
* Incorrect initial frame.
* SVC changing task ownership directly instead of deferring to PendSV.
* PendSV priority configured above device/timer handlers.
* Critical-section restore failing to preserve prior mask state.
* Idle wait preventing scheduler-relevant interrupts from running.

# Boundary specification

C4 owns processor execution mechanics, special-register/exception handling, and critical sections. The kernel owns task state and scheduling policy.
