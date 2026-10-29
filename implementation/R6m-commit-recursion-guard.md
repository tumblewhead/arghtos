---
type: ImplementationExperiment
title: "R6m - crash commit-word recursion guard"
description: "Accepted trusted-profile crash specialization fusing recursive-fatal protection with commit invalidation."
tags: [implementation, footprint, crash, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

R5s guarantees the generated crash record begins each runtime with
`commit_magic = 0`. A record is valid only when that word exactly equals
`ARGHTOS_CRASH_COMMIT_MAGIC`.

R6m uses the same word as the recursive-fatal guard:

```text
0             first fatal may begin
1             capture in progress; record invalid
COMMIT_MAGIC  capture complete; record valid
nonzero       any second/recursive fatal terminates immediately
```

This removes the separate C9 FAULT_ACTIVE test/store from the trusted minimum
profile while retaining bounded recursive-fatal handling and commit-last crash
integrity.

# Authority

* `architecture/contracts/C5-fault-diagnostics.md`
* `proof/P7-bounded-memory-and-work.md`

# Fault injection

The host port can deliberately re-enter `arghtos_fatal()` from
`arghtos_port_crash_fill()`.

The R6m regression requires:

* exactly one crash-fill entry;
* recursive fatal transfers directly to the terminal hook;
* no second capture begins;
* `commit_magic` remains the invalid in-progress marker.

# Measurement

PR #5 CI run:

```text
run:        40833006113
conclusion: success
```

Same-revision physical result:

```text
R6l control
  linked before MSP       1358 B
  ArghtOS-only MSP          64 B
  complete resident       1422 B
  arghtos_fatal            156 B

R6m
  linked before MSP       1350 B
  ArghtOS-only MSP          64 B
  complete resident       1414 B
  arghtos_fatal            148 B

linked delta                -8 B
fatal symbol delta           -8 B
```

# Decision

Accept R6m as the current R6 minimum-size control at 1414 B.

R5ag remains the immutable merged R5 reference at 1534 B.
