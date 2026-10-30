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
  R6ai correctness control     = 1360 B
  phase: R6aj generated direct-async prebuilt entry repair
```

R6ae is preserved in
[implementation/R6ae-dispatch-owned-yield.md](implementation/R6ae-dispatch-owned-yield.md).

R6ae authoritative CI:

```text
run:        40985917356
conclusion: success
R6ad:      1360 B
R6ae:      1348 B
delta:      -12 B

arghtos_kernel_service:
  R6ad       144 B
  R6ae       132 B
arghtos_kernel_dispatch:
  both       116 B
```

R6af is preserved in
[implementation/R6af-unconditional-crash-task-id.md](implementation/R6af-unconditional-crash-task-id.md).

R6af authoritative CI:

```text
run:        40988335478
conclusion: success
R6ae:      1348 B
R6af:      1348 B
delta:        0 B
arghtos_fatal:
  both       136 B
```

R6ag is preserved in
[implementation/R6ag-timer-owned-sleep-release.md](implementation/R6ag-timer-owned-sleep-release.md).

R6ag authoritative CI:

```text
run:        40989065832
conclusion: success
R6ae:      1348 B
R6ag:      1340 B
delta:       -8 B

arghtos_kernel_service:
  R6ae       132 B
  R6ag       128 B
```

R6ah is preserved in
[implementation/R6ah-default-dispatch-services.md](implementation/R6ah-default-dispatch-services.md).

R6ah authoritative CI:

```text
run:        40989861862
conclusion: success
R6ag:      1340 B
R6ah:      1340 B
delta:        0 B
arghtos_kernel_service:
  both       128 B
```

R6ai exposed a semantic defect in the R5z raw-class schedule lineage.

R5z stores raw async priority in the async suffix, but the shared trampoline
still used `ARGHTOS_TASK_PERIOD(id) != 0` as its class test. For the canonical
async priority 3 this evaluates true, so the historical 18-byte trampoline
treated the async task as periodic and did not preserve the required
async-return-fatal lifecycle.

Errata are now attached to:

* [implementation/R5z-raw-class-schedule.md](implementation/R5z-raw-class-schedule.md)
* [implementation/R5-target-crossing-1534.md](implementation/R5-target-crossing-1534.md)

R6ai is the direct runtime correctness repair:

```text
run:        40990461247
conclusion: success

R6ag historical size-only               1340 B
R6ai generated-ID runtime class repair  1360 B

trampoline:
  R6ag                                     18 B
  R6ai                                     40 B
```

Therefore R6ag remains useful footprint provenance but is not a semantic
control for "async return is fatal." R6ai = 1360 B is the current correctness
control.

R6aj attempts to recover the lost 20 B without a runtime class branch:

* generated periodic frames keep stacked PC = periodic trampoline;
* the sole generated async frame uses stacked PC = application async entry;
* the async frame LR points to the existing return guard;
* under R6aj that guard records `ARGHTOS_FAULT_ASYNC_RETURNED`;
* periodic trampoline code is branchless/periodic-only;
* a relocation audit proves the canonical object has 23 trampoline PCs and one
  direct async PC before linking.

## Exact next action

1. Let PR #5 CI run R6ai and R6aj side by side.
2. Require the R6aj host async-return-guard regression and generated-frame
   relocation audit green.
3. Require portability, sanitizers, historical controls, and physical controls
   green.
4. Compare complete resident bytes and trampoline/return-guard symbols.
5. Accept R6aj only if it is semantically correct, stays <=1536 B, and strictly
   improves the 1360 B R6ai correctness control.
6. If accepted, R6aj becomes the new semantic control; compare its size to the
   1340 B historical R6ag size-only image to quantify the true repair cost.
7. Do not merge PR #5 without explicit user permission.
