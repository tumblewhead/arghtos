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
  R6ak accepted current ctl    = 1328 B
  R6al rejected neutral        = 1328 B
  phase: R6am fixed aligned first-dispatch MSP reclaim (wired)
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

R6am targets only first-dispatch PendSV MSP reclamation.

The current selected path reads stacked xPSR[9] and conditionally reclaims
either 32 or 36 bytes. R6am will use a fixed 32-byte reclaim only if CI proves:

1. the call from portable C into the selected `arghtos_port_start()` obeys
   the Cortex-M23 AAPCS 8-byte public-interface SP rule;
2. the selected `arghtos_port_start()` has zero static stack usage, so MSP
   stays 8-byte aligned until the first exception;
3. the first scheduler exception frame therefore has no hardware alignment
   padding; and
4. any higher-priority handler before PendSV restores that same underlying
   Thread frame, so tail-chaining does not alter its padding status.

Generic/debug profiles retain xPSR alignment decoding.

## Exact next action

1. Let PR #5 CI build physical R6ak and R6am in the same revision.
2. R6am must first pass `tools/check_stack_usage.py` proving selected
   `arghtos_port_start` static stack usage is exactly 0 B.
3. Require R6ak/R6am generated-frame audits, R6aj async-return regression,
   sanitizers, portability, historical controls, and all physical controls
   green.
4. Compare complete resident bytes and `PendSV_Handler`.
5. Accept R6am only for a strict physical reduction below 1328 B.
6. If the stack-usage premise fails, reject R6am regardless of linked size.
7. If neutral/regressive, preserve the result and keep R6ak as control.
8. Do not merge PR #5 without explicit user permission.
