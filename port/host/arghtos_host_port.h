#ifndef ARGHTOS_HOST_PORT_H
#define ARGHTOS_HOST_PORT_H

#include <arghtos/arghtos.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    ARGHTOS_HOST_STARTUP = 0,
    ARGHTOS_HOST_TASK,
    ARGHTOS_HOST_HANDLER
} arghtos_host_mode_t;

void arghtos_host_reset(void);
void arghtos_host_set_mode(arghtos_host_mode_t mode);
arghtos_host_mode_t arghtos_host_mode(void);
void arghtos_host_set_time(arghtos_time_t now);
void arghtos_host_advance_time(arghtos_duration_t dt);
bool arghtos_host_timer_is_armed(void);
arghtos_time_t arghtos_host_timer_deadline(void);
uint32_t arghtos_host_timer_arm_count(void);
uint32_t arghtos_host_timer_disarm_count(void);
uint32_t arghtos_host_dispatch_request_count(void);
void arghtos_host_clear_dispatch_requests(void);
bool arghtos_host_interrupts_masked(void);
uint32_t arghtos_host_idle_count(void);
void arghtos_host_set_start_jmp(jmp_buf *env);
void arghtos_host_set_fault_jmp(jmp_buf *env);
void arghtos_host_set_recursive_fault_on_crash_fill(bool enable);
uint32_t arghtos_host_crash_fill_count(void);
#if ARGHTOS_ENTRY_NEXT_OVERLAY
uintptr_t arghtos_host_initial_arg(void);
#endif

#endif
