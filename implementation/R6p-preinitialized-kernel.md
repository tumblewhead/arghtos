---
type: ImplementationExperiment
title: "R6p - preinitialized generated kernel state"
description: "Accepted selected-port specialization preseeding INITIALIZED when Cortex-M23 port init is proved infallible."
tags: [implementation, footprint, init, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

For the selected generated Cortex-M23 profile, `arghtos_port_init()` is a
fixed bounded register/disarm sequence that returns `ARGHTOS_OK`
unconditionally.

The generated C9 image may therefore preseed:

```text
current_task = ARGHTOS_TASK_ID_INVALID
flags        = ARGHTOS_KF_INITIALIZED
```

and reduce `arghtos_init(config)` to executing the required port initializer
and returning its result.

Generic/fallible ports retain the runtime INITIALIZED commit.

# Authority

* `architecture/contracts/C4-context-switch-port.md`
* `architecture/contracts/C9-kernel-global-state.md`
* `proof/P0-static-configuration-validity.md`

# Measurement

PR #5 CI run:

```text
run:        40834001254
conclusion: success
```

Same-revision physical result:

```text
R6o control
  linked before MSP       1329 B
  ArghtOS-only MSP          64 B
  complete resident       1393 B
  arghtos_init              28 B

R6p
  linked before MSP       1320 B
  ArghtOS-only MSP          64 B
  complete resident       1384 B
  arghtos_init              20 B

symbol delta                -8 B
linked delta                -9 B
```

# Decision

Accept R6p as the current R6 minimum-size control at 1384 B.

R5ag remains the immutable merged R5 reference at 1534 B.
