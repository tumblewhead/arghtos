# Code blocks

* [B0 - Configuration](B0-configuration.md)
* [B1 - Runtime state](B1-runtime-state.md)
* [B2 - Scheduling policy](B2-scheduling-policy.md)
* [B3 - Time and release](B3-time-release.md)
* [B4 - Cortex-M23 context port](B4-context-port.md)
* [B5 - Diagnostics and fault capture](B5-diagnostics.md)
* [B6 - API and syscall gateway](B6-api-gateway.md)

```mermaid
flowchart LR
  B0["B0 Config"] --> B1["B1 State"]
  B0 --> B2["B2 Policy"]
  B1 <--> B2
  B3["B3 Time"] --> B1
  B6["B6 API/SVC"] --> B1
  B4["B4 Context/PendSV"] <--> B2
  B4 --> B1
  B1 --> B5["B5 Diagnostics"]
  B4 --> B5
```
