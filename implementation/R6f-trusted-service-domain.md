---
type: ImplementationExperiment
title: "R6f - trusted private service-ID domain"
description: "Accepted generated-profile specialization removing the unreachable BAD_SERVICE default branch."
tags: [implementation, footprint, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

The trusted generated Cortex-M23 profile reaches
`arghtos_kernel_service()` only through fixed public-service veneers and the
generated task-return trampoline.

Under that closed service domain, only YIELD, WAIT, SLEEP_UNTIL, WAKE, and
TASK_RETURN are reachable. The runtime BAD_SERVICE default branch is therefore
redundant in the generated minimum profile.

The generic/debug profile retains the fatal invalid-service path. R6f uses no
compiler unreachable intrinsic and introduces no undefined C behavior.

# Authority

The specialization is constrained by:

* `architecture/contracts/C4-context-switch-port.md`
* `architecture/contracts/C1-task-api-lifecycle.md`
* `proof/P1-task-state-invariants.md`

# Measurement

PR #5 CI run:

```text
run:        40817586249
conclusion: success
```

Same-revision physical result:

```text
R6e control
  linked before MSP          1418 B
  ArghtOS-only MSP             64 B
  complete resident          1482 B
  arghtos_kernel_service      160 B

R6f
  linked before MSP          1414 B
  ArghtOS-only MSP             64 B
  complete resident          1478 B
  arghtos_kernel_service      156 B

linked delta                   -4 B
service symbol delta           -4 B
```

All retained host, sanitizer, portability, generic Cortex-M23, frozen R5ag,
R6b-R6e, and R6f gates passed.

# Decision

Accept R6f as the current R6 minimum-size control at 1478 B.

R5ag remains the immutable merged R5 reference at 1534 B.
