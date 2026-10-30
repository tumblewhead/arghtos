---
type: ImplementationExperiment
title: "R6r - flagless minimum kernel state"
description: "Accepted trusted-profile specialization removing the minimum C9 flags byte after all flag roles are discharged."
tags: [implementation, footprint, kernel-state, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

In the fully specialized trusted generated profile, every C9 flag role has
already been discharged:

* R6p materializes successful initialization by construction;
* minimal-validation production rules remove runtime INITIALIZED/STARTED misuse
  checks;
* R5n proves dispatch cannot enter before start;
* R6b proves timer entry cannot occur before start;
* R6m moves recursive-fatal state into the crash commit word.

The remaining STARTED write is therefore observationally dead. The minimum
kernel state can contain only `current_task`.

The 32-bit minimum C9 representation shrinks from 4 bytes to 1 byte.

# Authority

* `architecture/contracts/C9-kernel-global-state.md`
* `proof/P0-static-configuration-validity.md`
* `proof/P1-task-state-invariants.md`

# Measurement-harness correction

During R6r work, the Cortex-M23 board stub was corrected so time/timer state is
externally mutable to LTO. That exposed an old measurement-harness weakness:
historical absolute sizes had benefited from constant folding through the
synthetic board boundary.

The harness is now split explicitly. See:

`implementation/FOOTPRINT-HARNESS-METHODOLOGY.md`

R6r acceptance is based on the corrected physical-board lane, not comparison
against a legacy-harness R6q number.

# Physical measurement

Authoritative CI:

```text
run:        40922006295
conclusion: success
```

Same-revision physical-board comparison:

```text
R6q physical control
  linked before MSP       1440 B
  ArghtOS-only MSP          64 B
  complete resident       1504 B
  arghtos_start              52 B

R6r
  linked before MSP       1428 B
  ArghtOS-only MSP          64 B
  complete resident       1492 B
  arghtos_start              40 B

linked delta               -12 B
start symbol delta          -12 B
```

The source representation also has:

```text
sizeof(arghtos_kernel_state_t):
  R6q minimum profile   4 B
  R6r flagless profile  1 B
```

Layout/link effects mean the resident delta is expressed primarily as a
12-byte code reduction in the final image rather than a simple three-byte
resident-data subtraction.

# Historical reproduction

The same CI run reproduces the frozen legacy-harness chain exactly, including:

```text
R5ag = 1534 B
R6q  = 1356 B
```

Those values are retained as historical experiment evidence, not as the new
absolute physical-board baseline.

# Decision

Accept R6r as the current physical-board R6 control at 1492 B.

This is 44 B below the original 1536 B target under the stronger physical-board
measurement model.
