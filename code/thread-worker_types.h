#ifndef TW_TYPES_H
#define TW_TYPES_H

#include <ucontext.h>

typedef unsigned int worker_t;

typedef enum { NEW, RUNNING, WAITING, READY, TERMINATED } THREAD_STATUS;

typedef struct TCB {
    // thread Id
    worker_t id;
    // thread status
    THREAD_STATUS status;
    // thread context
    //     contains thread stack, stack size, and stack flags
    //     contains register info and other information
    //     contains signal mask for any blocked signals
    ucontext_t context;

    // thread priority
    int priority;

} tcb;

#endif
