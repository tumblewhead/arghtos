---
type: ImplementationExperiment
title: "R6ah - default-dispatch service control flow"
description: "Neutral control-flow inversion making dispatch the common trusted service epilogue."
tags: [implementation, footprint, service, cortex-m23, r6, rejected]
status: rejected-neutral
---

# Hypothesis

Under R6ad/R6ae/R6ag every admitted service dispatches except:

* WAIT consuming an already-pending wake;
* SLEEP_UNTIL with an already-due deadline.

R6ah early-returns from those two cases and makes dispatch the common epilogue,
removing the explicit dispatch boolean.

# Authority

* `architecture/contracts/C1-task-api-lifecycle.md`
* `proof/P1-task-state-invariants.md`

# Regression

The host profile verifies zero dispatch for both early-return cases and retains
the existing one-dispatch checks for dispatching service outcomes.

# Measurement

Authoritative physical-board CI:

```text
run:        40989861862
conclusion: success
```

Same-revision result:

```text
R6ag control
  linked before MSP       1276 B
  ArghtOS-only MSP          64 B
  complete resident       1340 B
  arghtos_kernel_service   128 B

R6ah
  linked before MSP       1276 B
  ArghtOS-only MSP          64 B
  complete resident       1340 B
  arghtos_kernel_service   128 B

delta                        0 B
```

# Decision

Reject R6ah as neutral. LLVM already emits an equivalent service control-flow
shape for R6ag.

Keep R6ag = 1340 B as the accepted physical control.
