#include <arghtos/arghtos.h>

static arghtos_time_t board_now;
static arghtos_time_t board_deadline;

arghtos_time_t arghtos_m23_board_time_now(void)
{
    return board_now;
}

void arghtos_m23_board_timer_arm(arghtos_time_t absolute_deadline)
{
    board_deadline = absolute_deadline;
}

void arghtos_m23_board_timer_disarm(void)
{
    board_deadline = 0u;
}

ARGHTOS_NORETURN void arghtos_m23_board_fault_terminal(void)
{
    for (;;) {
        __asm volatile("wfi");
    }
}
