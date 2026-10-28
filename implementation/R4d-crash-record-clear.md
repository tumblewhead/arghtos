---
type: ImplementationCheckpoint
title: "R4d - Whole-prefix crash-record clear"
description: "Rejected neutral experiment replacing individual crash-record zero stores with a bytewise prefix clear loop."
tags: [implementation, footprint, crash, size-first, rejected]
status: rejected
---

# Baseline

```text
R4b = 2781 B
```

# Experiment

After clearing `commit_magic` first, clear the 60-byte crash-record prefix
bytewise, then rewrite only nonzero header fields.

The implementation is defined C because an `unsigned char *` may inspect or
modify any object representation. Runtime cost was intentionally allowed to
increase.

# Verification

Generic fatal-path crash-record tests passed with the size-first clear enabled.

CI run 40659630435 passed all gates.

# Measured result

```text
R4b  2781 B
R4d  2781 B
-----------
delta    0 B
```

`arghtos_fatal` remained 180 B.

# Decision

Reject. The uglier/slower representation produces no linked size saving.
