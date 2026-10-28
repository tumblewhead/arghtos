---
type: ImplementationCheckpoint
title: "R5n - Proof-discharged dispatch and trampoline checks"
description: "Accepted trusted generated-profile specialization removing dispatcher/trampoline assertions that are inductive consequences of C0/C4/P1."
tags: [implementation, footprint, proof, dispatch, accepted]
status: accepted
---

# Baseline

```text
R5m = 1782 B
```

# Proof boundary

Generic/debug profiles retain all runtime corruption checks.

R5n removes only checks discharged by C0/C4/P1 for the trusted generated
minimum profile:

* dispatch is entered only after STARTED is committed;
* nonzero outgoing context implies a previously selected configured task;
* zero outgoing context is first-dispatch/idle with INVALID current task;
* selector returns INVALID or a READY configured task;
* generated saved contexts are nonzero/valid;
* the private trampoline begins from a dispatcher-selected configured task with
  a non-NULL generated entry;
* specialized periodic TASK_RETURN returns OK or enters a non-returning fatal
  path.

Unknown service IDs, fatal recursion, wake FAULTED handling, and timer-state
invariants remain checked.

# Verification

Authoritative CI:

```text
40723999331  success
```

All host, sanitizer, portability, and Cortex-M23 comparison gates passed.

# Measured result

```text
R5m  1782 B
R5n  1722 B
-----------
saved   60 B
```

Main symbol changes:

```text
arghtos_kernel_dispatch       180 -> 148 B
arghtos_task_start_trampoline  48 ->  18 B
```

All task/config state remains 240 B. The 72 B ArghtOS-only MSP bound remains
valid.

# Decision

Accept R5n for the trusted generated minimum-size profile.

```text
linked before MSP  1650 B
MSP                  72 B
-------------------------
R5n total          1722 B
soft target        1536 B
remaining gap       186 B
```

The minimum profile no longer claims runtime diagnostic detection for these
proof-impossible combinations if a trusted premise or arbitrary memory
integrity is violated. Generic/debug profiles retain that diagnostic coverage.
