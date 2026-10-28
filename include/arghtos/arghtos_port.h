#ifndef ARGHTOS_ARGHTOS_PORT_H
#define ARGHTOS_ARGHTOS_PORT_H

#include <arghtos/arghtos.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uintptr_t arghtos_context_t;
typedef uintptr_t arghtos_irq_key_t;

#define ARGHTOS_CONTEXT_NONE ((arghtos_context_t)0)

arghtos_result_t arghtos_port_init(void);
bool arghtos_port_stack_valid(void *stack_base, uint32_t stack_size);

arghtos_time_t arghtos_port_time_now(void);
void arghtos_port_timer_arm(arghtos_time_t absolute_deadline);
void arghtos_port_timer_disarm(void);

arghtos_irq_key_t arghtos_port_critical_enter(void);
void arghtos_port_critical_exit(arghtos_irq_key_t key);

bool arghtos_port_is_task_context(void);
bool arghtos_port_is_handler_context(void);

arghtos_result_t arghtos_port_service_call(
    uint8_t service_id,
    uint32_t arg0);

void arghtos_port_request_dispatch(void);
ARGHTOS_NORETURN void arghtos_port_start(void);
void arghtos_port_idle_wait(void);

arghtos_context_t arghtos_port_build_initial_context(
    void *stack_base,
    uint32_t stack_size,
    void (*thread_start)(void),
    void (*return_guard)(void));

void arghtos_port_crash_fill(
    arghtos_crash_record_t *record,
    const void *port_fault_context);

ARGHTOS_NORETURN void arghtos_port_fault_terminal(void);

#ifdef __cplusplus
}
#endif

#endif
