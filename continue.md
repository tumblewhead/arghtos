# ArghtOS live continuation checkpoint

Last refreshed: 2026-10-28

Read [agents.md](agents.md) first. Repository state is authoritative.

## Active repository state

```text
repo:   https://github.com/tumblewhead/arghtos
base:   main @ bf44d8495074360f2fb6283322e323ff7f62d24f
branch: implementation/r5-size-specialization-001
phase:  R5 footprint reduction
```

PR #3 was squash-merged to main. R4 is frozen.

## Frozen control

Default capacity:

```text
24 tasks
```

Final R4 control:

```text
R4y frozen control             = 2294 B
R5a accepted minimum            = 2290 B
R5b accepted minimum            = 2286 B
R5d accepted minimum            = 2270 B
R5e accepted minimum            = 2202 B
R5h accepted minimum            = 1970 B
R5i accepted minimum            = 1914 B
R5j accepted minimum            = 1906 B
linked before MSP               = 1834 B
ArghtOS-only MSP                =   72 B
soft target                     = 1536 B
remaining gap                   =  370 B
```

The final same-revision profiling-enabled R3c reference on the frozen R4 source
was 3104 B.

R4y remains the immutable control for R5 experiments.

## R5 rules

R5 may trade runtime cycles for resident bytes but must preserve:

* portable C11 kernel boundary;
* synchronous periodic RMS semantics;
* async residual-capacity semantics and age credit;
* wake/wait race closure;
* arithmetic delayed-release catch-up;
* required 64-byte crash record;
* no heap;
* bounded work;
* Cortex-M23 task-context ABI unless an owning contract/proof is explicitly
  updated;
* correct first-dispatch MSP reclamation and alignment handling;
* PRIMASK and race-free idle protocol.

Only linked Cortex-M23 evidence counts.

## Current accepted minimum-size profile

R4y uses:

* trusted/static production configuration;
* compact runtime/descriptor/unified slot;
* no runtime profiling in minimum profile;
* size-first selector;
* no dispatch clock read;
* no start epoch;
* split task metadata;
* entry/next-time lifetime overlay;
* full LTO;
* `-fomit-frame-pointer`;
* `-fno-jump-tables`;
* no ARM unwind index;
* compact PendSV R4-R11 save/restore;
* direct SVC branch to saved EXC_RETURN.

## R5a accepted result

Authoritative CI:

```text
40670407899  success

R4y 2294 B
R5a 2290 B
saved   4 B
```

`PendSV_Handler` shrank from 108 B to 104 B. All measured C symbols are
unchanged and the 80 B MSP bound remains valid.

See
[implementation/R5a-compact-pendsv-tail.md](implementation/R5a-compact-pendsv-tail.md).

## R5b accepted result

Authoritative CI:

```text
40670635274  success

R5a 2290 B
R5b 2286 B
saved   4 B
```

PendSV shrank 104 -> 102 B and HardFault 28 -> 26 B. MSP remains 80 B.

See
[implementation/R5b-compact-exc-checks.md](implementation/R5b-compact-exc-checks.md).

## R5c rejected result

```text
R5b 2286 B
R5c 2286 B
delta   0 B
```

Compact wake encoding shrank the wake helper by 12 B but enlarged
`arghtos_kernel_service` by 12 B because the encoded value was immediately
decoded back into separate result/dispatch locals.

See
[implementation/R5c-compact-wake-result.md](implementation/R5c-compact-wake-result.md).

## R5d accepted result

```text
CI 40671500777  success

R5b 2286 B
R5d 2270 B
saved 16 B
```

R5d carries one encoded service outcome byte through the entire specialized
service dispatcher.

See
[implementation/R5d-compact-service-outcome.md](implementation/R5d-compact-service-outcome.md).

## R5e accepted result

```text
CI 40671952282  success

R5d 2270 B
R5e 2202 B
saved 68 B
```

R5e overlays async age credit with the async sleep deadline. Task/config state
falls 336 -> 312 B and code also shrinks by 44 B.

See
[implementation/R5e-age-deadline-overlay.md](implementation/R5e-age-deadline-overlay.md).

## R5f rejected result

```text
CI 40672791022  success

R5e 2202 B
R5f 2306 B
delta +104 B
```

The variable-length schedule stream saves 72 B of configuration state but adds
176 B of text. Reject the scan-based decoder.

See
[implementation/R5f-variable-schedule-stream.md](implementation/R5f-variable-schedule-stream.md).

## R5g rejected result

```text
CI 40673681141  success

R5e 2202 B
R5g 2218 B
delta +16 B
```

R5g saved 48 B of configuration state but its general 32-bit schedule
reconstruction path added 64 B of code.

See [implementation/R5g-schedule16.md](implementation/R5g-schedule16.md).

## R5h accepted result

```text
CI 40674018494  success

R5e 2202 B
R5h 1970 B
saved 232 B
```

R5h keeps the fixed 16-bit representation but specializes zero-overflow
generated configurations to direct halfword access.

```text
.text          1910 -> 1726 B
.arghtos_cfg    312 ->  264 B
```

See
[implementation/R5h-direct-schedule16.md](implementation/R5h-direct-schedule16.md).

## R5i accepted result

```text
CI 40718436840  success

R5h 1970 B
R5i 1914 B
saved  56 B
```

R5i replaces the fixed 16-bit schedule table with O(1) fixed 8-bit schedule
codes plus exact overflow fallback. The target fixture has zero overflow
entries.

```text
.arghtos_cfg          264 -> 240 B
process_time_locked   208 -> 188 B
kernel_dispatch       188 -> 180 B
arghtos_start         100 ->  96 B
```

See [implementation/R5i-schedule8.md](implementation/R5i-schedule8.md).

## R5j accepted result

```text
CI 40719129049  success

R5i 1914 B
R5j 1906 B
saved   8 B
```

R5j is an accounting-only requalification of the exact R5i linked image.
The deepest linked normal ArghtOS-owned MSP chain is now:

```text
SVC explicit push               8 B
kernel_service frame           16 B
process_time_locked frame      48 B
-----------------------------------
total                          72 B
```

See [implementation/R5j-msp-72.md](implementation/R5j-msp-72.md).

## Exact next action: R5k compact Cortex-M23 crash fill

Keep R5j as accepted control at 1906 B.

R5k should replace only the Cortex-M23 implementation of
`arghtos_port_crash_fill()` in the minimum-size target build.

Current C/LTO code copies a fixed target context into the required 64-byte
architecture-neutral crash record using scalar loads/stores.

R5k hypothesis:

* keep the portable kernel and C5 crash protocol unchanged;
* keep `arghtos_m23_fault_context_t` unchanged;
* implement the port payload copy as a leaf Thumb-1 assembly function;
* use paired `LDMIA/STMIA` transfers for MSP/PSP and hardware-frame words;
* allocate no MSP stack in the helper;
* keep NULL-context capture of live MSP/PSP;
* preserve exactly:
  * `port_format = 0x4D23`;
  * `port_words = 2, 3, or 10`;
  * state word ordering;
  * skipped hardware R12 slot;
  * generic zero-before-fill and commit-last protocol.

Acceptance:

1. Keep R5i linked code / R5j 72 B MSP accounting as control.
2. Gate the assembly helper behind an R5k-only port build define.
3. Exclude only the Cortex-M23 C crash-fill definition in R5k.
4. Run frozen host/sanitizer/portability gates.
5. Link R5k with 72 B MSP.
6. Inspect `arghtos_fatal` + crash-fill symbol sizes and disassembly.
7. Confirm the helper is leaf/no-stack and the 72 B MSP bound remains valid.
8. Accept only if total <1906 B.
9. Refresh this handoff.

