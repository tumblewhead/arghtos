---
type: ImplementationCheckpoint
title: "R4g - Remove dead dispatch time read"
description: "Accepted size-first optimization eliminating the scheduler clock read from dispatch when runtime profiling is absent."
tags: [implementation, footprint, dispatch, profiling, accepted]
status: accepted
---

# Baseline

```text
R4b = 2781 B
```

# Observation

After runtime profiling is compiled out, `arghtos_kernel_dispatch()` no longer
uses a dispatch timestamp. The function still called `arghtos_port_time_now()`
and discarded the result.

# Experiment

In the minimum-size no-profiling profile, do not perform the dead clock read.

No scheduler state transition, ordering rule, or public result changes.

# Verification

R4g host profile tests and all CI gates passed.

CI run 40660448008.

# Measured result

```text
R4b  2781 B
R4g  2773 B
-----------
saved    8 B
```

Symbol delta:

```text
arghtos_kernel_dispatch  160 -> 152 B
```

# Decision

Accept for minimum-size production.
