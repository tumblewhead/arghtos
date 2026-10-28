---
type: ImplementationCheckpoint
title: "R4i - Service switch versus if-chain"
description: "Rejected neutral experiment expressing the five service cases as an if/else chain."
tags: [implementation, footprint, service, codegen, rejected]
status: rejected
---

# Baseline

```text
R4h = 2770 B
```

# Experiment

Replace the five-way `arghtos_kernel_service()` switch with an explicit
if/else chain while preserving all service semantics.

Generic host tests ran the if-chain directly.

# Measured result

CI run 40661034656 passed.

```text
R4h  2770 B
R4i  2770 B
-----------
delta    0 B
```

`arghtos_kernel_service` remained 256 B.

# Decision

Reject. Clang canonicalizes both forms to the same linked footprint, so retain
the clearer switch source.
