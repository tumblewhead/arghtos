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
R6i:    accepted current control  = 1470 B
saved from R5ag                  =   64 B
saved from R4y                   =  824 B
```

Rejected-neutral checkpoints:

* [implementation/R6g-proven-current-domain.md](implementation/R6g-proven-current-domain.md)
* [implementation/R6h-single-async-time.md](implementation/R6h-single-async-time.md)

Accepted R6i checkpoint:

* [implementation/R6i-known-unmasked-pendsv.md](implementation/R6i-known-unmasked-pendsv.md)

R6i authoritative measurement CI:

```text
run:        40818322636
conclusion: success
measured head:
            62dabb5dab89bfc8861fd91a7ab0868a4e5f8a8f
```

R6i physical accounting:

```text
linked before MSP  1406 B
ArghtOS-only MSP      64 B
-------------------------
complete resident   1470 B
```

R6i depends on the trusted Cortex-M23 profile guaranteeing PRIMASK=0 at every
supported PendSV entry. PendSV still masks across kernel dispatch and idle, and
interrupts are restored at the same pre-return point. The generic target path
retains exact prior-PRIMASK save/restore.

R5ag remains immutable and continues to build as the merged R5 reference.

## Exact next action

1. Treat R6i = 1470 B as the accepted R6 control.
2. Let CI validate this result-documentation-only head.
3. Inspect remaining large symbols before selecting R6j. Current primary
   candidates are `process_time_locked`, `arghtos_fatal`,
   `arghtos_kernel_service`, and `arghtos_kernel_dispatch`.
4. Avoid revisiting R6g/R6h without a materially different hypothesis.
5. Keep R6j as a side-by-side challenger against R6i and preserve R5ag.
6. Do not merge PR #5 without explicit user permission.
