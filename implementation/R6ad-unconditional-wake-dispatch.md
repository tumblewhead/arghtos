---
type: ImplementationExperiment
title: "R6ad - unconditional trusted wake dispatch"
description: "Accepted size-for-cycles specialization requesting one bounded dispatch after every trusted wake."
tags: [implementation, footprint, wake, scheduler, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

The trusted single-async profile can request one scheduler reevaluation after
every accepted WAKE rather than returning conditional dispatch intent.

Wake state/latch semantics remain unchanged:

* DORMANT/WAITING -> READY;
* READY unchanged;
* RUNNING remains RUNNING and sets wake_pending;
* SLEEPING remains SLEEPING and sets wake_pending.

Only dispatch-request policy changes.

# Runtime trade

READY/RUNNING/SLEEPING wakes may add one redundant PendSV/dispatcher pass.

The extra work is bounded and is accounted for in P6/P7. It does not make a
task eligible, consume the wake latch, or bypass the ordinary selector.

# Authority

* `architecture/contracts/C2-wake.md`
* `proof/P3-wake-wait-race-freedom.md`
* `proof/P6-rms-schedulability.md`
* `proof/P7-bounded-memory-and-work.md`

# Regression

The host profile explicitly verifies:

* DORMANT wake -> READY + one dispatch;
* repeated READY wake leaves READY, latch clear, + one dispatch;
* task-context WAKE uses the same dispatch policy;
* RUNNING wake leaves RUNNING, sets wake_pending, + one dispatch;
* WAIT consumes the latch without blocking;
* SLEEPING wake leaves SLEEPING, sets wake_pending, + one dispatch.

# Measurement

Authoritative physical-board CI:

```text
run:        40985479485
conclusion: success
```

Same-revision result:

```text
R6ac control
  linked before MSP              1308 B
  ArghtOS-only MSP                 64 B
  complete resident              1372 B
  wake_locked_compact              52 B
  arghtos_kernel_service          148 B
  arghtos_wake_from_interrupt      24 B

R6ad
  linked before MSP              1296 B
  ArghtOS-only MSP                 64 B
  complete resident              1360 B
  wake_locked_compact              44 B
  arghtos_kernel_service          144 B
  arghtos_wake_from_interrupt      24 B

linked delta                      -12 B
```

All host, sanitizer, portability, historical-reproduction, physical-control,
symbol-report, and artifact gates passed.

# Decision

Accept R6ad as the current physical-board R6 control at 1360 B.
