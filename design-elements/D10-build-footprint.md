---
type: CodeModuleDesign
title: "D10 - Build, linker, resident-footprint enforcement, and module budgets"
description: "Design of the build/size pipeline that treats all ArghtOS code, constants, metadata, and MSP as SRAM-resident and measures the 1.5 KiB soft target from the first skeleton."
tags: [design, build, linker, sram, footprint, budget]
status: draft
---

# Goal

Make resident footprint a continuously measured design variable rather than a late-stage surprise.

# Fundamental accounting rule

Every ArghtOS-induced byte resident in SRAM counts.

This includes:

* portable kernel machine code;
* Cortex-M23 port machine code;
* assembly;
* literal pools;
* jump tables;
* const/read-only objects;
* C0 descriptor table;
* C8 controls;
* C9 state;
* required crash record;
* ArghtOS-only MSP requirement;
* code emitted into application translation units by ArghtOS headers.

Section names do not create exclusions.

# Known fixed lower bound at C=N=32

```text
C0 descriptors      512 B
C8 controls         640 B
C9 state             20 B
crash record         64 B
--------------------------
pre-code total     1236 B

soft target        1536 B
--------------------------
remaining           300 B
```

That 300 B is the envelope for code/constants plus ArghtOS-only MSP.

This is an extremely aggressive target.

# Module red-line budgets

The following are **measurement red lines, not forecasts**. They intentionally sum to the 300-byte residual envelope.

| Element | Resident code+const red line |
|---|---:|
| D01 public API wrappers/ABI-induced code | 20 B |
| D02 init/config/start | 40 B |
| D03 lifecycle/wake/service | 36 B |
| D04 selector/aging | 32 B |
| D05 time/release | 36 B |
| D06 Cortex execution/context/time glue | 48 B |
| D07 diagnostics/fatal/profile | 24 B |
| D08 task trampoline/return guard | 8 B |
| D09 integration-only glue | 8 B |
| **Code/const subtotal** | **252 B** |
| **ArghtOS-only MSP red line** | **48 B** |
| **Residual envelope** | **300 B** |

These numbers are intentionally uncomfortable.

Their purpose is to reveal infeasibility immediately.

I0 has now run this experiment. The current linked implementation is about 4106 B before MSP, so the 300 B residual envelope is decisively exceeded. The correct response is to revisit the soft target, task-capacity metadata, or feature representation rather than massage the accounting.

# Budget attribution rules

A function belongs to the logical design element that owns its behavior, even though functions share `arghtos_kernel.o`.

Symbol/map tooling must attribute code by symbol where possible.

Shared helpers are charged to:

* the module that owns the semantics; or
* a separately reported shared-helper bucket that still counts in the 252 B subtotal.

Do not count the same bytes twice.

# Build variants

At minimum:

## Host verification build

Purpose:

* sanitizers;
* deterministic fake port;
* exhaustive/property tests.

Size is not representative.

## Cortex-M23 footprint build

Purpose:

* actual target ISA;
* production feature set;
* map/disassembly;
* stack-usage reports.

Recommended initial compiler experiments:

```text
-Os
-ffunction-sections
-fdata-sections
-Wl,--gc-sections
```

Also measure:

* LTO on/off;
* alternative size optimization flag if toolchain supports it;
* single-TU versus physically split kernel.

No compiler option is canonical until measurements show smallest correct image.

# Linker-map authority

Every footprint result should archive:

* ELF;
* linker map;
* section-size summary;
* symbol-size report;
* disassembly;
* compiler version/options;
* `ARGHTOS_MAX_TASKS`;
* configured `task_count`;
* MSP reservation assumptions.

# Recommended section attribution

If the linker/toolchain supports it cleanly, ArghtOS objects may be grouped into named output sections such as:

```text
.arghtos_text
.arghtos_rodata
.arghtos_data
.arghtos_bss
.arghtos_noinit
```

This is for measurement only.

Do not add veneers/copies solely to obtain pretty section names.

If grouping changes code size, use map/symbol attribution instead.

# Header-induced code accounting

D01/C10 prohibit assuming application-generated inline code is free.

A footprint test application should instantiate every public API once and measure any ArghtOS-induced code outside the core object files.

# MSP measurement

Two values are retained:

* `M_MSP_arghtos`: worst ArghtOS-owned execution depth used in the 1.5 KiB subtotal;
* `M_MSP_system`: physical stack reservation required by full handler nesting.

Evidence sources:

* compiler stack-usage output;
* manual assembly stack analysis;
* target high-water/canary measurements;
* worst allowed exception nesting model.

The 48 B red line above is aspirational and may prove impossible.

# Automatic size gates

CI/build tooling should check:

```text
sizeof(task_config) == 16
sizeof(task_control) == 20
sizeof(kernel_state) == 20
sizeof(crash_record) == 64
```

and report:

```text
pre-code metadata bytes
kernel/port code+const bytes
other ArghtOS bytes
M_MSP_arghtos
F_arghtos total
F_system total
```

Because 1.5 KiB is a soft target, ordinary development builds may warn when it is exceeded.

A build claiming the footprint milestone must fail if `F_arghtos > 1536`.

# No-hidden-table rule

Production ArghtOS code may not add a resident table scaling with:

* `ARGHTOS_MAX_TASKS`;
* number of service IDs;
* number of fault reasons;
* task states;

without explicit contract/design review and map evidence.

Examples to scrutinize:

* debug name tables;
* function-pointer dispatch tables;
* priority lookup arrays;
* precomputed deadline arrays;
* string tables;
* state-name arrays.

# I0 skeleton checkpoint result

The checkpoint has been executed. The current Cortex-M23 `-Oz` linked footprint report is:

```text
portable kernel + resident state     2938 B
Cortex-M23 port C                     408 B
context + fault assembly              172 B
board adapter                          12 B
32 task descriptors                   512 B
required crash record                  64 B
linker resident metadata               16 B
-------------------------------------------
ArghtOS before MSP                   4122 B
soft target                          1536 B
excess                               2586 B
```

Compiler sweep showed 3308 B linked text at `-Os`, 3056 B at `-Oz`, and 2764 B with exploratory `-Oz + LTO`. This confirms a feature/architecture-scale gap rather than an optimization-flag problem.

Evidence: [I0 footprint skeleton](../implementation/I0-footprint-skeleton.md).

# Original skeleton checkpoint procedure

The checkpoint procedure was:



* C0/C8/C9 objects;
* public functions;
* empty-but-correct port entry symbols;
* crash record type;
* minimal start/dispatch stubs.

Link it for Cortex-M23 and capture the first map.

This immediately tells us how much of the 300 B residual is consumed by ABI/prologue/exception scaffolding alone.

# Dangerous build/code sections and tester guidance

## 1. Counting only writable sections

**Bad report:**

```text
.data + .bss = 660 B
therefore scheduler fits
```

while 900 B of code/rodata also resides in SRAM.

**Tester should:**

* reconstruct the total from ELF program/section headers;
* reconcile against the linker map;
* explicitly list text, rodata, literals, data, bss, noinit, metadata, MSP.

## 2. Double-counting descriptor/control bytes

If section totals already include C0/C8/C9, adding "metadata 1236 B" again gives a false oversized result.

**Tester should:**

* provide both a conceptual breakdown and one physical linker total;
* verify each byte range belongs to exactly one total category.

## 3. LTO removes a function only in test app

A footprint harness that never calls sleep/profile can make dead stripping look artificially excellent.

**Tester should:**

* footprint harness must exercise/reference every required public feature;
* keep one reachable path to each mandatory function;
* compare symbol retention against the API list.

## 4. Switch compiler emits jump table

A compact-looking C switch may produce both code and rodata.

**Tester should:**

* inspect disassembly/map after every service/state switch change;
* compare if-chain versus switch;
* use the smaller linked result.

## 5. Stack usage report ignores assembly

Compiler `.su` files do not account for manual pushes in D06 assembly or hardware exception stacking.

**Tester should:**

* manually account assembly pushes and architectural frames;
* combine with C stack reports;
* validate with target high-water measurements.

## 6. Weak/default port hook survives in image alongside board override

Depending on linker behavior, an unexpected default implementation may remain resident.

**Tester should:**

* inspect symbol/map for duplicate/default port bodies;
* ensure only selected implementations are linked;
* prefer explicit link selection over resident unused defaults when smaller.

## 7. Debug assertions add file/function strings

`assert()` may pull filenames, line formatting, and abort machinery.

**Tester should:**

* minimum-footprint production build must inspect strings/symbol dependencies;
* use numeric fatal reasons for kernel invariants;
* compare debug and production map separately.

## 8. "One tiny helper" breaches its red line

At this scale a 20-byte helper is significant.

**Tester should:**

* produce per-symbol size diffs on every PR;
* highlight >8-byte changes in core code;
* require explanation for new literal pools or veneers.

# Test/build seam

D10 should eventually be automated by a small deterministic size-report script that consumes ELF/map files and emits machine-readable JSON plus a human Markdown summary.

# Boundary specification

D10 owns measurement and build acceptance, not scheduler runtime behavior.


## Static MSP result

The stabilized `-Oz` compiler stack-usage artifacts plus the SVC handler's explicit 8-byte save give a current normal-path ArghtOS-only MSP lower bound of approximately 104 B.

That already exceeds the original 48 B MSP red line and yields:

```text
4122 B linked + 104 B static MSP = 4226 B lower bound
```

Target high-water testing is still required; 104 B is not a final physical MSP reservation.

# Current 24-task R2c footprint profile

The original 32-task red-line table above was the pre-implementation design
target and is retained as historical design evidence.

The current default capacity is 24 tasks.

The active R2c compact profile has measured:

```text
unified task slots        576 B
static C9                  12 B
crash record               64 B
linker metadata            16 B
ArghtOS-only MSP          104 B
portable kernel/code     2076 B
Cortex-M23 port C         340 B
context assembly          188 B
board adapter              12 B
------------------------------
total                    3376 B
```

CI run: 40649991293.

The build system must continue to report the generic/R1/R2 profiles
side-by-side until the compact profile is promoted from experiment to the
canonical implementation.

The `task_descriptors` key emitted by the current footprint script is a legacy
label. For R2c, the `.arghtos_cfg` section contains the entire 576-byte mutable
unified slot array, not a separate descriptor table. A future reporting cleanup
should rename this field without changing the physical accounting.

# Current R3c minimum-footprint profile

R3 production specialization reduces the merged R2c 3376 B profile to:

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

CI run: 40653715778.

R3 breakdown:

```text
R2c 3376 B
R3a 3204 B   -172 B
R3b 3164 B    -40 B
R3c 3072 B    -92 B
```

The target remains unmet by 1536 B.

Validation specialization is no longer the dominant remaining lever; further footprint work should move to explicit API/feature specialization or port/code-structure reduction.
# R4a profiling-surface promotion

The minimum-size production profile may compile runtime profiling out.

Measured same-revision comparison:

```text
                         R3c    R4a   delta
kernel code/data         1830   1699  -131 B
unified task slots        576    480   -96 B
all other categories      666    666     0 B
-------------------------------------------
total                    3072   2845  -227 B
```

CI evidence: run 40658079082.

The minimum-size baseline for subsequent size experiments is therefore 2845 B.
The generic/debug and profiling-enabled production builds remain in CI as
behavioral/reference profiles.
