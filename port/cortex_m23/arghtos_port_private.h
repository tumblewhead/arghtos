#ifndef ARGHTOS_CORTEX_M23_PORT_PRIVATE_H
#define ARGHTOS_CORTEX_M23_PORT_PRIVATE_H

#include <arghtos/arghtos_port.h>
#include "../../src/arghtos_kernel_port.h"

#define ARGHTOS_M23_SCB_ICSR_ADDR        ((uintptr_t)0xE000ED04u)
#define ARGHTOS_M23_SHPR3_PENDSV_ADDR    ((uintptr_t)0xE000ED22u)
#define ARGHTOS_M23_ICSR_PENDSVSET       (1u << 28)
#define ARGHTOS_M23_CONTROL_SPSEL        (1u << 1)
#define ARGHTOS_M23_XPSR_T               (1u << 24)
#define ARGHTOS_M23_MIN_CONTEXT_RESERVE  68u
#define ARGHTOS_M23_CRASH_FORMAT         0x4D23u

typedef struct {
    uint32_t msp;
    uint32_t psp;
    uint32_t exc_return;
    const uint32_t *basic_frame;
} arghtos_m23_fault_context_t;

arghtos_time_t arghtos_m23_board_time_now(void);
void arghtos_m23_board_timer_arm(arghtos_time_t absolute_deadline);
void arghtos_m23_board_timer_disarm(void);
ARGHTOS_NORETURN void arghtos_m23_board_fault_terminal(void);

#endif
