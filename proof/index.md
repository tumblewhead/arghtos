# Scheduler proofs

* [P0 - Static configuration and resident-footprint boundedness](P0-static-configuration-validity.md) - Proves that the configured scheduler instance is finite, statically allocated, and internally well-formed.
* [P1 - Task-state invariants](P1-task-state-invariants.md) - Proves legal scheduler state transitions and single-running-task behavior.
* [P2 - Context and stack integrity](P2-context-stack-integrity.md) - Proves MSP/PSP ownership and task-stack isolation assumptions.
* [P3 - Wake/wait race freedom](P3-wake-wait-race-freedom.md) - Proves idempotent asynchronous wake behavior and prevents lost wakeups at the wait boundary.
* [P4 - Time and release correctness](P4-time-release-correctness.md) - Proves tickless absolute-time ordering, wrap-safe comparisons, and release behavior.
* [P5 - Priority-domain isolation](P5-priority-domain-isolation.md) - Proves asynchronous aging cannot enter the protected RMS priority domain.
* [P6 - RMS schedulability](P6-rms-schedulability.md) - Defines the response-time proof required for all compile-time periodic tasks.
* [P7 - Bounded resident footprint and scheduler work](P7-bounded-memory-and-work.md) - Proves bounded SRAM use, bounded critical sections, and finite scheduler work.
* [P8 - Async starvation avoidance](P8-async-starvation-avoidance.md) - Proves eventual service for continuously READY async tasks when async dispatch opportunities continue.
