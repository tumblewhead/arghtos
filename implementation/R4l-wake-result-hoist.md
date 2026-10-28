---
type: ImplementationCheckpoint
title: "R4l - Hoist common wake success result"
description: "Rejected neutral experiment moving the common ARGHTOS_OK result store out of three wake-state branches."
tags: [implementation, footprint, wake, codegen, rejected]
status: rejected
---

# Baseline

Legacy attributed baseline before physical-padding correction:

```text
R4h = 2770 B
```

# Experiment

In `wake_locked()`, write `*result = ARGHTOS_OK` once before the state switch
instead of separately in DORMANT/WAITING, READY, and RUNNING/SLEEPING branches.

Host wake-state tests passed.

# Measured result

CI run 40662127141 passed the corrected experiment.

```text
wake_locked  112 B -> 112 B
total         unchanged
```

# Decision

Reject. Clang already coalesces the common stores efficiently.
