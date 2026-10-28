---
type: ImplementationCheckpoint
title: "R4o - Safe identical code folding"
description: "Rejected neutral linker experiment adding lld --icf=safe to the accepted R4n profile."
tags: [implementation, footprint, linker, icf, rejected]
status: rejected
---

# Baseline

```text
R4n = 2450 B
```

# Experiment

Add lld `--icf=safe` to the full-LTO, omit-frame-pointer minimum-size
Cortex-M23 profile.

# Measured result

CI run 40663421625 passed.

```text
R4n  2450 B
R4o  2450 B
-----------
delta    0 B
```

The symbol-size reports are identical.

# Decision

Reject. Keep the simpler linker command.
