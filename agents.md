# ArghtOS agent instructions

This repository is the canonical nonvolatile memory for the ArghtOS project.

Do not reconstruct authority from chat history when repository evidence exists.

## Mandatory startup sequence

Every new agent working in this repository must:

1. Read this file.
2. Read [continue.md](continue.md) immediately afterward.
3. Fetch the branch and PR named in `continue.md`.
4. Read only the proof, architecture, contract, design, implementation, and test artifacts referenced by `continue.md` unless additional context is needed.
5. Confirm the repository/branch state before editing.
6. Continue from the exact next action in `continue.md` rather than restarting a completed phase.

If chat history conflicts with repository state, repository state wins unless the user explicitly changes the requirement.

## Mandatory CONTINUE discipline

`continue.md` is the live handoff.

The active agent must keep it fresh.

Update `continue.md` whenever any of these changes:

* active branch or PR;
* measured footprint;
* accepted/rejected experiment;
* requirements;
* proof/contract status;
* current risk;
* next planned action;
* CI evidence;
* a discovery changes what the next agent should do.

Before ending a substantial work session, `continue.md` must contain the **exact next step the next agent should take**.

Do not leave vague text such as "continue optimization."

Write the next action so another agent can execute it without asking the user to repeat context.

## Project purpose

ArghtOS is a very small statically allocated preemptive scheduler targeted first at Cortex-M23.

Core goals:

* protected periodic RMS scheduling;
* asynchronous event workers in residual capacity;
* no heap;
* bounded work;
* fixed/static task population;
* tickless timing;
* explicit wake/wait race handling;
* lightweight profiling and crash evidence;
* clean processor portability;
* complete resident-footprint accounting.

The implementation is proof-driven.

The engineering sequence is:

```text
requirements
  -> proofs
  -> architecture
  -> concrete contracts
  -> code-module design
  -> implementation
  -> measured reduction experiments
```

Do not bypass an upstream proof/contract when changing semantics.

## Current architectural rules that are easy to break

### Complete SRAM accounting

All ArghtOS-resident bytes count.

This includes:

* `.text`;
* `.rodata`;
* literal pools;
* jump tables;
* `.data`;
* `.bss`;
* `.noinit`;
* generated/static task metadata;
* crash record;
* ArghtOS-only MSP requirement;
* port code;
* assembly;
* code emitted by ArghtOS headers.

Never report only mutable SRAM.

Application task bodies and task PSP stacks are excluded from the ArghtOS-overhead subtotal but remain part of total MCU SRAM.

### Portability boundary

The portable kernel must compile unchanged for every target.

Portable sources must not contain target-specific branches or processor mechanisms.

Primary split:

```text
include/arghtos/arghtos.h       public portable API
include/arghtos/arghtos_port.h  architecture-neutral kernel -> port ABI
src/arghtos_internal.h          portable private state/types
src/arghtos_kernel_port.h       architecture-neutral port -> kernel callbacks
src/arghtos_kernel.c            portable C11 kernel

port/<target>/                   all nonportable implementation
```

For Cortex-M23, target-specific PRIMASK, PSP/MSP, SVC, PendSV, WFI, EXC_RETURN, CMSIS/vendor headers, timer realization, and crash-register extraction belong only under `port/cortex_m23/`.

A new port requiring edits to `arghtos_kernel.c` is evidence that the port ABI is incomplete. Extend the abstraction instead of adding target `#ifdef` blocks to portable code.

See [design-elements/D13-porting-guide.md](design-elements/D13-porting-guide.md).

### Task capacity and priorities

Current default capacity:

```text
ARGHTOS_MAX_TASKS = 24
```

Async priority representation:

```text
0x00       reserved / no async priority
0x01..MAX legal async priorities
0xFF       reserved / invalid
ARGHTOS_ASYNC_PRIORITY_MAX <= 254
```

### Periodic tasks

V1 periodic tasks are:

* synchronous;
* phase 0;
* relative deadline equals period;
* fixed RMS;
* shorter period = higher priority;
* equal period tie = smaller task ID;
* one active instance maximum;
* no wait/sleep self-suspension;
* return means job complete.

### Async tasks

Async tasks:

* are statically declared;
* are persistent workers;
* may wait/sleep/yield;
* use base priority plus opportunity-based age credit;
* cannot outrank READY periodic tasks;
* use one pending-wake bit only for race closure;
* do not store event multiplicity in the scheduler;
* returning from entry is fatal.

### Wake/wait race

Do not replace the pending-wake handshake with naive idempotent wake.

The required linearized behavior is:

```text
wake before wait commit:
    set pending bit
    wait consumes bit and does not block

wait commit before wake:
    RUNNING -> WAITING
    wake performs WAITING -> READY
```

Repeated wakes collapse deliberately. Driver/application state owns multiplicity.

### Time

Scheduler-visible time is modulo 2^32.

Legal relative horizons are less than half-range.

Periodic delayed release must use arithmetic catch-up, never a loop proportional to missed periods.

### Context switch

Only the selected port performs execution-context handoff.

On Cortex-M23:

* tasks use Thread mode / PSP;
* kernel and handlers use MSP;
* PendSV is the only task handoff point;
* SVC implements the abstract task service call;
* first-dispatch MSP frame reclamation includes xPSR alignment padding;
* idle uses the race-free masked WFI protocol.

### Crash logging

V1 has a required 64-byte architecture-neutral crash record.

Generic crash header/payload rules belong to the kernel contract.

Architecture register extraction belongs to the target port.

Do not make crash capture optional without explicit requirement/contract review.

## Proof and contract authority

Proofs live under:

```text
proof/
```

Architecture/contracts live under:

```text
architecture/
architecture/contracts/
```

Code-module designs live under:

```text
design-elements/
```

Important indexes:

* [proof/index.md](proof/index.md)
* [architecture/index.md](architecture/index.md)
* [architecture/contracts/index.md](architecture/contracts/index.md)
* [design-elements/index.md](design-elements/index.md)

The owning contract is authoritative if a summary disagrees with it.

## Proof ladder

Current proof set:

```text
P0 static configuration / resident-footprint boundedness
P1 task-state invariants
P2 context/stack integrity
P3 wake/wait race freedom
P4 time/release correctness
P5 priority-domain isolation
P6 RMS schedulability
P7 bounded resident footprint / scheduler work
P8 async starvation avoidance
```

Implementation evidence may discharge parts of these proofs, but do not call a proof fully discharged if target timing/WCET/hardware evidence is still missing.

## Implementation workflow

Use short measured experiments.

For each footprint experiment:

1. Preserve the previous profile as a comparison build when practical.
2. Make one conceptually isolated change.
3. Run host tests.
4. Run sanitizers.
5. Run portability checks.
6. Compile Cortex-M23.
7. Link the footprint harness.
8. Record map/symbol/MSP evidence.
9. Update the relevant implementation checkpoint.
10. Update `continue.md`.

Do not infer size savings from source inspection.

Only linked evidence counts.

## Linkable-skeleton rule

When starting a new implementation layer or target port, begin with a linkable skeleton before populating behavior.

This gives early evidence for:

* code structure;
* symbol boundaries;
* memory cost;
* linker surprises;
* portability leaks.

## Dangerous-code testing rule

Design documents contain expanded "dangerous code" sections.

Treat those examples as test requirements, not prose decoration.

Testers should deliberately probe:

* wait/wake interleavings;
* redundant READY wake;
* timer wraparound;
* many missed releases;
* equal-period RMS ties;
* async age saturation;
* nested serialization;
* register-canary preservation;
* EXC_RETURN handling;
* runtime stack alignment padding;
* idle WFI race;
* crash commit ordering;
* recursive faults;
* invalid dispatch combinations;
* periodic stack high-water stability.

See [design-elements/D11-test-seams.md](design-elements/D11-test-seams.md).

## Branch and merge discipline

Before editing:

* confirm the branch named in `continue.md`;
* confirm whether its prerequisite work is already merged to `main`;
* do not continue an obsolete/diverged experiment branch when a canonical merge exists;
* create a new branch from current canonical `main` for a new experiment phase.

Keep experimental PRs draft until the experiment is measured and documented.

Do not merge unless the user requests or clearly approves the merge.

## CI

The GitHub Actions workflow is:

```text
.github/workflows/ci.yml
```

The frozen R4 merge matrix intentionally keeps only supported/reference
surfaces:

* portable-source boundary;
* generic host kernel tests;
* accepted size-first selector host regression;
* profiling-enabled R3c reference;
* accepted minimum-source R4u host profile;
* ASan/UBSan;
* generic Cortex-M23 compile;
* R3c physical footprint reference;
* final R4y minimum footprint;
* R3c/R4y map and symbol evidence.

Historical R4 experiments are documented under `implementation/`; rejected
experiment profiles are not part of the supported Makefile/CI surface after
the R4 freeze.

When starting a new post-R4 experiment phase, branch from canonical `main`
after PR #3 is merged and add one live challenger beside the frozen R4y
control.

## R5 target-crossing guardrail

PR #4 has an accepted target-crossing control:

```text
R5ag complete resident footprint = 1534 B
soft target                      = 1536 B
margin                           =    2 B
```

Treat R5ag as immutable control for any post-target experiment. Add challengers
beside it; do not mutate its measured build in place.

The target depends on trusted generated-profile proofs, including generated
task ordering/class assumptions and the measured 23-periodic + 1-async
fixture. Do not generalize those assumptions to arbitrary runtime config.

Because the margin is only 2 B, any cleanup or toolchain/linker change must
re-link the Cortex-M23 control before claiming the target remains met.

See [continue.md](continue.md) and
[implementation/R5-target-crossing-1534.md](implementation/R5-target-crossing-1534.md).

## User preferences

The user values:

* proof-driven architecture;
* deterministic boundaries;
* compact context;
* precise Mermaid diagrams where useful;
* explicit dangerous-code/test guidance;
* repository commits over chat-only memory;
* machine-readable handoffs when requested;
* clean portability;
* linker/map evidence;
* iteration speed.

Do not ask the user to repeat information already in the repository or current conversation.

Prefer a best-effort implementation with measured evidence over speculative discussion.

## When context is getting full

Do not rely on chat compression alone.

Before context pressure becomes a problem:

1. refresh `continue.md`;
2. commit all relevant work;
3. ensure branch/PR/CI evidence is named;
4. state the exact next action;
5. let the next agent resume from repository state.
