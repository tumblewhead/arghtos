---
type: ImplementationExperiment
title: "R6am - aligned first-dispatch MSP reclaim"
description: "Accepted Cortex-M23 specialization using fixed 32-byte first-dispatch MSP reclaim under a proved aligned startup path."
tags: [implementation, footprint, cortex-m23, pendsv, r6, accepted]
status: accepted
---

# Hypothesis

For the trusted selected Cortex-M23 startup path, the first Thread-mode
exception frame is known to start from an 8-byte-aligned MSP with no hardware
alignment padding.

R6am therefore replaces first-dispatch xPSR[9] decoding and 32/36-byte reclaim
selection with a fixed 32-byte MSP reclaim.

# Authority

* `architecture/contracts/C4-context-switch-port.md`
* `proof/P2-context-stack-integrity.md`

# Evidence

The final selected startup implementation is the explicit Cortex-M23 assembly
`arghtos_port_start` used only in R6am. It pends PendSV, enables interrupts,
executes ISB, then waits in a WFI loop without any SP/MSP write.

CI also builds a dedicated non-LTO audit object from the corresponding C startup
source and confirms the equivalent C path has zero static stack usage. This is
supplemental evidence; the linked R6am image selects the assembly startup.

Generated-frame audit remains green:

```text
23 periodic frames via R4
 1 direct async frame
PASS
```

Authoritative CI:

```text
run:        40998129667
conclusion: success
```

# Physical measurement

```text
R6ak control
  linked before MSP       1264 B
  ArghtOS-only MSP          64 B
  complete resident       1328 B
  PendSV_Handler            92 B

R6am
  linked before MSP       1256 B
  ArghtOS-only MSP          64 B
  complete resident       1320 B
  arghtos_port_start        18 B
  PendSV_Handler            84 B

delta                        -8 B
```

# Decision

Accept R6am as the current semantic and physical Cortex-M23 control at 1320 B.

Generic/debug profiles retain xPSR alignment-padding decoding.
