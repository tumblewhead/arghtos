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
  R6u accepted                 = 1420 B
  R6v accepted                 = 1396 B
  R6aa accepted                = 1384 B
  R6ab accepted                = 1380 B
  R6ac accepted                = 1372 B
  R6ad accepted                = 1360 B
  R6ae accepted                = 1348 B
  R6af rejected neutral        = 1348 B
  R6ag historical size-only   = 1340 B
  R6ah rejected neutral        = 1340 B
  R6ai runtime correctness ctl = 1360 B
  R6aj accepted semantic ctl   = 1340 B
  R6ak accepted                = 1328 B
  R6al rejected neutral        = 1328 B
  R6am accepted                = 1320 B
  R6an rejected neutral        = 1320 B
  R6ao accepted                = 1319 B
  R6ap accepted current ctl    = 1318 B
  phase: R6aq timer-entry task0-due specialization
```

Correctness repair chain:

* [implementation/R6ai-generated-trampoline-class.md](implementation/R6ai-generated-trampoline-class.md)
* [implementation/R6aj-direct-async-prebuilt-entry.md](implementation/R6aj-direct-async-prebuilt-entry.md)
* [implementation/R6ak-r4-asm-periodic-trampoline.md](implementation/R6ak-r4-asm-periodic-trampoline.md)

R6ak authoritative CI:

```text
run:        40996040636
conclusion: success

R6aj semantic control       1340 B
R6ak                        1328 B
delta                        -12 B

task-start trampoline:
  R6aj                        18 B
  R6ak                         8 B

frame relocation audit:
  23 periodic frames via R4
   1 direct async frame
  PASS
```

R6ak preserves the R6aj async-return-fatal repair and is now the semantic and
physical R6 control.

R6al is preserved in
[implementation/R6al-direct-unit-timer-arm.md](implementation/R6al-direct-unit-timer-arm.md).

R6al authoritative CI:

```text
run:        40996547586
conclusion: success
R6ak:      1328 B
R6al:      1328 B
delta:        0 B
arghtos_timer_interrupt:
  both       184 B
```

R6ak remains the current semantic and physical control at 1328 B.

R6am is preserved in
[implementation/R6am-aligned-first-dispatch.md](implementation/R6am-aligned-first-dispatch.md).

R6am authoritative CI:

```text
run:        40998129667
conclusion: success

R6ak control              1328 B
R6am                      1320 B
delta                       -8 B

arghtos_port_start           18 B
PendSV_Handler:
  R6ak                       92 B
  R6am                       84 B

generated-frame audit: PASS
selected startup: assembly, no SP/MSP write
```

R6am is now the accepted semantic and physical control.

R6an is preserved in
[implementation/R6an-reason-recursion-guard.md](implementation/R6an-reason-recursion-guard.md).

R6an authoritative CI:

```text
run:        40998859284
conclusion: success
R6am:      1320 B
R6an:      1320 B
delta:        0 B
arghtos_fatal:
  both       136 B
```

R6ao is preserved in
[implementation/R6ao-periodic-only-schedule8.md](implementation/R6ao-periodic-only-schedule8.md).

R6ao authoritative CI:

```text
run:        40999743039
conclusion: success
R6am:      1320 B
R6ao:      1319 B
delta:       -1 B

arghtos_static_schedule8:
  R6am        24 B
  R6ao        23 B
```

R6ap is preserved in
[implementation/R6ap-omit-unit-period-byte.md](implementation/R6ap-omit-unit-period-byte.md).

R6ap authoritative CI:

```text
run:        41014470378
conclusion: success
R6ao:      1319 B
R6ap:      1318 B
delta:       -1 B

arghtos_static_schedule8:
  R6ao        23 B
  R6ap        22 B

arghtos_timer_interrupt:
  both       184 B
```

R6aq targets task 0 inside the timer-owned time engine. The accepted contracts
already establish:

* the generated task 0 period is exactly one tick;
* task 0 is the timer-dominant deadline source;
* `process_time_locked()` is called only from the scheduler timer entry;
* an armed timer is serviced at or after its absolute deadline.

Therefore task 0 is due on every admitted time-engine entry. R6aq will process
task 0 as an unconditional due release, then scan periodic IDs 1..P-1 with the
ordinary due predicate.

## Exact next action

1. Add the R6aq timer-entry/task0-due contract and P4 proof refinement.
2. Add a profile gate requiring R6ag timer ownership, R6t unit-period dominance,
   R6ap schedule reconstruction, and trusted timer-entry semantics.
3. Specialize task 0 catch-up before the remaining periodic scan.
4. Add host regression with deliberately late timer service to verify task0
   catch-up, missed-release count, and next_time = now+1.
5. Build physical R6ap and R6aq side by side.
6. Accept R6aq only for a strict physical reduction below 1318 B.
7. Do not merge PR #5 without explicit user permission.
