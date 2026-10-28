---
type: ImplementationCheckpoint
title: "R4w - Disable automatic inlining"
description: "Rejected compiler experiment adding -fno-inline-functions to the accepted R4v minimum-size profile."
tags: [implementation, footprint, compiler, inline, rejected]
status: rejected
---

# Baseline

```text
R4v = 2298 B
```

# Experiment

Add `-fno-inline-functions` to all target C compilation and LTO link commands.

The intent was to spend call cycles in exchange for less duplicated code.

# Measured result

CI passed all build and verification gates.

```text
R4v  2298 B
R4w  2594 B
-----------
delta +296 B
```

Representative decomposition:

```text
dispatch 296 B -> dispatch 168 B + selector 168 B
process  256 B -> process 208 B + timer/time/missed helpers
fatal    232 B -> fatal 164 B + fault/port-crash/read helpers
```

The current LTO + -Oz inliner is therefore already making substantially better
size choices than a global no-inline policy.

# Decision

Reject R4w. Retain R4v compiler settings.
