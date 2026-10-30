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

# Erratum discovered during R6ai

R6ai found one R5z call site that did **not** satisfy the partition premise
stated above.

The task-start trampoline still classified its current task with:

~~~c
ARGHTOS_TASK_PERIOD(id) != 0u
~~~

Under R5z raw schedule encoding, an async suffix entry stores its raw async
priority rather than zero. For the canonical 23-periodic + 1-async fixture,
task 23 stores priority \`3\`. Therefore the trampoline read \`3\` as a
nonzero period and entered the periodic TASK_RETURN loop.

The historical R5z measurement remains valid as footprint evidence, but the
original semantic acceptance was incomplete: the 18-byte trampoline was partly
obtained because LTO proved every raw schedule byte nonzero and removed the
async-return-fatal path.

Consequences:

* R5z and descendants using that trampoline cannot claim the preserved
  "async return is fatal" invariant without an additional repair;
* the R5z statement that every period/priority read occurred within a
  class-partitioned call site is false for this trampoline read;
* R6ai's explicit generated-ID class test is the first measured direct repair,
  but costs 20 B on the physical profile;
* subsequent work should prefer generated initial-context specialization so the
  async class decision is discharged before runtime.

Do not reinterpret the historical 1618 B measurement. Treat it as size
provenance with this semantic erratum attached.

# Erratum resolution

R6aj resolves the async-trampoline defect without resident-byte cost by moving
task-class selection into the generated prebuilt Cortex-M23 initial frame.

The sole async task now starts directly at its application entry with the
return guard as LR. Periodic tasks retain the shared periodic trampoline.

Authoritative repair:

```text
R6ai runtime correctness control  1360 B
R6aj generated-context repair     1340 B
R6aj vs historical R6ag              0 B
```

See [R6aj direct async prebuilt entry](R6aj-direct-async-prebuilt-entry.md).

The historical R5z artifact remains tagged with this erratum because its own
binary predates the repair.
