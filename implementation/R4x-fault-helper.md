---
type: ImplementationCheckpoint
title: "R4x - Selective out-of-line fault attribution"
description: "Rejected experiment forcing only the task-fault attribution predicate out of LTO."
tags: [implementation, footprint, fault, compiler, rejected]
status: rejected
---

# Baseline

```text
R4y = 2294 B
```

# Experiment

The global R4w no-inline build suggested that separating
`task_fault_attributed()` might shrink the combined fatal path.

To preserve portable C11, R4x did not use a compiler-specific source attribute.
Instead it:

* moved the predicate into a separate portable C translation unit;
* compiled that helper without LTO;
* left the rest of the accepted R4y image under full LTO.

# Verification

CI run 40667150732 passed all host, sanitizer, portability, and Cortex-M23
gates.

# Measured result

```text
R4y  2294 B
R4x  2314 B
-----------
delta  +20 B
```

The external predicate is 24 B. `arghtos_fatal` shrinks only 4 B:

```text
fatal              232 -> 228 B
external predicate   0 ->  24 B
net                       +20 B
```

# Decision

Reject. The apparent ~44 B opportunity observed in the global R4w split depended
on broader no-inline codegen changes and does not survive selective isolation.
