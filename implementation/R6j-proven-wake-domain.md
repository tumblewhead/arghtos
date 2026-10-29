---
type: ImplementationExperiment
title: "R6j - proven non-faulted wake domain"
description: "Accepted trusted-profile specialization removing unreachable FAULTED and unknown-state wake diagnostics."
tags: [implementation, footprint, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

The accepted terminal-only fault profile never resumes scheduler execution
after a fatal event and does not write FAULTED into task state. Combined with
the legal lifecycle transition proof and pristine generated initialization,
wake can observe only DORMANT, READY, RUNNING, WAITING, or SLEEPING.

The trusted minimum-profile wake helper can therefore omit:

* FAULTED -> BAD_STATE handling;
* unknown-state -> kernel invariant fatal handling.

Generic/debug builds retain both diagnostics.

# Authority

* `architecture/contracts/C1-task-api-lifecycle.md`
* `architecture/contracts/C2-wake.md`
* `proof/P1-task-state-invariants.md`

# Measurement

PR #5 CI run:

```text
run:        40831677055
conclusion: success
```

Same-revision physical result:

```text
R6i control
  linked before MSP       1406 B
  ArghtOS-only MSP          64 B
  complete resident       1470 B
  wake_locked_compact       72 B

R6j
  linked before MSP       1386 B
  ArghtOS-only MSP          64 B
  complete resident       1450 B
  wake_locked_compact       52 B

linked delta               -20 B
wake symbol delta           -20 B
```

All retained host, sanitizer, portability, generic Cortex-M23, frozen R5ag,
R6b-R6f, R6i, and R6j gates passed.

# Decision

Accept R6j as the current R6 minimum-size control at 1450 B.

R5ag remains the immutable merged R5 reference at 1534 B.
