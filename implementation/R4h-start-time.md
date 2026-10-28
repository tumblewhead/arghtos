---
type: ImplementationCheckpoint
title: "R4h - Remove unused scheduler start epoch"
description: "Accepted minimum-size optimization removing kernel.start_time, which was written but never read."
tags: [implementation, footprint, kernel-state, accepted]
status: accepted
---

# Baseline

```text
R4g = 2773 B
```

# Observation

`kernel.start_time` was assigned during init/start/test reset but had no runtime
reader, public API exposure, or test dependency.

# Experiment

Omit the field and its stores only in the no-profile minimum-size production
profile.

# Verification

R4h host profile and all CI gates passed.

CI run 40660689158.

# Measured result

```text
R4g  2773 B
R4h  2770 B
-----------
saved    3 B
```

LLVM scalarizes the private kernel object, so the physical linked saving is
3 B rather than the nominal 4-byte struct field.

# Decision

Accept for minimum-size production. C9 must describe the start epoch as absent
from this profile.
