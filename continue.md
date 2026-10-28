# ArghtOS live continuation checkpoint

Last refreshed: 2026-10-28

Read [agents.md](agents.md) first. Repository state is authoritative.

## Active repository state

```text
repo:   https://github.com/tumblewhead/arghtos
base:   main @ dcf188aae8765d0d65d729af706da4bdaa922474
branch: implementation/r4-feature-specialization-001
PR:     #3
title:  R4: freeze minimum-footprint Cortex-M23 production profile at 2294 B
```

PR #3 is in **freeze / merge-gate mode**. Do not start another R4 footprint
experiment on this branch.

## Frozen result

The final same-revision R3c reference is 3104 B. Earlier R4 documents may show
the historical 3080 B R3c value from earlier source revisions; do not rewrite
those historical measurements.

Default capacity:

```text
24 tasks
```

Complete physical ArghtOS resident footprint:

```text
physical R3c final reference          3104 B
R4h corrected physical baseline      2778 B
R4m full LTO                          2530 B
R4n omit frame pointer                2450 B
R4p no ARM unwind index               2434 B
R4q no jump tables                    2410 B
R4s split task metadata               2402 B
R4u entry/next-time lifetime overlay  2322 B
R4v compact PendSV assembly            2298 B
R4y direct SVC EXC_RETURN branch       2294 B
--------------------------------------------
frozen accepted minimum               2294 B
soft target                           1536 B
remaining gap                          758 B
```

R4y:

```text
linked before MSP  2214 B
ArghtOS-only MSP      80 B
total               2294 B
```

See:

* [implementation/R4-freeze.md](implementation/R4-freeze.md)
* [implementation/R4y-direct-svc-return.md](implementation/R4y-direct-svc-return.md)

## Supported minimum-size profile

The frozen R4y minimum-size Cortex-M23 profile uses:

* static/trusted production configuration;
* 24-task maximum;
* compact runtime + compact descriptor + unified slot;
* optional runtime profiling disabled only in this minimum-size profile;
* accepted size-first selector;
* no-profile dispatch clock read removed;
* unused start epoch removed;
* split task metadata;
* entry/next-time lifetime overlay;
* full LTO;
* `-fomit-frame-pointer`;
* `-fno-jump-tables`;
* linker script without unused ARM unwind index;
* compact Thumb-1 PendSV save/restore;
* direct SVC branch to saved EXC_RETURN.

The portable kernel remains target-independent C11.

## Preserved semantics / contracts

R4y preserves:

* synchronous periodic RMS scheduling;
* equal-period lower-ID tie order;
* one active periodic instance;
* arithmetic delayed-release catch-up;
* async residual-capacity scheduling and age credit;
* pending-wake race closure;
* async return as fatal;
* required 64-byte crash record;
* no heap;
* bounded scheduler work;
* portable kernel / target-port boundary;
* Cortex-M23 32-byte software R4-R11 task frame;
* first-dispatch MSP reclamation;
* PRIMASK serialization and race-free idle WFI protocol.

## Rejected experiment history

Rejected/neutral R4 experiments remain documented under `implementation/`.
They are evidence, not supported build profiles.

Notable rejected/neutral results include:

```text
R4c looped crash copy                 +8 B
R4d whole-prefix crash clear           0 B
R4e timer sentinel                    +4 B
R4f specialized startup timer          0 B
R4i service if-chain                   0 B
R4j fault-range source rewrite         0 B
R4k two-pass time engine              +4 B
R4l wake-result hoist                  0 B
R4o lld --icf=safe                     0 B
R4r forced machine outliner            0 B
R4t ID-native metadata indexing        0 B
R4w global no-inline                +296 B
R4x fault helper                     +20 B
R4x time_due helper                  +16 B
R4x task_is_async helper             +32 B
R4z timer-delta recomputation          0 B
```

The post-freeze R4aa/R4ab research commits were intentionally removed from the
PR branch before merge because they were not accepted checkpoints.

## Freeze cleanup completed

The merge branch now:

* removes post-R4y experimental helper translation units;
* removes rejected R4t/R4x/R4z private source switches;
* removes rejected legacy kernel/port experiment branches from product source;
* keeps useful wake/state regression coverage;
* collapses the Makefile from the lab matrix to supported/reference profiles;
* collapses CI to supported/reference profiles;
* preserves all measured experiment checkpoint documents;
* updates PR #3 title/body to the complete R4 scope.

Supported Makefile/CI surfaces are now:

* generic host tests;
* accepted size-first selector host regression;
* profiling-enabled R3c reference;
* accepted minimum-source R4u host tests;
* ASan/UBSan;
* generic Cortex-M23 compile;
* generic footprint harness;
* R3c footprint reference;
* final R4y footprint;
* R3c/R4y map and symbol evidence.

## Final merge-gate evidence

```text
CI run:    40669529301
head:      c62e8ac51af76e9ed148e856b2a808c64c1f4cc9
conclusion: success
```

All frozen CI steps passed.

Final same-revision footprint evidence:

```text
R3c reference  3104 B
R4y minimum    2294 B
saved           810 B
```

The commit that records this evidence is documentation-only. Its CI must remain
green before PR #3 is marked ready.

## Exact next action

1. Confirm CI for this documentation-only handoff commit is green.
2. Confirm PR #3 remains mergeable.
3. Mark PR #3 ready for review/merge.
4. User should merge PR #3, preferably with **Squash and merge**.
5. After merge, start any new footprint research from fresh canonical `main`,
   using frozen R4y as the control.

Do not add another experiment to PR #3.
