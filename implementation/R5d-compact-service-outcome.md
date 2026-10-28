---
type: ImplementationCheckpoint
title: "R5d - Compact service outcome"
description: "Accepted minimum-profile service dispatcher using one encoded byte for result plus dispatch intent."
tags: [implementation, footprint, service, wake, accepted]
status: accepted
---

# Baseline

```text
R5b = 2286 B
```

# Experiment

Use one internal service outcome byte:

```text
bits 0..2  public result code
bit 3      request dispatch
```

The minimum specialized service dispatcher carries this encoded value through
all service cases instead of maintaining separate `result` and `dispatch`
locals.

The wake case reuses the compact R5c wake helper and does not decode it inside
the switch.

# Verification

Authoritative CI:

```text
40671500777  success
```

All dedicated host, sanitizer, portability, and Cortex-M23 gates passed.

# Measured result

```text
R5b  2286 B
R5d  2270 B
-----------
saved   16 B
```

Relevant symbols:

```text
wake_locked             92 B
wake_locked_compact     80 B   -12 B

R5b kernel_service     228 B
R5d kernel_service     224 B    -4 B
```

The interrupt wake wrapper remains 40 B.

# MSP audit

The service control flow changes representation only. It does not add call
depth or automatic arrays. The accepted ArghtOS-only MSP bound remains 80 B.

# Decision

Accept R5d.

```text
linked before MSP  2190 B
MSP                  80 B
-------------------------
R5d total          2270 B
remaining gap       734 B
```
