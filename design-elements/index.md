# ArghtOS code-module design elements

This directory turns the audited contracts into concrete implementation modules. The design remains implementation-oriented but stops before writing production code.

* [D00 - Module map and physical source layout](D00-module-map.md)
* [D01 - Public API and ABI surface](D01-public-api.md)
* [D02 - Kernel state, configuration, and startup](D02-kernel-state-config-start.md)
* [D03 - Lifecycle, wake, and syscall gateway](D03-lifecycle-wake-gateway.md)
* [D04 - Scheduling policy and async aging](D04-scheduler-policy.md)
* [D05 - Time, sleep, and periodic release engine](D05-time-release.md)
* [D06 - Cortex-M23 execution port and context switch](D06-cortex-m23-port.md)
* [D07 - Runtime profiling, crash capture, and fatal path](D07-diagnostics-fault.md)
* [D08 - Task trampoline and periodic job restart](D08-task-trampoline.md)
* [D09 - Integrated kernel translation unit](D09-kernel-integration.md)
* [D10 - Build, linker, and resident-footprint enforcement](D10-build-footprint.md)
* [D11 - Test seams and proof-to-test map](D11-test-seams.md)

All design elements depend on the audited contract baseline in [../architecture/contracts/](../architecture/contracts/) and the gate in [../architecture/A1-consistency-audit.md](../architecture/A1-consistency-audit.md).

The **dangerous code** section in each element is normative for verification planning: testers should use the listed failure examples as deliberate fault-injection or adversarial test cases where practical.
* [D12 - Code-module design consistency audit](D12-design-consistency-audit.md) - Design freeze/gate before implementation.
