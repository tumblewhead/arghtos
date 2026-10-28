---
type: ImplementationCheckpoint
title: "R4e - Shared timer sentinel"
description: "Rejected aggregate experiment replacing have_timer booleans with UINT32_MAX delta sentinels in start and runtime time processing."
tags: [implementation, footprint, timer, size-first, rejected]
status: rejected
---

# Baseline

```text
R4b = 2781 B
```

# Experiment

Use `UINT32_MAX` as the no-timer sentinel in both `arghtos_start()` and
`process_time_locked()`, removing the explicit `have_timer` boolean.

Host timer/release tests passed.

# Measured result

CI run 40659857312 passed.

```text
R4b  2781 B
R4e  2785 B
-----------
delta   +4 B
```

Symbol split:

```text
arghtos_start        124 -> 116 B   -8 B
process_time_locked  292 -> 304 B  +12 B
```

# Decision

Reject the shared transformation.

The start-path half is independently promising and should be re-expressed
without the general timer helper: at startup, next release is exactly
`start + period`, so the earliest deadline is determined by the smallest
period directly.
