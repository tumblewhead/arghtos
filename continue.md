# ArghtOS live continuation checkpoint

Last refreshed: 2026-10-29

Read [agents.md](agents.md) first. Repository state is authoritative.

## Canonical repository state

```text
repo:      https://github.com/tumblewhead/arghtos
base:      main
R5 merge:  afc38c559835dc26573634bf56e1293f75389162
PR #4:     merged
phase:     R5 frozen; post-R5 research may begin
```

PR #4 merged the R5 size-specialization work into `main`.

## Frozen R5 control

The accepted Cortex-M23 minimum-size generated profile is R5ag:

```text
task capacity          24
measured fixture       23 periodic + 1 async
linked before MSP      1470 B
ArghtOS-only MSP         64 B
complete resident      1534 B
soft target            1536 B
margin                    2 B
R4y control            2294 B
saved from R4y          760 B
```

R5ag is now immutable control evidence. Any further size experiment must be a
new challenger beside R5ag rather than modifying the frozen profile in place.

Authoritative validation before merge:

```text
target-crossing CI     40812507607  success
cleaned CI             40814851219  success
ready-head CI          40814905429  success
final handoff CI       40814968750  success
```

See [implementation/R5-target-crossing-1534.md](implementation/R5-target-crossing-1534.md).

## Preserved semantics

The frozen result retains:

* synchronous periodic RMS scheduling;
* lower task ID wins equal-period periodic ties;
* one active periodic instance;
* arithmetic delayed-release catch-up;
* async residual-capacity scheduling with required age credit;
* wake/wait pending-wake race closure;
* async return as fatal;
* required 64-byte crash record with commit marker last;
* recursive fatal protection;
* no heap and bounded scheduler work;
* portable target-independent C11 scheduler core;
* Cortex-M23 task context ABI and PSP/MSP ownership;
* first-dispatch MSP reclamation and xPSR alignment handling;
* PRIMASK serialization and race-free WFI idle.

Generated-profile assumptions are part of the proof boundary and must not be
generalized to arbitrary runtime configurations.

## Frozen build/CI surface

The post-R5 cleanup intentionally retains:

* portability check;
* generic host kernel tests;
* accepted size-first selector host regression;
* R3c profiling-enabled reference;
* R4u host minimum-source reference;
* R5ag host semantic profile;
* ASan/UBSan;
* generic Cortex-M23 compile;
* R3c footprint reference;
* frozen R4y footprint;
* frozen R5ag footprint;
* explicit CI assertion that R5ag resident footprint is <=1536 B.

Historical accepted/rejected experiment documents remain under
`implementation/` as evidence.

## Footprint guardrail

The margin is only 2 B. Never claim R5ag still meets the target after a
compiler, linker, source-order, linker-script, generated-fixture, or profile
change without re-linking the physical Cortex-M23 image and reading the
footprint report.

## Active post-R5 research

```text
branch: implementation/r6-size-research-001
PR:     #5 (draft)
R5ag:   frozen merged reference = 1534 B
R6a:    rejected neutral         = 1534 B
R6b:    accepted                  = 1522 B
R6c:    accepted                  = 1514 B
R6d:    accepted                  = 1494 B
R6e:    accepted                  = 1482 B
R6f:    accepted                  = 1478 B
R6g:    rejected neutral          = 1478 B
R6h:    rejected neutral          = 1478 B
R6i:    accepted                  = 1470 B
R6j:    accepted                  = 1450 B
R6k:    accepted control          = 1434 B
phase:  R6l pre-masked scheduler-start experiment
```

R6k is preserved in
[implementation/R6k-preseeded-periodic-ready.md](implementation/R6k-preseeded-periodic-ready.md).

R6l moves ownership of the scheduler-start critical section to the trusted
generated integration. On Cortex-M23, the generated caller enters
`arghtos_start()` in Thread/MSP with PRIMASK=1, the scheduler timer disarmed,
and no active dispatch. `arghtos_start()` remains masked through deadline
materialization, STARTED commit, and timer arm; `arghtos_port_start()` then
takes ownership and restores normal interrupt eligibility.

Generic profiles retain the ordinary critical enter/exit pair.

Owning refinements:

* [architecture/contracts/C4-context-switch-port.md](architecture/contracts/C4-context-switch-port.md)
* [architecture/contracts/C0-static-task-configuration.md](architecture/contracts/C0-static-task-configuration.md)
* [proof/P2-context-stack-integrity.md](proof/P2-context-stack-integrity.md)

## Exact next action

1. Let PR #5 CI build R6k and R6l in the same revision.
2. Compare complete resident bytes and `arghtos_start` symbol size.
3. Accept R6l only for a strict linked physical reduction with the pre-masked
   host-start regression and all retained gates green.
4. If accepted, preserve the exact masked-entry/start ownership preconditions in
   an R6l implementation checkpoint.
5. If neutral/regressive, reject and preserve the result before selecting R6m.
6. Do not merge PR #5 without explicit user permission.
