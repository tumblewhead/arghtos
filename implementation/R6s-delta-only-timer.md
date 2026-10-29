---
type: ImplementationExperiment
title: "R6s - physical delta-only timer selection"
description: "Accepted physical-board specialization retaining only the minimum timer delta."
tags: [implementation, footprint, timer, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

The always-armed timer scan does not need to retain both the winning absolute
deadline and its unsigned distance from `now`.

For each candidate `t`:

```text
delta = t - now
```

R6s retains only the minimum `delta` and arms `now + best_delta`.

Modulo-2^32 arithmetic reconstructs the exact winning deadline, so behavior is
identical to the ordinary absolute-deadline path.

# Authority

* `architecture/contracts/C3-time-timer.md`
* `proof/P4-time-release-correctness.md`

# Measurement

Authoritative physical-board CI:

```text
run:        40922680748
conclusion: success
```

Same-revision result:

```text
R6r physical control
  linked before MSP       1428 B
  ArghtOS-only MSP          64 B
  complete resident       1492 B
  process_time_locked      252 B

R6s
  linked before MSP       1416 B
  ArghtOS-only MSP          64 B
  complete resident       1480 B
  process_time_locked      240 B

linked delta               -12 B
time-engine symbol delta    -12 B
```

Historical legacy-harness controls also reproduced exactly in the same run.

# Decision

Accept R6s as the current physical-board R6 control at 1480 B.
