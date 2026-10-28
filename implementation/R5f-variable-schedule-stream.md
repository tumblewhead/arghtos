---
type: ImplementationCheckpoint
title: "R5f - Variable-length compressed static schedule"
description: "Rejected experiment replacing per-task 32-bit schedules with a compact byte stream decoded by bounded scan."
tags: [implementation, footprint, schedule, compression, rejected]
status: rejected
---

# Baseline

```text
R5e = 2202 B
```

# Experiment

Remove the 32-bit schedule word from each unified task slot and store immutable
schedules in a variable-length byte stream.

Encoding:

```text
0x01..0x7E  direct periodic period
0x81..0xFF  direct async priority
0x7F        escape followed by exact 32-bit schedule value
```

The decoder scans from the beginning of the stream to the requested task ID.
Work remains bounded by the 24-task maximum and full schedule semantics remain
representable through the escape form.

The decoder was deliberately compiled without LTO to avoid cloning the scan
into every schedule access.

# Verification

Final authoritative CI:

```text
40672791022  success
```

The host profile exercised both a direct async code and an escaped exact
32-bit periodic value.

# Measured result

```text
R5e  2202 B
R5f  2306 B
-----------
delta +104 B
```

Resident task/config state:

```text
R5e .arghtos_cfg  312 B
R5f .arghtos_cfg  240 B
raw data saving    72 B
```

But text grew:

```text
R5e .text  1910 B
R5f .text  2086 B
delta      +176 B
```

Notable R5f symbols:

```text
arghtos_r5f_schedule_get   76 B
task_is_async              30 B
arghtos_start             140 B
arghtos_kernel_dispatch   328 B
process_time_locked       232 B
```

The variable-length decoder and altered optimizer boundaries overwhelm the
configuration-state saving.

# Decision

Reject R5f.

The underlying result is still useful: immutable schedule data is compressible,
but schedule access must remain O(1) or nearly so. A fixed-width compact code
with an optional exact-value overflow table is the next useful design.
