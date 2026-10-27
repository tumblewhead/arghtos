---
type: Contract
title: "C4 - Context-switch port"
description: "Cortex-M23 boundary for MSP/PSP ownership, exception entry, and task context save/restore."
tags: [architecture, contract, cortex-m23, context, psp, msp, pendsv, svc]
status: draft
---

# Goal

Contain architecture-specific register and exception mechanics behind a narrow port so scheduler policy remains ordinary C.

# Required behavior

* Thread-mode tasks execute using PSP.
* Scheduler and exception handlers execute using MSP.
* PendSV is the single context-switch point.
* SVC is the task-to-kernel gateway for ownership-changing task APIs.
* The outgoing task continuation is saved before its runtime record stops representing the executing context.
* The selected task continuation is restored only after policy has selected a READY task.
* Initial task context is synthesized so first dispatch and resumed dispatch converge on the same task-entry/return path.

# Context flow

```mermaid
sequenceDiagram
  participant T1 as Task A / PSP
  participant E as Exception entry
  participant P as PendSV / MSP
  participant K as Scheduler C core
  participant T2 as Task B / PSP
  T1->>E: preemption / SVC / IRQ return path
  E->>P: PendSV pending
  P->>P: save software context of A
  P->>K: select_next(saved_psp_A)
  K-->>P: next = B, psp_B
  P->>P: restore software context of B
  P-->>T2: exception return using PSP
```

# Dependencies

[P2](../../proof/P2-context-stack-integrity.md), [P7](../../proof/P7-bounded-memory-and-work.md).

# Dangerous aspects

* Incorrect exception-return value.
* Stack alignment failure.
* Save/restore asymmetry.
* Corrupt PSP accepted without validation in diagnostic builds.
* Scheduler code unexpectedly running on PSP.
* Nesting assumptions violated by interrupt priorities.
* Context code depending on compiler-generated stack frames in ways not captured by the port contract.

# Boundary specification

The port owns exception entry/exit, special registers, and register save/restore. The scheduler core owns task state and selection policy. Neither side may duplicate the other's responsibility.
