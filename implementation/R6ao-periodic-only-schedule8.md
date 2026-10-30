---
type: ImplementationExperiment
title: "R6ao - periodic-only raw schedule table"
description: "Accepted generated-data specialization omitting the unused sole-async priority byte."
tags: [implementation, footprint, generated-config, schedule, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

In the accepted generated single-async profile, the sole async task's raw
schedule8 priority byte has no remaining runtime consumer.

R5p/R5ab prove the task set is a periodic prefix plus one async task. R6aj/R6ak
discharge class in the generated initial context, and the single-async selector
does not compare priorities.

The generated schedule8 object can therefore contain only the periodic prefix.

# Authority

* `architecture/contracts/C0-static-task-configuration.md`
* `proof/P0-static-configuration-validity.md`

# Measurement

Authoritative CI:

```text
run:        40999554567
conclusion: success
```

Same-revision physical result:

```text
R6am control
  linked before MSP          1256 B
  ArghtOS-only MSP             64 B
  complete resident          1320 B
  arghtos_static_schedule8      24 B

R6ao
  linked before MSP          1255 B
  ArghtOS-only MSP             64 B
  complete resident          1319 B
  arghtos_static_schedule8      23 B

delta                          -1 B
```

Host and target static assertions pin the schedule object to exactly the
periodic count.

# Decision

Accept R6ao as the current semantic and physical Cortex-M23 control at 1319 B.
