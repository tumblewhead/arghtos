---
type: ImplementationExperiment
title: "R6u - singleton async release under unit-period timer"
description: "Accepted physical-board recomposition of the singleton async time suffix after R6t removed timer-candidate tracking."
tags: [implementation, footprint, timer, async, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

The earlier R6h singleton-async specialization was neutral in its old code
context. R6t materially changes that context by removing all timer-candidate
tracking from the async suffix.

With exactly one generated async task, the suffix now only checks that one task
for a due SLEEPING -> READY transition. R6u replaces the one-iteration loop by
one direct slot operation.

# Authority

* `architecture/contracts/C3-time-timer.md`
* `proof/P4-time-release-correctness.md`

# Measurement

Authoritative physical-board CI:

```text
run:        40923612503
conclusion: success
```

Same-revision result:

```text
R6t physical control
  linked before MSP       1360 B
  ArghtOS-only MSP          64 B
  complete resident       1424 B
  process_time_locked      184 B

R6u
  linked before MSP       1356 B
  ArghtOS-only MSP          64 B
  complete resident       1420 B
  process_time_locked      180 B

linked delta                -4 B
time-engine symbol delta     -4 B
```

All host, sanitizer, portability, historical-reproduction, physical-control,
symbol-report, and artifact gates passed.

# Decision

Accept R6u as the current physical-board R6 control at 1420 B.
