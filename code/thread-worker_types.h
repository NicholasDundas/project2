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

    //return value for threads
    void* retval;

    int priority; //debug

    //queue related structures, unused for normal TCBs
    size_t queue_size;
    struct TCB *next, *prev;
} tcb;

#endif
