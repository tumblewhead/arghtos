---
type: ImplementationExperiment
title: "R6am - aligned first-dispatch MSP reclamation"
description: "Accepted Cortex-M23 startup/PendSV specialization proving an unpadded first exception frame and reclaiming a fixed 32 bytes."
tags: [implementation, footprint, context, msp, pendsv, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

The selected generated startup enters `arghtos_port_start()` across the
Cortex-M23 C ABI with an 8-byte-aligned MSP. If the selected startup routine
does not alter SP/MSP before the first pending scheduler exception becomes
eligible, the first hardware exception frame is exactly 32 bytes and xPSR[9]
cannot indicate an alignment pad.

R6am can then remove the first-dispatch xPSR load/test and reclaim exactly
32 bytes.

# Final evidence construction

An initial attempt tried to consume Clang's LTO `.su` file directly. Clang did
not emit that side file for the linked LTO object, so the experiment failed
closed and made no footprint claim.

The final R6am challenger strengthens the implementation instead:

* the selected C `arghtos_port_start()` definition is omitted;
* Cortex-M23 assembly supplies the selected start routine;
* that sequence pends PendSV, enables interrupts, executes ISB, and enters WFI;
* it contains no instruction that writes SP or MSP;
* an auxiliary same-source stack audit also reports
  `arghtos_port_start = 0 B static`.

The generic C startup path remains unchanged.

# Authority

* `architecture/contracts/C4-context-switch-port.md`
* `proof/P2-context-stack-integrity.md`

# Measurement

Authoritative physical-board CI:

```text
run:        40997923526
conclusion: success
```

Same-revision result:

```text
R6ak semantic control
  linked before MSP       1264 B
  ArghtOS-only MSP          64 B
  complete resident       1328 B
  arghtos_port_start        24 B
  PendSV_Handler            92 B

R6am
  linked before MSP       1256 B
  ArghtOS-only MSP          64 B
  complete resident       1320 B
  arghtos_port_start        18 B
  PendSV_Handler            84 B
```

```text
complete delta              -8 B
port_start symbol delta      -6 B
PendSV symbol delta          -8 B
```

The symbol deltas overlap with link/layout effects, so only the linked
complete-resident delta is authoritative.

# Decision

Accept R6am as the current semantic and physical R6 control at 1320 B.

The 64-byte ArghtOS-only MSP accounting is unchanged.
