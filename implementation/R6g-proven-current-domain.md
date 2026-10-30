---
type: ImplementationExperiment
title: "R6g - proven fatal current-task domain"
description: "Neutral experiment replacing fatal-path range validation with the existing R5n INVALID-or-configured invariant."
tags: [implementation, footprint, cortex-m23, r6, rejected]
status: rejected-neutral
---

# Hypothesis

R5n proves that supported generated-profile execution keeps
`kernel.current_task` either INVALID or equal to a configured task ID.

R6g tested replacing the fatal path's repeated `id < task_count` check with
`id != ARGHTOS_TASK_ID_INVALID`, without enabling the previously rejected R5w
trusted-crash-port split.

# Measurement

The first CI attempt exposed only a build hygiene issue: the stronger profile
made `task_id_valid()` unused under `-Werror`. After constraining R6g to the
full trusted minimum profile and compiling that discharged helper out, the
authoritative comparison was:

```text
run: 40817867634
conclusion: success

R6f control
  linked before MSP       1414 B
  ArghtOS-only MSP          64 B
  complete resident       1478 B
  arghtos_fatal            156 B

R6g challenger
  linked before MSP       1414 B
  ArghtOS-only MSP          64 B
  complete resident       1478 B
  arghtos_fatal            156 B

delta                        0 B
```

# Decision

Reject R6g as neutral.

The R5n domain refinement is valid, but the Cortex-M23 optimizer already emits
an equivalent-size check in the accepted control. Do not keep R6g as a live
footprint challenger.
