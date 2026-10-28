#include <arghtos/arghtos.h>

#include <stdbool.h>
#include <stdint.h>

bool arghtos_time_due_external(arghtos_time_t now, arghtos_time_t t)
{
    return (int32_t)(now - t) >= 0;
}
