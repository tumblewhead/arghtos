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

## Measurement classes

Historical footprint evidence through R6q is reproduced with the original
constant-foldable synthetic board harness. Those values remain canonical for
experiment provenance, including R5ag = 1534 B and legacy R6q = 1356 B.

New absolute footprint claims use the physical-board harness, where synthetic
time/timer state is volatile and placed in application-owned `.app_data`.
This prevents LTO from proving time constant or timer writes unobservable.

See
[implementation/FOOTPRINT-HARNESS-METHODOLOGY.md](implementation/FOOTPRINT-HARNESS-METHODOLOGY.md).

## Active post-R5 research

```text
branch: implementation/r6-size-research-001
PR:     #5 (draft)

historical legacy harness
  R5ag frozen merged reference = 1534 B
  R6q latest legacy checkpoint = 1356 B

physical-board harness
  R6q physical control         = 1504 B
  R6r accepted                 = 1492 B
  R6s accepted                 = 1480 B
  R6t accepted                 = 1424 B
  R6u accepted control         = 1420 B
  saved from physical R6q      =   84 B
  margin below 1536 target     =  116 B
  phase: R6v unconditional unit-tick dispatch experiment
```

R6u is preserved in
[implementation/R6u-single-async-unit-timer.md](implementation/R6u-single-async-unit-timer.md).

Authoritative R6u CI:

```text
run:        40923612503
conclusion: success

physical R6t           1424 B
physical R6u           1420 B
delta                     -4 B

process_time_locked:
  R6t                   184 B
  R6u                   180 B
```

R6v trades bounded runtime cycles for bytes. Under the unit-period profile,
timer entry requests PendSV every tick, even when the time engine creates no
new READY work. Because profiling is off and async age-reset side effects are
elided, a redundant dispatch over an unchanged eligibility set preempts and
reselects the same current task. Any already-pending scheduling indication
already required PendSV.

This allows the time engine to drop its `changed` result and all persistent
change bookkeeping. P6 explicitly charges the possible extra PendSV/dispatcher
pass to kernel overhead.

Owning refinements:

* [architecture/contracts/C3-time-timer.md](architecture/contracts/C3-time-timer.md)
* [proof/P1-task-state-invariants.md](proof/P1-task-state-invariants.md)
* [proof/P6-rms-schedulability.md](proof/P6-rms-schedulability.md)

## Exact next action

1. Let PR #5 CI build physical R6u and R6v in the same revision.
2. Require the new redundant-dispatch host regression, all semantic gates,
   historical reproduction, and physical controls green.
3. Compare complete resident bytes, `process_time_locked`, and
   `arghtos_timer_interrupt`.
4. Accept R6v only for a strict physical reduction.
5. If accepted, preserve the bounded runtime-cost caveat in the implementation
   checkpoint and P6.
6. Do not merge PR #5 without explicit user permission.
