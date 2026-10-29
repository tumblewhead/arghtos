---
type: ImplementationCheckpoint
title: "R5z - Raw class-partitioned schedule bytes"
description: "Accepted generated-order specialization removing schedule class tags from the zero-overflow schedule8 path."
tags: [implementation, footprint, generated-config, schedule, accepted]
status: accepted
---

# Baseline

```text
R5y = 1646 B
```

# Premise

R5p already proves that task IDs are partitioned by class:

```text
0 .. P-1      periodic, RMS ordered
P .. N-1      async
```

R5i zero-overflow schedule8 already guarantees every target schedule value fits
directly in one byte.

Therefore the generated profile does not need a class tag inside each schedule
byte. R5z stores:

* raw period bytes in the periodic prefix;
* raw priority bytes in the async suffix.

All call sites that read a period or priority already operate within the
corresponding generated ID partition.

Generic R5i encoding remains unchanged.

# Verification

Authoritative CI:

```text
40806548358  success
```

Host tests, sanitizers, portability, and all Cortex-M23 comparison profiles
passed.

# Measured result

```text
R5y  1646 B
R5z  1618 B
-----------
saved   28 B
```

Notable symbol changes:

```text
process_time_locked          180 -> 176 B
arghtos_start                 80 ->  76 B
arghtos_task_start_trampoline 40 ->  18 B
```

Task/config state remains 240 B. The static ArghtOS-only MSP bound remains
64 B because the time-engine frame is still 36 B.

# Decision

Accept R5z.

```text
linked before MSP  1554 B
MSP                  64 B
-------------------------
R5z total          1618 B
soft target        1536 B
remaining gap        82 B
```
