#include "arghtos_internal.h"

#include <stdbool.h>

#if !ARGHTOS_STATIC_CONFIG || !ARGHTOS_COMPACT_DESCRIPTOR || !ARGHTOS_UNIFIED_SLOT
#error "external async predicate is only for the static unified-slot profile"
#endif

extern arghtos_task_control_t arghtos_static_slots[ARGHTOS_MAX_TASKS];

bool arghtos_task_is_async_external(arghtos_task_id_t id)
{
    return (arghtos_static_slots[id].schedule & ARGHTOS_STATIC_ASYNC_TAG) != 0u;
}
