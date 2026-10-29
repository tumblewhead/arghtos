# ArghtOS live continuation checkpoint

Last refreshed: 2026-10-29

Read [agents.md](agents.md) first. Repository state is authoritative.

## Active repository state

```text
repo:   https://github.com/tumblewhead/arghtos
base:   main @ bf44d8495074360f2fb6283322e323ff7f62d24f
branch: implementation/r5-size-specialization-001
PR:     #4 (ready for review)
phase:  R5 freeze/cleanup validation
```

R4 remains frozen on main. R5 research is isolated on PR #4.

## Target met

Default capacity remains:

```text
24 tasks
```

Final verified minimum-size Cortex-M23 result:

```text
R4y frozen control             2294 B
R5ag accepted minimum           1534 B
soft target                     1536 B
margin                             2 B
saved from R4y                    760 B
```

R5ag physical accounting:

```text
linked before MSP  1470 B
ArghtOS-only MSP      64 B
-------------------------
complete resident   1534 B
target              1536 B
target_met          true
```

Authoritative target-crossing CI:

```text
run:       40812507607
code head: 1a381be85a2507fc3071e4616a4bb7d770d77adf
conclusion: success
```

The branch contains documentation-only commits after the measured code head.

See:

* [implementation/R5-target-crossing-1534.md](implementation/R5-target-crossing-1534.md)
* [implementation/R5y-proven-periodic-state.md](implementation/R5y-proven-periodic-state.md)

## Late-stage chain

```text
R5y   1646 B
R5z   1618 B   -28 B   raw class-partitioned schedule
R5aa  1618 B     0 B   delta-only timer, rejected neutral
R5ab  1590 B   -28 B   single-async generated selector
R5ac  1550 B   -40 B   terminal fault-state mutation elision
R5ad  1542 B    -8 B   compact Cortex-M23 API veneers + handler cuts
R5ae  1542 B     0 B   local veneer section, rejected neutral
R5af  1538 B    -4 B   14-byte assembly HardFault context bridge
R5ag  1534 B    -4 B   direct terminal-hook linker alias
```

## Accepted R5 architectural specializations

R5ag includes the accepted R4y minimum profile plus:

* compact PendSV/HardFault EXC_RETURN handling;
* compact one-byte service outcome;
* async age/deadline lifetime overlay;
* fixed zero-overflow schedule8;
* generated prebuilt Cortex-M23 initial task frames;
* pristine generated startup state;
* proof-discharged dispatch/trampoline checks;
* generated minimum-period startup timer;
* generated task-class / RMS ordering;
* partitioned periodic/async time scan;
* proof-redundant async age-reset elision;
* preseeded generated crash-record invariants;
* always-armed timer when periodic tasks are guaranteed;
* 64-byte ArghtOS-only MSP requalification;
* proof-redundant periodic-state fault-branch removal;
* raw class-partitioned schedule values;
* single-async selector specialization for the measured generated profile;
* terminal-only fault-state mutation elision;
* compact target service veneers and handler branches;
* assembly hard-fault context bridge using two PUSH instructions;
* link-time alias from `arghtos_port_fault_terminal` directly to the
  Cortex-M23 board terminal hook.

## Required preserved behavior

The accepted target-crossing profile still preserves:

* synchronous periodic RMS behavior and lower-ID tie semantics;
* one active periodic instance;
* arithmetic delayed-release catch-up;
* async residual-capacity behavior and required age-credit semantics;
* wake/wait pending-wake race closure;
* async return as fatal;
* required 64-byte crash record and commit-last protocol;
* recursive-fatal protection;
* no heap;
* bounded scheduler work;
* portable target-independent C11 scheduler core;
* Cortex-M23 task context and PSP/MSP ownership;
* first-dispatch MSP reclamation and xPSR alignment handling;
* PRIMASK serialization and race-free idle protocol.

Generated-profile proofs/preconditions are part of the accepted result and must
not be silently generalized to arbitrary runtime configuration.

## Important rejected / neutral experiments

Historical rejected/neutral results remain evidence and should not be
resurrected without a new hypothesis.

Notable late examples:

```text
R5aa delta-only timer                  0 B
R5ae explicit local veneer section     0 B
R5k assembly crash-fill split        +12 B
R5v unit-period timer                 +24 B versus accepted control
R5w trusted crash-port split           +8 B versus R5u
```

Earlier R4 rejected experiments remain documented under `implementation/`.

## Footprint caveat

The R5ag margin is only 2 B.

Any of these may move the image back over 1536 B:

* source reordering;
* linker-script changes;
* compiler or LLD version changes;
* changing generated task count/class mix;
* changing the 23-periodic + 1-async measured generated fixture assumptions;
* adding diagnostics or validation to the minimum profile.

Never claim the target is still met without re-linking the Cortex-M23 R5ag
profile and reading the physical footprint report.

## Freeze / cleanup checkpoint

R5ag remains the frozen accepted control. Cleanup changes are build-system and
CI-surface changes only; they do not intentionally change R5ag semantics.

Cleanup commits:

```text
ad2b286aa6ddc4528cd2fb201601cd4eaafca774  freeze Makefile around durable controls
8b1bcffdcd81be2d18a2ac9973cd9e5c6f10060e  freeze CI around supported controls
```

The Makefile experiment surface was collapsed from 4791 lines to 264 lines.
Historical experiment documents under `implementation/` remain preserved as
evidence. The supported/reference build surface now retains:

* generic portability and host tests;
* accepted size-first selector host regression;
* R3c profiling-enabled reference;
* R4u host minimum-source reference;
* R5ag host semantic profile using the accepted R5 core specializations;
* ASan/UBSan;
* generic Cortex-M23 compile;
* R3c physical footprint reference;
* frozen R4y physical footprint;
* frozen R5ag physical footprint.

CI now explicitly fails if `build/footprint-r5ag.json` reports
`arghtos_resident_total > 1536` or `target_met != true`.

No historical experiment document was deleted.

## Cleaned validation result

The freeze/cleanup build-system head was:

```text
a9df59100571938073d76151b9da83158ffc766e
```

Authoritative cleaned CI:

```text
run:        40814851219
workflow:   arghtos-ci
conclusion: success
```

Cleaned R5ag physical accounting from that run:

```text
linked before MSP  1470 B
ArghtOS-only MSP      64 B
-------------------------
complete resident   1534 B
target              1536 B
margin                 2 B
target_met           true
```

Every retained portability, host, sanitizer, generic Cortex-M23, R3c, R4y,
R5ag, symbol-report, and artifact-upload gate completed successfully. The
explicit CI resident-footprint assertion also passed.

This proves the Makefile/CI cleanup did not disturb the frozen R5ag target
crossing.

## PR state

PR #4 is now ready for review.

The cleaned documentation-head validation was:

```text
head:       48ab4de3c192409de0a9f84543e98a2db468dfe4
CI run:     40814905429
conclusion: success
```

No merge has been performed.

## Exact next action

1. Treat R5ag = 1534 B as the frozen R5 control.
2. Confirm CI remains green on this documentation-only continuation update.
3. Do not merge PR #4 unless the user explicitly requests it.
4. If more size research is desired, start a new challenger beside R5ag rather
   than modifying the frozen control.
