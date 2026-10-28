---
type: ImplementationCheckpoint
title: "R4j - Range-coded task fault attribution"
description: "Rejected neutral experiment replacing four fault-code comparisons with one contiguous-range test plus CPU-exception comparison."
tags: [implementation, footprint, fatal, codegen, rejected]
status: rejected
---

# Baseline

```text
R4h = 2770 B
```

# Experiment

The task-attributed kernel fault codes 3, 4, and 5 are contiguous. Test:

```c
(uint16_t)(reason - 3u) <= 2u
```

plus the CPU-exception code, instead of four explicit equality comparisons.

Dedicated host tests verified the exact attributed set.

# Measured result

CI run 40661343619 passed.

```text
R4h  2770 B
R4j  2770 B
-----------
delta    0 B
```

# Decision

Reject. Keep the clearer explicit fault-reason comparisons.
