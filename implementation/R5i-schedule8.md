---
type: ImplementationCheckpoint
title: "R5i - Fixed 8-bit schedule codes"
description: "Accepted O(1) schedule8 representation with direct zero-overflow target access and exact overflow fallback."
tags: [implementation, footprint, schedule, static-config, accepted]
status: accepted
---

# Baseline

```text
R5h = 1970 B
```

# Representation

One byte per task:

```text
0x01..0x5F  direct periodic period 1..95
0x60..0x77  overflow index 0..23
0x81..0xFF  direct async priority 1..127
```

Longer periodic periods and async priorities 128..254 remain exactly
representable through an optional 32-bit overflow table.

The target footprint fixture uses periods 1..24 and therefore has zero
overflow entries.

# Verification

Authoritative CI:

```text
40718436840  success
```

The host R5i profile uses three tasks and one overflow entry to exercise:

* exact overflow periodic decoding;
* direct async decoding;
* direct periodic decoding.

The Cortex-M23 footprint uses the direct zero-overflow byte path.

# Measured result

```text
R5h  1970 B
R5i  1914 B
-----------
saved   56 B
```

Resident task/config state:

```text
R5h .arghtos_cfg  264 B
R5i .arghtos_cfg  240 B
saved              24 B
```

Code also shrinks by 32 B:

```text
process_time_locked       208 -> 188 B
arghtos_kernel_dispatch   188 -> 180 B
arghtos_start             100 ->  96 B
```

Other major symbols are unchanged.

# Decision

Accept R5i.

Using the previously accepted 80 B static MSP accounting:

```text
linked before MSP  1834 B
MSP                  80 B
-------------------------
R5i total          1914 B
soft target        1536 B
remaining gap       378 B
```

MSP is re-audited separately in R5j rather than retroactively changing this
measurement.
