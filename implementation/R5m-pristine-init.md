---
type: ImplementationCheckpoint
title: "R5m - Pristine generated initialization"
description: "Accepted trusted generated-profile specialization eliminating redundant runtime task-state initialization."
tags: [implementation, footprint, static-config, init, accepted]
status: accepted
---

# Baseline

```text
R5l = 1822 B
```

# Experiment

R5l already supplies:

* valid prebuilt context tokens;
* zero initial next-time/age state;
* zero packed task status;
* zero/inactive unused generated slots;
* one-shot trusted initialization.

For R5m, `arghtos_init()` therefore performs only:

```text
port_init()
propagate failure
current_task = INVALID
flags = INITIALIZED
return OK
```

The final portable timer disarm is also removed in this profile because C4
already requires successful `arghtos_port_init()` to leave the scheduler
timer disarmed.

Generic/debug and non-R5m initialization remain unchanged.

# Verification

Authoritative CI:

```text
40722646982  success
```

All host, sanitizer, portability, and Cortex-M23 profile gates passed.

# Measured result

```text
R5l  1822 B
R5m  1782 B
-----------
saved   40 B
```

Main symbol change:

```text
arghtos_init  72 -> 32 B
```

Resident task/config state remains 240 B and the accepted ArghtOS-only MSP
bound remains 72 B.

# Decision

Accept R5m for the trusted generated minimum-size profile.

```text
linked before MSP  1710 B
MSP                  72 B
-------------------------
R5m total          1782 B
soft target        1536 B
remaining gap       246 B
```
