---
type: ImplementationCheckpoint
title: "R4c - Looped Cortex-M23 crash-frame copy"
description: "Rejected size-first experiment replacing explicit crash-frame stores with loops."
tags: [implementation, footprint, cortex-m23, crash, rejected]
status: rejected
---

# Baseline

```text
R4b = 2781 B
```

# Experiment

Replace the seven explicit basic-frame stores in
`arghtos_port_crash_fill()` with two small loops.

The runtime cost was allowed to increase if resident code shrank.

# Measured result

CI run 40659438965 passed.

```text
R4b  2781 B
R4c  2789 B
-----------
delta   +8 B
```

Only the Cortex-M23 port changed:

```text
arghtos_port_crash_fill  84 B -> 92 B
```

# Decision

Reject.

On Cortex-M23 Thumb code at `-Oz`, the explicit seven-store form is smaller
than the two-loop form. Do not carry the R4c implementation forward.
