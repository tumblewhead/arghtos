---
type: ImplementationExperiment
title: "R6l - pre-masked scheduler start"
description: "Accepted generated Cortex-M23 specialization moving scheduler-start serialization ownership to the integration."
tags: [implementation, footprint, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

The trusted generated Cortex-M23 integration may enter `arghtos_start()`
already scheduler-serialized:

```text
Thread mode on MSP
PRIMASK = 1
scheduler timer disarmed
no active PendSV dispatch
```

The caller does not restore PRIMASK after transferring control. The start
transaction remains masked through runtime deadline materialization, STARTED
commit, and first timer arm. The non-returning `arghtos_port_start()` then
takes ownership and restores normal interrupt eligibility.

Generic profiles retain the ordinary critical enter/exit pair.

# Authority

* `architecture/contracts/C4-context-switch-port.md`
* `architecture/contracts/C0-static-task-configuration.md`
* `proof/P2-context-stack-integrity.md`

# Measurement

PR #5 CI run:

```text
run:        40832483244
conclusion: success
```

Same-revision physical result:

```text
R6k control
  linked before MSP       1370 B
  ArghtOS-only MSP          64 B
  complete resident       1434 B
  arghtos_start             60 B

R6l
  linked before MSP       1358 B
  ArghtOS-only MSP          64 B
  complete resident       1422 B
  arghtos_start             48 B

linked delta               -12 B
start symbol delta          -12 B
```

The host profile explicitly enters start masked and verifies that
`arghtos_port_start()` takes ownership and restores normal interrupt state.

# Decision

Accept R6l as the current R6 minimum-size control at 1422 B.

R5ag remains the immutable merged R5 reference at 1534 B.
