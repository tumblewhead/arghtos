---
type: ImplementationCheckpoint
title: "R5o - Generated minimum-period startup timer"
description: "Accepted trusted generated-profile specialization replacing runtime startup timer minimum search with a build-time minimum period."
tags: [implementation, footprint, generated-config, timer, accepted]
status: accepted
---

# Baseline

```text
R5n = 1722 B
```

# Experiment

The trusted generated configuration already knows the smallest periodic period.
R5o emits that value as `ARGHTOS_R5O_MIN_PERIOD`.

At scheduler start, periodic tasks still receive:

```text
state = READY
next_time = start + period
```

but the kernel no longer runs `timer_consider()` to rediscover the minimum.
It arms:

```text
start + generated_min_period
```

directly.

The host profile uses minimum period 10 and retains the existing exact
deadline regression. The 24-task footprint fixture uses minimum period 1.

# Verification

Authoritative CI:

```text
40725383649  success
```

All host, sanitizer, portability, and Cortex-M23 gates passed.

# Measured result

```text
R5n  1722 B
R5o  1702 B
-----------
saved   20 B
```

Main symbol change:

```text
arghtos_start  96 -> 76 B
```

Other major symbols are unchanged. Resident task/config state remains 240 B
and the 72 B ArghtOS-only MSP bound remains valid.

# Decision

Accept R5o.

```text
linked before MSP  1630 B
MSP                  72 B
-------------------------
R5o total          1702 B
soft target        1536 B
remaining gap       166 B
```
