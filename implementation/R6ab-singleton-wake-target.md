---
type: ImplementationExperiment
title: "R6ab - singleton generated wake target"
description: "Accepted trusted-profile specialization removing runtime wake-target plumbing for the sole generated async task."
tags: [implementation, footprint, wake, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

Generated ordering plus the single-async profile proves that every supported
wake targets:

```text
ARGHTOS_R5P_PERIODIC_COUNT
```

The minimum profile can therefore use that task ID directly rather than
transporting a runtime target through the public veneer, service dispatcher,
and compact wake helper.

# Authority

* `architecture/contracts/C1-task-api-lifecycle.md`
* `architecture/contracts/C2-wake.md`
* `proof/P1-task-state-invariants.md`

# Measurement

Authoritative physical-board CI:

```text
run:        40984337290
conclusion: success
```

Same-revision result:

```text
R6aa control
  linked before MSP       1320 B
  ArghtOS-only MSP          64 B
  complete resident       1384 B
  public arghtos_wake        8 B

R6ab
  linked before MSP       1316 B
  ArghtOS-only MSP          64 B
  complete resident       1380 B
  public arghtos_wake        6 B

linked delta                -4 B
```

`wake_locked_compact`, interrupt wake, and the generic service symbol already
compiled to equivalent sizes; the remaining linked gain comes from the target
plumbing/packing around the public wake veneer.

# Decision

Accept R6ab as the current physical-board R6 control at 1380 B.
