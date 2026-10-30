#include <arghtos/arghtos.h>

#ifndef ARGHTOS_FOOTPRINT_LEGACY_CONSTANT_BOARD
#define ARGHTOS_FOOTPRINT_LEGACY_CONSTANT_BOARD 0
#endif

#if ARGHTOS_FOOTPRINT_LEGACY_CONSTANT_BOARD
/*
 * Historical footprint harness used by the frozen R3/R4/R5/R6a-q evidence.
 * These ordinary static variables permit LTO to constant-fold the synthetic
 * clock/timer behavior. Keep this mode only for exact reproduction of those
 * historical measurements.
 */
static arghtos_time_t board_now;
static arghtos_time_t board_deadline;
#else
/*
 * Physical-board footprint harness.
 *
 * Model hardware/application-owned timer state as externally mutable.
 * Volatile prevents LTO from proving the clock is permanently zero or timer
 * writes unobservable. The .app_data placement excludes this synthetic board
 * state from ArghtOS-resident accounting, matching real MMIO ownership.
 */
__attribute__((section(".app_data")))
static volatile arghtos_time_t board_now;
__attribute__((section(".app_data")))
static volatile arghtos_time_t board_deadline;
#endif

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
