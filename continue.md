# ArghtOS live continuation checkpoint

Last refreshed: 2026-10-28

This file is the live handoff. Read [agents.md](agents.md) first.

## Canonical repository

```text
https://github.com/tumblewhead/arghtos
```

## Canonical base

R2c was merged to `main` as:

```text
4988c639bebd6a02ab9c037b3fb0f879c58ef46b
Merge pull request #1
I1: footprint reduction experiments through R2c
```

Do not resume R3 from the old I1 branch.

## Active work

Branch:

```text
implementation/r3-production-specialization-001
```

Draft PR:

```text
#2 R3: production-profile validation and service specialization
https://github.com/tumblewhead/arghtos/pull/2
```

The branch is based directly on merged R2c `main`.

## Current objective

Reduce the complete ArghtOS SRAM-resident footprint while preserving the valid-use scheduler semantics and keeping the generic/debug profile as the verification/reference surface.

Soft target:

```text
1536 B complete ArghtOS resident footprint
```

Current default capacity:

```text
24 tasks
```

All code, const data, metadata, port code, assembly, crash storage, and ArghtOS-only MSP count.

## Important requirement state

### Async priorities

```text
0x00       reserved / no async priority
0x01..MAX legal
0xFF       reserved / invalid
ARGHTOS_ASYNC_PRIORITY_MAX <= 254
```

### Portability

`src/arghtos_kernel.c` is target-independent C11.

Target-specific code belongs under `port/<target>/`.

Read [design-elements/D13-porting-guide.md](design-elements/D13-porting-guide.md) before changing the port boundary.

## Historical footprint progression

Frozen I0:

```text
4226 B
```

R1/R2 progression at 24-task requirement:

```text
24-task R1b        3716 B
R2a compact state  3702 B
R2b compact desc   3592 B
R2c unified slot   3376 B
```

R2c representation:

```text
context / initial stack top   4 B
next_time                     4 B
total_cpu_ticks               4 B
max_run_ticks                 2 B
age_credit                    1 B
packed status                 1 B
entry                         4 B
schedule                      4 B
---------------------------------
24 B/task
24 tasks = 576 B
```

R2c merged to `main`.

## R3 measured results

R3 starts from merged R2c = 3376 B.

### R3a: trusted call context

Production profile assumes task APIs are called from task context and interrupt wake from an approved handler context.

Measured:

```text
3376 -> 3204 B
saving = 172 B
```

### R3b: specialized service legality

Additional production assumptions:

* service is legal for generated task class;
* wake ID is a generated valid async task ID;
* service entry corresponds to the running task.

Measured:

```text
3204 -> 3164 B
saving = 40 B
```

### R3c: minimal production validation

Additional production assumptions:

* init is called once;
* start is called once after successful init;
* interrupt wake occurs only after start;
* profile pointer and task ID are valid;
* sleep durations/deadlines obey half-range legality.

Measured:

```text
3164 -> 3072 B
saving = 92 B
```

Total R3 saving:

```text
3376 -> 3072 B
saving = 304 B
```

Relative to I0:

```text
4226 -> 3072 B
saving = 1154 B
```

Current footprint is exactly:

```text
3072 B = 2 * 1536 B target
```

## R3c footprint breakdown

CI evidence: GitHub Actions run `40653715778`.

All generic/R1/R2/R3 host profiles, ASan/UBSan, portability checks, Cortex-M23 builds, footprint links, and symbol reporting passed.

Measured R3c:

```text
kernel code/data        1830 B
Cortex-M23 port C        282 B
context assembly         188 B
board adapter             12 B
unified task slots        576 B
crash record              64 B
linker metadata           16 B
ArghtOS-only MSP         104 B
------------------------------
total                   3072 B
```

Largest retained symbols:

```text
arghtos_static_slots          576 B
process_time_locked           292 B
arghtos_kernel_service        256 B
arghtos_select_next           232 B
arghtos_kernel_dispatch       204 B
arghtos_fatal                 180 B
PendSV_Handler                130 B
arghtos_start                 128 B
wake_locked                   112 B
arghtos_init                  100 B
arghtos_port_crash_fill        84 B
arghtos_task_start_trampoline  76 B
arghtos_profile_get            60 B
```

## R3 production trust boundary

R3c removes selected application misuse validation only.

Still retained:

* context-construction failure detection;
* FAULTED-state wake handling;
* unknown-service fatal handling;
* dispatch/current-task/context/state invariants;
* task trampoline ID invariant;
* fatal recursion handling;
* timer/state invariants.

Contracts C0, C1, C2, and C5 on the active branch document the R3 production preconditions.

Implementation checkpoint:

[implementation/R3-production-specialization.md](implementation/R3-production-specialization.md)

## Current repository state

R3 implementation and documentation are on the active branch and PR #2.

R3 is measured and internally documented.

PR #2 remains draft.

Do not merge it unless the user explicitly approves merging.

## Current strategic conclusion

Validation specialization has reached diminishing returns.

Measured incremental R3 gains:

```text
R3a 172 B
R3b  40 B
R3c  92 B
```

Further removal of scheduler-internal invariants is not recommended under R3.

The remaining footprint is dominated by actual functionality:

```text
process_time_locked       292 B
kernel service path       256 B
selector                  232 B
dispatch                  204 B
fatal/crash               180 B + port crash fill
context assembly          188 B
task slots                576 B
```

## Exact next action

Start **R4a: profiling-surface specialization experiment** from the active R3 branch.

Do not remove profiling canonically yet.

The goal is to measure the maximum footprint attributable to the profiling feature before asking the user whether the requirement should change.

### R4a experiment

Add a compile-time experimental profile on top of R3c that disables runtime profiling only in that profile.

Measure removal of:

* public `arghtos_profile_get()`;
* per-task `total_cpu_ticks` storage;
* per-task `max_run_ticks` storage;
* run-start accounting that exists only for profiling;
* profile snapshot copying;
* any port/kernel code that dead-strips as a consequence.

Do **not** remove:

* periodic missed-release evidence if it is needed for scheduler/diagnostic correctness;
* async age credit because it is scheduler state;
* crash logging;
* fatal handling;
* scheduler timing/release behavior.

### R4a expected structural opportunity

Current R3c task slot contains 6 bytes/task used specifically by profiling:

```text
total_cpu_ticks  4 B
max_run_ticks    2 B
--------------------
6 B/task
24 tasks = 144 B
```

There is also executable accounting and API code:

* `arghtos_profile_get` ~60 B;
* portions of dispatch accounting;
* `run_start` global state may become removable;
* related helper/code paths may shrink.

R4a therefore has a plausible saving materially larger than another round of validation stripping.

### R4a acceptance procedure

1. Add a new experimental build profile, keeping R3c unchanged beside it.
2. Keep generic/debug profile tests.
3. Add host tests proving scheduling semantics are unchanged when profiling is compiled out.
4. Run ASan/UBSan.
5. Run portability check.
6. Compile Cortex-M23.
7. Link R3c and R4a in the same CI revision.
8. Record exact linked delta and symbol changes.
9. Update `implementation/` with the result.
10. Refresh this `continue.md` with the result and the exact next action.

If R4a saves meaningful space, present the measurement to the user before making profiling optional/canonical because profiling is currently a documented v1 requirement.

## Files to read before R4a

Read these first:

* [agents.md](agents.md)
* this file
* [implementation/R3-production-specialization.md](implementation/R3-production-specialization.md)
* [architecture/contracts/C5-fault-diagnostics.md](architecture/contracts/C5-fault-diagnostics.md)
* [architecture/contracts/C8-task-control-record.md](architecture/contracts/C8-task-control-record.md)
* [design-elements/D10-build-footprint.md](design-elements/D10-build-footprint.md)
* [design-elements/D11-test-seams.md](design-elements/D11-test-seams.md)
* `src/arghtos_internal.h`
* `src/arghtos_kernel.c`
* `Makefile`
* `.github/workflows/ci.yml`

Do not reread the entire repository unless a contradiction forces it.

## Known bookkeeping issue

The footprint report still uses the legacy JSON key:

```text
task_descriptors
```

for the R2c/R3 unified `.arghtos_cfg` slot array.

In R3c that 576 B is the complete mutable unified task-slot array, not a separate descriptor table.

This is a reporting-label cleanup, not a physical accounting error.

## Handoff rule

After R4a or any subsequent experiment, update this file before ending the session.

At minimum refresh:

* active branch/PR;
* latest passing CI run;
* current best footprint;
* accepted/rejected profile;
* exact next action;
* any changed requirement or trust precondition.
