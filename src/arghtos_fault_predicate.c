#include <arghtos/arghtos.h>

#include <stdbool.h>

bool arghtos_task_fault_attributed(arghtos_fault_reason_t reason)
{
    return (reason == ARGHTOS_FAULT_ASYNC_RETURNED) ||
           (reason == ARGHTOS_FAULT_STACK_RANGE) ||
           (reason == ARGHTOS_FAULT_CONTEXT_CORRUPT) ||
           (reason == ARGHTOS_FAULT_CPU_EXCEPTION);
}
