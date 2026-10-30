---
type: ImplementationExperiment
title: "R6ai - generated trampoline class repair"
description: "Correctness-restoring generated-ID class test exposing the R5z raw-schedule async trampoline defect."
tags: [implementation, correctness, footprint, trampoline, cortex-m23, r6]
status: correctness-control-regressive
---

# Finding

R5z stores raw period bytes in the periodic prefix and raw async-priority bytes
in the async suffix. The shared task-start trampoline nevertheless classified
its current task using:

```c
ARGHTOS_TASK_PERIOD(id) != 0u
```

For the canonical fixture the sole async task stores priority 3, so that test
is true and the async task enters the periodic TASK_RETURN loop instead of
treating application return as fatal.

R6ai repairs the class decision using the already-proved generated ID boundary:

```text
id < ARGHTOS_R5P_PERIODIC_COUNT  => periodic
otherwise                        => async
```

# Measurement

Authoritative physical-board CI:

```text
run:        40990461247
conclusion: success
```

Same-revision result:

```text
R6ag historical size control
  complete resident                1340 B
  arghtos_task_start_trampoline      18 B

R6ai correctness repair
  complete resident                1360 B
  arghtos_task_start_trampoline      40 B

delta                               +20 B
```

# Interpretation

The 20-byte increase is not evidence that R6ai should be discarded as an
ordinary size regression. It is the measured cost of restoring the missing
runtime async-class branch.

The historical R6ag 1340 B image remains useful size evidence but is not a
valid semantic control for the "async return is fatal" invariant.

# Decision

Keep R6ai = 1360 B as the current **correctness control** while testing a
generated initial-context repair that discharges task class before runtime.

Do not merge the R5z-derived minimum profile as semantically complete until a
repair preserving async-return-fatal behavior is accepted.
