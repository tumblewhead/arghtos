---
type: ImplementationExperiment
title: "R6an - reason-owned recursive-fatal guard"
description: "Neutral crash-recursion representation experiment using the nonzero fault reason as the in-progress guard."
tags: [implementation, footprint, crash, cortex-m23, r6, rejected]
status: rejected-neutral
---

# Hypothesis

All admitted selected-profile fatal callers use nonzero fault reasons, while the
generated crash image begins with reason NONE = 0.

R6an therefore uses the reason field as the recursive-fatal guard and leaves
commit_magic at zero until the final commit-last write.

# Regression

The recursive-fatal host test verifies:

* exactly one crash-fill call occurs;
* the first nonzero reason remains visible in the invalid in-progress record;
* commit_magic remains zero while capture is interrupted.

# Measurement

Authoritative physical-board CI:

```text
run:        40998859284
conclusion: success
```

Same-revision result:

```text
R6am control
  linked before MSP       1256 B
  ArghtOS-only MSP          64 B
  complete resident       1320 B
  arghtos_fatal            136 B

R6an
  linked before MSP       1256 B
  ArghtOS-only MSP          64 B
  complete resident       1320 B
  arghtos_fatal            136 B

delta                        0 B
```

# Decision

Reject R6an as neutral. Keep R6am = 1320 B as the accepted semantic and
physical control.
