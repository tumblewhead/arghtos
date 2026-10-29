---
type: ImplementationExperiment
title: "R6n - dispatch-only trusted service outcome"
description: "Accepted trusted-profile specialization proving every reachable service result is OK and retaining only dispatch intent."
tags: [implementation, footprint, service, wake, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

R6f closes the private service-ID domain, generated lifecycle ownership removes
task-class/target misuse, minimal validation excludes invalid timing inputs,
and R6j removes the only remaining reachable wake error state.

Therefore every service call admitted by the trusted generated minimum profile
returns `ARGHTOS_OK`. The internal service/wake outcome needs to carry only
dispatch intent.

Generic/debug profiles retain the full result/error domain.

# Authority

* `architecture/contracts/C1-task-api-lifecycle.md`
* `architecture/contracts/C2-wake.md`
* `proof/P1-task-state-invariants.md`

# Measurement

PR #5 CI run:

```text
run:        40833277119
conclusion: success
```

Same-revision physical result:

```text
R6m control
  linked before MSP                  1350 B
  ArghtOS-only MSP                     64 B
  complete resident                  1414 B
  wake_locked_compact                  52 B
  arghtos_wake_from_interrupt          28 B
  arghtos_kernel_service              156 B

R6n
  linked before MSP                  1330 B
  ArghtOS-only MSP                     64 B
  complete resident                  1394 B
  wake_locked_compact                  52 B
  arghtos_wake_from_interrupt          24 B
  arghtos_kernel_service              140 B

linked delta                          -20 B
interrupt-wake delta                   -4 B
service delta                         -16 B
```

# Decision

Accept R6n as the current R6 minimum-size control at 1394 B.

R5ag remains the immutable merged R5 reference at 1534 B.
