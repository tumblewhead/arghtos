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
control: R5ag = 1534 B
R6a:    rejected neutral, 1534 B, 0 B delta
phase:  R6b trusted timer-entry experiment
```

R6a is preserved in
[implementation/R6a-compact-wake-tree.md](implementation/R6a-compact-wake-tree.md).

R6b tests a proof-backed generated-profile specialization: elide the defensive
pre-start guard in `arghtos_timer_interrupt()` only when the selected
generated port/profile guarantees that the scheduler timer is disarmed before
start, STARTED is committed before first arm, and a disarmed timer cannot call
the timer entry.

The generic profile retains the guard. R5ag remains untouched.

Owning refinements:

* [architecture/contracts/C3-time-timer.md](architecture/contracts/C3-time-timer.md)
* [proof/P4-time-release-correctness.md](proof/P4-time-release-correctness.md)

## Exact next action

1. Let PR #5 CI build frozen R5ag and R6b in the same revision.
2. Compare complete resident bytes and `arghtos_timer_interrupt` symbol size.
3. Accept R6b only for a strict linked physical reduction with all gates green.
4. If accepted, record the exact generated-profile preconditions and freeze the
   new control beside R5ag evidence.
5. If neutral/regressive, reject it and preserve the result before choosing the
   next challenger.
6. Do not merge PR #5 without explicit user permission.
