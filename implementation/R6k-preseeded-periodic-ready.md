---
type: ImplementationExperiment
title: "R6k - preseeded periodic READY startup"
description: "Accepted generated-startup specialization moving periodic release-zero READY materialization into static metadata."
tags: [implementation, footprint, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

R5m already treats pristine generated mutable state as build-time evidence.
R5n/R6b and production API preconditions prevent supported scheduler execution
from observing task state before STARTED.

R6k therefore preseeds every generated periodic task's packed state as READY
while leaving the async suffix DORMANT. At runtime, `arghtos_start()` still
forms each `next_time` from the actual common start epoch and commits STARTED;
it no longer rewrites READY into metadata that already contains READY.

# Authority

* `architecture/contracts/C0-static-task-configuration.md`
* `proof/P1-task-state-invariants.md`

# Measurement

PR #5 CI run:

```text
run:        40832108981
conclusion: success
```

Same-revision physical result:

```text
R6j control
  linked before MSP       1386 B
  ArghtOS-only MSP          64 B
  complete resident       1450 B
  arghtos_start             76 B

R6k
  linked before MSP       1370 B
  ArghtOS-only MSP          64 B
  complete resident       1434 B
  arghtos_start             60 B

linked delta               -16 B
start symbol delta          -16 B
```

The generated metadata remains 24 B; moving READY bits from runtime writes into
the initial image adds no resident bytes.

The host profile explicitly verifies periodic READY and async DORMANT before
start, then verifies the normal runtime start epoch and timer behavior.

# Decision

Accept R6k as the current R6 minimum-size control at 1434 B.

R5ag remains the immutable merged R5 reference at 1534 B.
