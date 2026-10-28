---
type: ImplementationCheckpoint
title: "R4r - Forced LLVM machine outlining"
description: "Rejected neutral compiler experiment forcing machine outlining on the accepted R4q minimum-size profile."
tags: [implementation, footprint, compiler, outliner, rejected]
status: rejected
---

# Baseline

```text
R4q = 2410 B
```

# Experiment

Add:

```text
-mllvm -enable-machine-outliner=always
```

to all C compilation and the LTO link on top of accepted R4q.

# Measured result

CI run 40664239986 passed.

```text
R4q  2410 B
R4r  2410 B
-----------
delta    0 B
```

The symbol-size reports are identical.

# Decision

Reject. Keep the simpler accepted R4q compiler command.

The 80 B static MSP bound is unchanged because no outlined helper changed the
linked code shape.
