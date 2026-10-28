# ArghtOS live continuation checkpoint

Last refreshed: 2026-10-28

Read [agents.md](agents.md) first. Repository state is authoritative.

## Active repository state

```text
repo:   https://github.com/tumblewhead/arghtos
base:   main @ dcf188aae8765d0d65d729af706da4bdaa922474
branch: implementation/r4-feature-specialization-001
PR:     #3 (draft)
```

Soft target:

```text
1536 B complete ArghtOS resident footprint
24-task default
```

Physical linker output-section occupancy is authoritative. All ArghtOS code,
constants, data, generated scheduler state, crash state, port code, assembly,
alignment/linker metadata, and ArghtOS-only MSP count.

## Minimum-size policy

Runtime profiling is optional in the minimum-size production profile and
enabled by default elsewhere.

Minimum-size work may trade cycles for bytes, but must retain scheduler
semantics, C11 defined behavior, portability boundaries, bounded work, crash
integrity, wake/wait correctness, and required invariants.

## Accepted footprint chain

```text
physical R3c                           3080 B
R4h corrected physical baseline       2778 B
R4m full LTO                           2530 B
R4n omit frame pointer                 2450 B
R4p no ARM unwind index                2434 B
R4q no jump tables                     2410 B
R4s split task metadata                2402 B
R4u entry/next-time lifetime overlay   2322 B
R4v compact PendSV assembly             2298 B
R4y direct SVC EXC_RETURN branch        2294 B
---------------------------------------------
current accepted minimum               2294 B
soft target                            1536 B
remaining gap                           758 B
```

R4y:

```text
linked before MSP  2214 B
MSP                  80 B
total               2294 B
```

Authoritative R4y CI evidence is recorded in
[implementation/R4y-direct-svc-return.md](implementation/R4y-direct-svc-return.md).

## Selective inlining/outlining conclusion

R4w global no-inline was strongly negative:

```text
R4v 2298 B
R4w 2594 B
delta +296 B
```

R4x then tested the strongest selective-outlining candidates while retaining
normal full LTO everywhere else.

Authoritative selective sweep:

```text
CI 40667730031  success

candidate                  total      delta vs R4y
R4y baseline               2294 B       0 B
fault attribution          2314 B     +20 B
time_due                   2310 B     +16 B
task_is_async              2326 B     +32 B
```

All host, sanitizer, portability, and Cortex-M23 gates passed.

Conclusion: for these candidates, Clang/LLVM `-Oz` + full LTO already makes
better size decisions than manual out-of-line boundaries.

Do not repeat these three helpers without a materially different ABI or
algorithmic hypothesis.

See [implementation/R4x-fault-helper.md](implementation/R4x-fault-helper.md).

## R4z timer-delta recomputation

R4z traded retained `best_delta` state for recomputation.

```text
R4y 2294 B
R4z 2294 B
delta   0 B
```

Reject as neutral.

See
[implementation/R4z-timer-delta-recompute.md](implementation/R4z-timer-delta-recompute.md).

## Rejected / neutral experiments

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

## Exact next action: freeze/clean PR #3 before merge

PR #3 now contains a large accepted R4 progression plus many rejected
experimental build switches and stale historical target names.

Before merging:

1. Keep the accepted minimum-size path through R4y.
2. Keep generic/debug and useful reference profiles needed for regression.
3. Remove or quarantine rejected experimental build targets/switches that are
   not part of the supported product surface.
4. Preserve implementation checkpoint documents for the experiment history.
5. Update PR #3 title/body to describe the full R4 footprint-specialization
   work rather than the old R4a-only description.
6. Refresh this file to the frozen branch head.
7. Run one final clean CI on that frozen branch.
8. If green and mergeable, merge PR #3, preferably as a squash merge.

Do not start another footprint experiment before this cleanup unless the user
explicitly chooses to continue research instead of freezing R4.

## Architecture rules

* portable kernel remains target-independent C11;
* Cortex-M23 mechanics remain under `port/cortex_m23/`;
* no dynamic allocation;
* periodic synchronous RMS semantics remain;
* async age/wake semantics remain;
* arithmetic missed-release catch-up remains;
* required 64-byte crash record remains;
* no hidden application-owned exclusions;
* do not remove invariants merely for bytes.

## Useful files

* [agents.md](agents.md)
* this file
* [implementation/R4u-entry-next-overlay.md](implementation/R4u-entry-next-overlay.md)
* [implementation/R4v-compact-pendsv.md](implementation/R4v-compact-pendsv.md)
* [implementation/R4y-direct-svc-return.md](implementation/R4y-direct-svc-return.md)
* [implementation/R4x-fault-helper.md](implementation/R4x-fault-helper.md)
* [implementation/R4z-timer-delta-recompute.md](implementation/R4z-timer-delta-recompute.md)
* `Makefile`
* `.github/workflows/ci.yml`
