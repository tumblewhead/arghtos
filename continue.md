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
  R6v accepted control         = 1396 B
  R6w rejected regressive      = 1400 B
  R6x rejected regressive      = 1400 B
  R6y rejected neutral         = 1396 B
  phase: R6z trusted selected crash payload
```

Recent rejected checkpoints:

* [implementation/R6w-early-periodic-ready.md](implementation/R6w-early-periodic-ready.md)
* [implementation/R6x-bounded-missed-loop.md](implementation/R6x-bounded-missed-loop.md)
* [implementation/R6y-packed-missed-add.md](implementation/R6y-packed-missed-add.md)

R6y authoritative CI:

```text
run:        40983178221
conclusion: success
R6v:       1396 B
R6y:       1396 B
delta:        0 B
```

R6z targets the 156-byte fatal path. The selected Cortex-M23 crash filler is
proved to emit exactly 2, 3, or 10 payload words; the host verification port
emits exactly 1. Thus every selected-port crash fill satisfies:

```text
1 <= port_words <= 10
```

R6z composes this with R6m rather than re-enabling the old R5w experiment:

* generated crash flags preseed PORT_STATE_VALID;
* TASK_VALID is ORed into that preseed;
* the post-fill >10 clamp is removed;
* the post-fill zero-payload test and PORT_STATE_VALID write are removed;
* R6m commit-word recursion protection and commit-last ordering remain intact.

Owning refinements:

* [architecture/contracts/C4-context-switch-port.md](architecture/contracts/C4-context-switch-port.md)
* [architecture/contracts/C5-fault-diagnostics.md](architecture/contracts/C5-fault-diagnostics.md)
* [proof/P2-context-stack-integrity.md](proof/P2-context-stack-integrity.md)

## Exact next action

1. Let PR #5 CI build physical R6v and R6z in the same revision.
2. Require crash-record, recursive-fatal, sanitizer, portability, historical
   reproduction, and physical-control gates green.
3. Compare complete resident bytes and `arghtos_fatal`.
4. Accept R6z only for a strict physical reduction.
5. If neutral/regressive, preserve the result and keep R6v = 1396 B as control.
6. Do not merge PR #5 without explicit user permission.
