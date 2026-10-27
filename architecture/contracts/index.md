# Architecture contracts

* [C0 - Static task configuration](C0-static-task-configuration.md)
* [C1 - Task API and lifecycle](C1-task-api-lifecycle.md)
* [C2 - ISR wake contract](C2-isr-wake.md)
* [C3 - Time and timer contract](C3-time-timer.md)
* [C4 - Context-switch port contract](C4-context-switch-port.md)
* [C5 - Fault and diagnostics contract](C5-fault-diagnostics.md)
* [C6 - Scheduling policy contract](C6-scheduling-policy.md)

```mermaid
flowchart LR
  C0["C0 Config"] --> C6["C6 Policy"]
  C1["C1 Task API"] --> C6
  C2["C2 ISR wake"] --> C6
  C3["C3 Time"] --> C6
  C6 --> C4["C4 Context port"]
  C1 --> C5["C5 Fault/diagnostics"]
  C2 --> C5
  C3 --> C5
  C4 --> C5
```
