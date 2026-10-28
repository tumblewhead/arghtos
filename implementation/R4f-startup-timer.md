---
type: ImplementationCheckpoint
title: "R4f - Direct startup timer selection"
description: "Rejected neutral experiment selecting startup timer directly from minimum period."
tags: [implementation, footprint, startup, timer, rejected]
status: rejected
---

# Baseline

```text
R4b = 2781 B
```

# Experiment

At scheduler start, periodic next release is exactly `start + period`. Replace
the general timer-consider path with a direct minimum-period scan.

The target-only build initially exposed a dead `task_is_periodic()` helper;
that helper was conditionally removed before final measurement.

# Verification

Host startup/release tests passed. Final CI run 40660220690 passed.

# Measured result

```text
R4b  2781 B
R4f  2781 B
-----------
delta    0 B
```

# Decision

Reject. The cleaner specialized logic produces no linked saving in the final
same-revision target comparison.
