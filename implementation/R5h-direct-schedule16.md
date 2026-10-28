---
type: ImplementationCheckpoint
title: "R5h - Direct zero-overflow schedule16"
description: "Accepted fixed 16-bit static schedule representation specialized to direct halfword access when generated configuration needs no overflow entries."
tags: [implementation, footprint, schedule, compression, accepted]
status: accepted
---

# Baseline

```text
R5e = 2202 B
```

R5g proved the 16-bit representation but used a general schedule reconstruction
helper and measured 2218 B.

# R5h specialization

When generated configuration has zero overflow schedules, the exact task class,
period, and async priority are read directly from the 16-bit code:

```text
0x0001..0x7EFF  direct periodic period
0x8001..0x80FE  direct async priority
```

The overflow-capable R5g representation remains defined for configurations
that need longer periods.

No 32-bit schedule reconstruction helper is present in the zero-overflow
target build.

# Verification

Authoritative CI:

```text
40674018494  success
```

Host tests separately cover the overflow-capable R5g representation and the
zero-overflow R5h specialization.

# Measured result

```text
R5e  2202 B
R5h  1970 B
-----------
saved  232 B
```

Physical sections:

```text
                 R5e      R5h
.text           1910 B   1726 B   -184 B
.arghtos_cfg     312 B    264 B    -48 B
```

Major symbol changes:

```text
kernel_dispatch        272 -> 188 B
process_time_locked    252 -> 208 B
kernel_service         208 -> 184 B
task_start_trampoline   72 ->  48 B
arghtos_init           164 -> 156 B
```

The task/config state is:

```text
core slots       8 * 24 = 192 B
status meta      1 * 24 =  24 B
schedule16       2 * 24 =  48 B
-------------------------------
total                     264 B
```

# MSP

No new call depth or automatic storage is introduced in the accepted
zero-overflow path. The ArghtOS-only MSP bound remains 80 B.

# Decision

Accept R5h.

```text
linked before MSP  1890 B
MSP                  80 B
-------------------------
R5h total          1970 B
remaining gap       434 B
```
