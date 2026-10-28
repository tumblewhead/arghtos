# Host verification port

This port implements the same architecture-neutral `arghtos_port.h` ABI as hardware ports, but uses deterministic C state instead of processor exceptions.

It exists to test the real portable kernel, not a reimplemented scheduler model.

The host port provides controls for task/handler/startup context classification, scheduler clock, one-shot timer observations, interrupt-mask nesting state, dispatch requests, synthetic context tokens, and start/fatal `setjmp` escape hooks.

`arghtos_port_service_call()` directly invokes `arghtos_kernel_service()` on the host. Hardware ports may use a trap or software interrupt instead.
