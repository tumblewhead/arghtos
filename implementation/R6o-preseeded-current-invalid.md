---
type: ImplementationExperiment
title: "R6o - preseeded INVALID current task"
description: "Accepted generated-image specialization moving the initial current-task sentinel out of runtime initialization."
tags: [implementation, footprint, init, generated-config, r6, accepted]
status: accepted
---

# Hypothesis

The trusted generated image may materialize
`kernel.current_task = ARGHTOS_TASK_ID_INVALID` before `arghtos_init()`.

The INITIALIZED flag remains clear until `arghtos_port_init()` succeeds, so
runtime initialization success/failure semantics are unchanged.

# Authority

* `architecture/contracts/C9-kernel-global-state.md`
* `architecture/contracts/C0-static-task-configuration.md`
* `proof/P1-task-state-invariants.md`

# Measurement

PR #5 CI run:

```text
run:        40833663326
conclusion: success
```

Same-revision physical result:

```text
R6n control
  linked before MSP       1330 B
  ArghtOS-only MSP          64 B
  complete resident       1394 B
  arghtos_init              32 B

R6o
  linked before MSP       1329 B
  ArghtOS-only MSP          64 B
  complete resident       1393 B
  arghtos_init              28 B

symbol delta                -4 B
linked delta                -1 B
```

The generated C9 object remains 4 bytes. Section/layout effects consume three
bytes of the instruction saving, but the complete physical image is still one
byte smaller.

# Decision

Accept R6o as the current R6 minimum-size control at 1393 B.

R5ag remains the immutable merged R5 reference at 1534 B.
