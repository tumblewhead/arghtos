---
type: ImplementationExperiment
title: "R6ag - timer-owned SLEEP_UNTIL release processing"
description: "Accepted service-path specialization removing opportunistic time-engine processing from successful future sleep requests."
tags: [implementation, footprint, sleep, timer, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

Under the accepted period-1 timer-dominance profile, a successful future
SLEEP_UNTIL request cannot require an earlier timer than the scheduler already
has armed.

The SVC path may therefore store the absolute deadline, mark the sole async
worker SLEEPING, request dispatch, and leave elapsed-time release processing to
the scheduler timer interrupt.

# Timing trade

R6ag removes opportunistic periodic-release processing from SVC. A periodic
release already ideally due while timer delivery is delayed now waits for the
timer handler.

This remains inside the existing C3/P6 timer-jitter bound. Schedulability
analysis must use that timer-service release-jitter bound and must not assume
SVC reduces it.

# Authority

* `architecture/contracts/C1-task-api-lifecycle.md`
* `architecture/contracts/C3-time-timer.md`
* `proof/P4-time-release-correctness.md`
* `proof/P6-rms-schedulability.md`

# Regression

The host challenger deliberately delays timer service, then verifies:

* SLEEP_UNTIL does not catch up overdue periodic time;
* SVC does not rearm the already-due timer;
* the timer interrupt performs arithmetic periodic catch-up;
* a later timer interrupt still releases the async sleep at its absolute deadline.

# Measurement

Authoritative physical-board CI:

```text
run:        40989065832
conclusion: success
```

Same-revision result:

```text
R6ae control
  linked before MSP          1284 B
  ArghtOS-only MSP             64 B
  complete resident          1348 B
  arghtos_kernel_service      132 B

R6ag
  linked before MSP          1276 B
  ArghtOS-only MSP             64 B
  complete resident          1340 B
  arghtos_kernel_service      128 B

linked delta                   -8 B
service symbol delta            -4 B
```

# Decision

Accept R6ag as the current physical-board R6 control at 1340 B.
