---
type: ImplementationExperiment
title: "R6ak - generated R4 assembly periodic trampoline"
description: "Accepted Cortex-M23 specialization seeding periodic entry in saved R4 and replacing the C trampoline with an 8-byte assembly loop."
tags: [implementation, correctness, footprint, trampoline, context, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

R6aj already discharges task class in the generated initial context. Periodic
frames can also place the immutable application entry directly in saved R4.

Because R4 is part of the ordinary PendSV-saved task context, a target assembly
trampoline can use it across every periodic suspension:

```asm
1:
    blx r4
    movs r0, #4
    svc #0
    b.n 1b
```

Service ID 4 is statically tied to `ARGHTOS_SERVICE_TASK_RETURN`. That service
does not inspect R1/arg0.

# Frame evidence

The physical build runs the generated-frame relocation audit in R4 mode:

```text
R6aj/R6ak prebuilt-context relocation check: PASS
(23 periodic frames via r4 + 1 direct async frame)
```

This verifies periodic entry relocations moved from stacked R0 to saved R4,
while periodic PCs still target the trampoline and the sole async PC still
targets the application entry directly.

# Authority

* `architecture/contracts/C1-task-api-lifecycle.md`
* `architecture/contracts/C4-context-switch-port.md`
* `proof/P1-task-state-invariants.md`
* `proof/P2-context-stack-integrity.md`

# Measurement

Authoritative physical-board CI:

```text
run:        40996040636
conclusion: success
```

Same-revision result:

```text
R6aj semantic control
  linked before MSP          1276 B
  ArghtOS-only MSP             64 B
  complete resident          1340 B
  task-start trampoline        18 B

R6ak
  linked before MSP          1264 B
  ArghtOS-only MSP             64 B
  complete resident          1328 B
  task-start trampoline         8 B
```

```text
linked delta                  -12 B
trampoline symbol delta       -10 B
```

The remaining two bytes are link/layout effects.

# Decision

Accept R6ak as the current semantic and physical R6 control at 1328 B.

The R6aj direct-async frame and async-return-fatal repair remain intact.
