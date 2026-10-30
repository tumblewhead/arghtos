---
type: ImplementationExperiment
title: "R6ae - dispatch-owned YIELD READY transition"
description: "Accepted Cortex-M23 specialization deferring YIELD RUNNING->READY to the immediately-owned PendSV dispatcher."
tags: [implementation, footprint, service, dispatch, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

The Cortex-M23 SVC service path pends PendSV for YIELD and cannot resume Thread
mode through the API call before that dispatch executes.

The YIELD service therefore need not write READY itself. PendSV saves the
outgoing context and the dispatcher performs its existing RUNNING -> READY
preemption transition before selection.

# Authority

* `architecture/contracts/C1-task-api-lifecycle.md`
* `architecture/contracts/C4-context-switch-port.md`
* `proof/P1-task-state-invariants.md`

# Regression

The host seam intentionally exposes the otherwise-unobservable intermediate
state:

* YIELD service leaves current task RUNNING;
* exactly one dispatch request is pending;
* simulated PendSV dispatch saves the outgoing context;
* dispatcher performs RUNNING -> READY and reselects the task RUNNING.

On Cortex-M23, Thread mode cannot execute in that interval.

# Measurement

Authoritative physical-board CI:

```text
run:        40985917356
conclusion: success
```

Same-revision result:

```text
R6ad control
  linked before MSP          1296 B
  ArghtOS-only MSP             64 B
  complete resident          1360 B
  arghtos_kernel_service      144 B
  arghtos_kernel_dispatch     116 B

R6ae
  linked before MSP          1284 B
  ArghtOS-only MSP             64 B
  complete resident          1348 B
  arghtos_kernel_service      132 B
  arghtos_kernel_dispatch     116 B

linked delta                  -12 B
```

All host, sanitizer, portability, historical-reproduction, physical-control,
symbol-report, and artifact gates passed.

# Decision

Accept R6ae as the current physical-board R6 control at 1348 B.
