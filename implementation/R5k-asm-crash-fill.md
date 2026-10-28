---
type: ImplementationCheckpoint
title: "R5k - Assembly Cortex-M23 crash-fill"
description: "Rejected near-miss moving Cortex-M23 crash payload extraction into a leaf Thumb assembly helper."
tags: [implementation, footprint, cortex-m23, crash, assembly, rejected]
status: rejected
---

# Baseline

```text
R5j = 1906 B
```

# Experiment

Replace only the Cortex-M23 implementation of
`arghtos_port_crash_fill()` with a leaf/no-stack Thumb helper using
`LDMIA/STMIA` transfers.

The portable C5 crash protocol remains unchanged:

* required 64-byte crash record;
* kernel clears generic payload before port fill;
* port format `0x4D23`;
* port word counts 2, 3, or 10;
* target state word ordering unchanged;
* generic release fence and commit marker remain kernel-owned.

# Verification

The first target build failed only because the assembly helper made the C
MSP/PSP reader helpers unused under `-Werror`. After gating those helpers out
for R5k, authoritative CI passed:

```text
40719949754  success
```

The assembly helper is leaf and allocates no MSP stack, so the accepted 72 B
ArghtOS-only MSP bound remains valid.

# Measured result

```text
R5j  1906 B
R5k  1918 B
-----------
delta  +12 B
```

The split is informative:

```text
arghtos_fatal             228 -> 160 B   -68 B
arghtos_port_crash_fill     0 ->  78 B   +78 B
other placement/alignment                 +2 B
```

# Decision

Reject R5k.

Moving the scalar copy out of the LTO fatal path exposes a much smaller
portable fatal core, but the standalone assembly helper costs more than the
inlined copy it replaces.

Do not repeat this exact helper split without a materially smaller target copy
routine or a broader crash-path contract change.
