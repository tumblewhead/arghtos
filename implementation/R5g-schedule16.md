---
type: ImplementationCheckpoint
title: "R5g - Fixed 16-bit static schedule codes"
description: "Rejected near-neutral experiment replacing 32-bit per-task schedules with O(1) 16-bit codes and optional exact-value overflow."
tags: [implementation, footprint, schedule, compression, rejected]
status: rejected
---

# Baseline

```text
R5e = 2202 B
```

# Experiment

Remove the 32-bit schedule word from each task slot and use a fixed-width
16-bit schedule table.

Encoding:

```text
0x0001..0x7EFF  direct periodic period
0x8001..0x80FE  direct async priority 1..254
0x7F00..0x7F17  overflow index 0..23
```

Overflow entries reference an optional exact 32-bit schedule table, preserving
the full legal schedule domain.

The footprint harness periods 1..24 use no overflow entries. Host verification
exercises direct periodic, direct async, and overflow forms.

# Verification

Authoritative CI:

```text
40673681141  success
```

# Measured result

```text
R5e  2202 B
R5g  2218 B
-----------
delta  +16 B
```

Configuration state behaves as intended:

```text
R5e task/config state  312 B
R5g task/config state  264 B
saved                   48 B
```

But linked code grows 64 B.

Notable changes:

```text
r5g_schedule_get          0 ->  36 B
task_is_async             0 ->  18 B
kernel_dispatch         272 -> 316 B
process_time_locked     252 -> 232 B
arghtos_init            164 -> 156 B
task_start_trampoline    72 ->  64 B
```

# Decision

Reject R5g as written.

The fixed-width representation is promising because the total miss is only
16 B while saving 48 B of resident state. The target build has zero overflow
entries, so the next experiment should specialize the zero-overflow case to
read period/priority directly from the 16-bit code without reconstructing an
exact 32-bit schedule through a helper.
