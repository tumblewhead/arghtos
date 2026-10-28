---
type: ImplementationCheckpoint
title: "R4x - Selective inlining/outlining sweep"
description: "Rejected size experiments selectively forcing individual helpers out of the full-LTO graph."
tags: [implementation, footprint, compiler, inlining, rejected]
status: rejected
---

# Baseline

Accepted minimum-size baseline:

```text
R4y = 2294 B
```

R4y retains full LTO, `-Oz`, `-fomit-frame-pointer`,
`-fno-jump-tables`, the R4u task overlay, compact PendSV, compact SVC return,
and the minimum linker script.

# Goal

R4w showed that globally disabling inlining is disastrous, but some large
functions became individually smaller. R4x tested whether selectively forcing
specific helpers out-of-line could reduce total resident bytes while keeping
normal LTO everywhere else.

To preserve portable C11, the experiments use separate non-LTO helper
translation units rather than compiler-specific `noinline` source
attributes.

# Experiment A: fault attribution

Move only `task_fault_attributed()` out of the LTO graph.

Authoritative CI:

```text
40667150732  success
```

Measured:

```text
R4y                    2294 B
fault-helper profile   2314 B
delta                   +20 B
```

Symbol split:

```text
arghtos_fatal                 232 -> 228 B   -4 B
arghtos_task_fault_attributed   0 ->  24 B  +24 B
net                                          +20 B
```

Reject.

# Experiment B: time_due

Move only `time_due()` out of the LTO graph.

Same-revision CI:

```text
40667730031  success
```

Measured:

```text
R4y                 2294 B
R4x-time            2310 B
delta                +16 B
```

Relevant symbol changes:

```text
arghtos_time_due_external      0 ->   8 B
arghtos_kernel_service       228 -> 236 B
other major symbols          unchanged
```

The helper itself is tiny, but losing cross-callsite folding in the service
path costs more than the extracted code saves.

Reject.

# Experiment C: task_is_async

Move only `task_is_async()` out of the LTO graph.

Same-revision CI:

```text
40667730031  success
```

Measured:

```text
R4y                 2294 B
R4x-async           2326 B
delta                +32 B
```

Relevant symbol changes:

```text
arghtos_task_is_async_external   0 ->  20 B
arghtos_kernel_service         228 -> 220 B   -8 B
arghtos_kernel_dispatch        296 -> 316 B  +20 B
net total                                    +32 B
```

Reject.

# Conclusion

The selective-outlining sweep produced no win:

```text
candidate                  total      delta vs R4y
R4y baseline               2294 B      0 B
fault attribution          2314 B    +20 B
time_due                   2310 B    +16 B
task_is_async              2326 B    +32 B
```

For these candidates, Clang/LLVM `-Oz` plus full LTO already makes better
size decisions than manual out-of-line boundaries.

Do not repeat these helpers without a materially different ABI or algorithmic
hypothesis.

The result does not prove that all selective inlining choices are globally
optimal, but it removes the strongest candidates exposed by the global
R4w no-inline experiment.
