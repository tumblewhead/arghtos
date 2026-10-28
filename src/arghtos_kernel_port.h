#ifndef ARGHTOS_KERNEL_PORT_H
#define ARGHTOS_KERNEL_PORT_H

#include <arghtos/arghtos_port.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t arghtos_service_id_t;

enum {
    ARGHTOS_SERVICE_YIELD       = 0u,
    ARGHTOS_SERVICE_WAIT        = 1u,
    ARGHTOS_SERVICE_SLEEP_UNTIL = 2u,
    ARGHTOS_SERVICE_WAKE        = 3u,
    ARGHTOS_SERVICE_TASK_RETURN = 4u
};

arghtos_context_t arghtos_kernel_dispatch(arghtos_context_t outgoing_context);
arghtos_result_t arghtos_kernel_service(uint8_t service_id, uint32_t arg0);
void arghtos_timer_interrupt(void);
ARGHTOS_NORETURN void arghtos_fatal(
    arghtos_fault_reason_t reason,
    const void *port_fault_context);

#ifdef __cplusplus
}
#endif

#endif
