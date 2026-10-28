#ifndef ARGHTOS_ARGHTOS_H
#define ARGHTOS_ARGHTOS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#define ARGHTOS_NORETURN [[noreturn]]
#else
#define ARGHTOS_NORETURN _Noreturn
#endif

#ifndef ARGHTOS_MAX_TASKS
#define ARGHTOS_MAX_TASKS 24u
#endif

#ifndef ARGHTOS_ASYNC_PRIORITY_MAX
#define ARGHTOS_ASYNC_PRIORITY_MAX 8u
#endif

#if ARGHTOS_MAX_TASKS < 1u || ARGHTOS_MAX_TASKS > 254u
#error "ARGHTOS_MAX_TASKS must be in [1,254]"
#endif

#if ARGHTOS_ASYNC_PRIORITY_MAX < 1u || ARGHTOS_ASYNC_PRIORITY_MAX > 254u
#error "ARGHTOS_ASYNC_PRIORITY_MAX must be in [1,254]"
#endif

typedef uint8_t  arghtos_task_id_t;
typedef uint8_t  arghtos_priority_t;
typedef uint32_t arghtos_time_t;
typedef uint32_t arghtos_duration_t;
typedef uint8_t  arghtos_result_t;
typedef uint8_t  arghtos_task_state_t;
typedef uint16_t arghtos_fault_reason_t;
typedef void (*arghtos_task_fn_t)(void);

#define ARGHTOS_TASK_ID_INVALID ((arghtos_task_id_t)UINT8_MAX)
#define ARGHTOS_PRIORITY_NONE    ((arghtos_priority_t)0u)
#define ARGHTOS_PRIORITY_INVALID ((arghtos_priority_t)UINT8_MAX)
#define ARGHTOS_TIME_HALF_RANGE ((arghtos_duration_t)0x80000000u)

enum {
    ARGHTOS_OK                 = 0u,
    ARGHTOS_ERR_BAD_ARGUMENT   = 1u,
    ARGHTOS_ERR_BAD_TASK_ID    = 2u,
    ARGHTOS_ERR_BAD_TASK_CLASS = 3u,
    ARGHTOS_ERR_BAD_STATE      = 4u,
    ARGHTOS_ERR_BAD_CONTEXT    = 5u,
    ARGHTOS_ERR_BAD_TIME       = 6u,
    ARGHTOS_ERR_BAD_CONFIG     = 7u
};

enum {
    ARGHTOS_TASK_DORMANT  = 0u,
    ARGHTOS_TASK_READY    = 1u,
    ARGHTOS_TASK_RUNNING  = 2u,
    ARGHTOS_TASK_WAITING  = 3u,
    ARGHTOS_TASK_SLEEPING = 4u,
    ARGHTOS_TASK_FAULTED  = 5u
};

enum {
    ARGHTOS_FAULT_NONE             = 0x0000u,
    ARGHTOS_FAULT_KERNEL_INVARIANT = 0x0001u,
    ARGHTOS_FAULT_BAD_SERVICE      = 0x0002u,
    ARGHTOS_FAULT_CONTEXT_CORRUPT  = 0x0003u,
    ARGHTOS_FAULT_ASYNC_RETURNED   = 0x0004u,
    ARGHTOS_FAULT_STACK_RANGE      = 0x0005u,
    ARGHTOS_FAULT_BAD_DISPATCH     = 0x0006u,
    ARGHTOS_FAULT_CPU_EXCEPTION    = 0x0100u
};

typedef struct {
    arghtos_task_fn_t entry;
    void             *stack_base;
    arghtos_time_t    period;
    uint16_t          stack_size;
    uint8_t           async_priority;
    uint8_t           flags;
} arghtos_task_config_t;

#define ARGHTOS_CRASH_MAGIC        0x41524748u
#define ARGHTOS_CRASH_COMMIT_MAGIC 0x43524153u
#define ARGHTOS_CRASH_VERSION      1u

#define ARGHTOS_CRASH_F_TASK_VALID       (1u << 0)
#define ARGHTOS_CRASH_F_PORT_STATE_VALID (1u << 1)
#define ARGHTOS_CRASH_F_RESERVED         (0xFCu)

typedef struct arghtos_crash_record {
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    uint16_t reason;
    uint8_t  task_id;
    uint8_t  flags;
    uint32_t timestamp;
    uint16_t port_format;
    uint8_t  port_words;
    uint8_t  reserved;
    uint32_t port_state[10];
    uint32_t commit_magic;
} arghtos_crash_record_t;

typedef struct {
    const arghtos_task_config_t *tasks;
    arghtos_crash_record_t      *crash_record;
    uint8_t                      task_count;
    uint8_t                      reserved[3];
} arghtos_config_t;

typedef struct {
    uint32_t total_cpu_ticks;
    uint32_t max_run_ticks;
    uint8_t  missed_release_count;
    uint8_t  age_credit;
    uint8_t  state;
    uint8_t  flags;
} arghtos_task_profile_t;

arghtos_result_t arghtos_init(const arghtos_config_t *config);
ARGHTOS_NORETURN void arghtos_start(void);

arghtos_result_t arghtos_yield(void);
arghtos_result_t arghtos_wait(void);
arghtos_result_t arghtos_sleep_until(arghtos_time_t absolute_time);
arghtos_result_t arghtos_sleep_for(arghtos_duration_t duration);
arghtos_result_t arghtos_wake(arghtos_task_id_t task_id);
arghtos_result_t arghtos_wake_from_interrupt(arghtos_task_id_t task_id);
arghtos_result_t arghtos_profile_get(
    arghtos_task_id_t task_id,
    arghtos_task_profile_t *out);

#ifdef __cplusplus
}
#endif

#endif
