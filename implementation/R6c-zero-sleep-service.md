---
type: ImplementationExperiment
title: "R6c - zero-duration sleep via due-deadline service path"
description: "Accepted minimum-profile wrapper simplification after R6b."
tags: [implementation, footprint, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

The explicit `arghtos_sleep_for(0)` fast path is semantically redundant in the
minimum profile because the existing sleep-until service path treats an
absolute deadline equal to `now` as already due.

R6c therefore routes zero duration through:

```text
time_now -> SVC SLEEP_UNTIL(now) -> due check -> ARGHTOS_OK
```

The task remains RUNNING and no dispatch is requested.

# Regression

The static-profile host suite explicitly checks:

* `arghtos_sleep_for(0) == ARGHTOS_OK`;
* the async task remains RUNNING;
* no dispatch request is emitted.

# Measurement

PR #5 CI run:

```text
run:        40816160631
conclusion: success
```

Same-revision physical result:

```text
R6b control
  linked before MSP       1458 B
  ArghtOS-only MSP          64 B
  complete resident       1522 B
  arghtos_sleep_for         14 B

R6c
  linked before MSP       1450 B
  ArghtOS-only MSP          64 B
  complete resident       1514 B
  arghtos_sleep_for          8 B

linked delta                -8 B
symbol delta                -6 B
```

The additional two linked bytes come from packing/alignment effects.

All retained host, sanitizer, portability, generic Cortex-M23, frozen reference,
R6b control, and R6c challenger gates passed.

# Decision

Accept R6c as the current R6 minimum-size control at 1514 B.

R5ag remains the immutable merged R5 reference at 1534 B.
