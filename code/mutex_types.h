#ifndef MTX_TYPES_H
#define MTX_TYPES_H

#include "thread-worker_types.h"
#include <stdatomic.h>

/* mutex struct definition */
typedef struct worker_mutex_t
{
    // whether the mutex is locked
    atomic_flag is_locked;
    
} worker_mutex_t;

#endif
