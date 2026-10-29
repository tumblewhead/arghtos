---
type: ImplementationExperiment
title: "R6q - preseeded periodic deadline offsets"
description: "Accepted generated-startup specialization storing each periodic period in next_time until the common start epoch is known."
tags: [implementation, footprint, startup, timer, generated-config, r6, accepted]
status: accepted
---

# Hypothesis

Before STARTED, no supported scheduler observer consumes periodic `next_time`
as an absolute deadline. The generated image can therefore preload each
periodic slot's `next_time` word with its period.

At the common runtime start epoch S:

```text
next_time += S
```

produces the same absolute first future release deadline `S + period` without
reloading the period from `schedule8`.

The async slot remains zero and the schedule table remains authoritative for
runtime periodic catch-up.

# Authority

* `architecture/contracts/C0-static-task-configuration.md`
* `proof/P4-time-release-correctness.md`

# Regression

The host profile verifies:

* before start: periodic `next_time == period`;
* before start: async `next_time == 0`;
* after start: periodic `next_time == start_epoch + period`;
* the first timer deadline is unchanged.

# Measurement

PR #5 CI run:

```text
run:        40834479467
conclusion: success
```

Same-revision physical result:

```text
R6p control
  linked before MSP       1320 B
  ArghtOS-only MSP          64 B
  complete resident       1384 B
  arghtos_start             48 B

R6q
  linked before MSP       1292 B
  ArghtOS-only MSP          64 B
  complete resident       1356 B
  arghtos_start             20 B

linked delta               -28 B
start symbol delta          -28 B
```

Generated metadata storage does not grow; the existing runtime word carries a
different pre-start interpretation.

# Decision

Accept R6q as the current R6 minimum-size control at 1356 B.

R5ag remains the immutable merged R5 reference at 1534 B.
