// File:	thread-worker.c

// List all group member's name:
/* Nicholas Dundas npd59
 * Liam O'Neil ljo38
 */
// username of iLab: npd59
// iLab Server: ilab1.cs.rutgers.edu


#include "thread-worker.h"
#include "thread-worker_types.h"

#include <signal.h>
#include <sys/time.h>

#define STACK_SIZE 16 * 1024
#define QUANTUM 10 * 1000


// INITIALIZE ALL YOUR OTHER VARIABLES HERE
int init_scheduler_done = 0;

struct sigaction sa;
struct itimerval timer;
tcb main_thread;
worker_t threadnum = 0;

// _queue->prev points to back of queue but it is not circular as the last element->next points to NULL
tcb* run_queue = NULL;
tcb* ready_queue = NULL;
tcb* block_queue = NULL;
tcb* terminated_queue = NULL;
tcb* running = NULL;

tcb* back(tcb* queue) { //returns last element, returns NULL if none
    return queue ? queue->prev : NULL;
}

tcb* pop_front(tcb** queue) { //pops front element, returns NULL if none
    tcb* temp = *queue;
    if(back(*queue) == *queue) {
        *queue = NULL;
    } else {
        (*queue) = temp->next;
        (*queue)->prev = temp->prev;
        temp->prev = NULL;
        temp->next = NULL;
    }
    return temp;
}

tcb* emplace_back(tcb** queue, tcb* thread) { //pushing element to the back and returns second argument
    if(queue) {
        thread->next = NULL; 
        (*queue)->prev->next = thread;
        (*queue)->prev = thread;
    } else {
        *queue = thread;
        (*queue)->next = NULL;
        thread->prev = thread;
    }
    return thread;
}

tcb* remove(tcb** queue, tcb* thread) { //removes an element from the list and return its, returns NULL if none found
    tcb* cur = *queue;
    while(cur) {
        if(cur->id == thread->id) break;
        cur = cur->next;
    }
    if(cur) { 
        if(cur == *queue) { //1st element 
            *queue = cur->next;
            (*queue)->prev = cur->prev; 
        } else if (cur == back(*queue)) { //Last element
            (*queue)->prev = cur->prev;
            cur->prev->next = NULL;
        } else { //In the middle
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
        }
        cur->prev = NULL;
        cur->next = NULL;
    }
    return cur;
}

tcb* get_thread(worker_t id) { //returns tcb for the given id or NULL
    tcb* cur = run_queue;
    while(cur) {
        if(cur->id == id) return cur;
        cur = cur->next;
    }

    cur = ready_queue;
    while(cur) {
        if(cur->id == id) return cur;
        cur = cur->next;
    }
    
    cur = block_queue;
    while(cur) {
        if(cur->id == id) return cur;
        cur = cur->next;
    }
    if(cur->id == id) return cur;
        
    cur = terminated_queue;
    while(cur) {
        if(cur->id == id) return cur;
        cur = cur->next;
    }

    return NULL;
}

void worker_run(void *(*function)(void *), void *arg) {
    worker_exit(function(arg));
}

void init_workers() {
    memset (&sa, 0, sizeof(sa));
    sa.sa_handler = &schedule;
    sigaction (SIGPROF, &sa, NULL);
    timer.it_interval.tv_usec = QUANTUM; 
	timer.it_interval.tv_sec = 0;
    if(setitimer(ITIMER_PROF, &timer, NULL) == -1) {
        perror("Error setting timer during worker init\n");
        exit(EXIT_FAILURE);
    }
    main_thread.id = -1;
    if (getcontext(&main_thread.context) == -1) {
		printf("Error getting main thread context\n");
		exit(EXIT_FAILURE);
	}	
	running = &main_thread;
}

/* create a new thread */
int worker_create(worker_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg) {
    if(!threadnum) {
        init_workers();
    }
    tcb* new_thread = malloc(sizeof(tcb));
    new_thread->id = threadnum++;
    if (getcontext(&new_thread->context) == -1) {
		printf("Error getting worker thread context\n");
		exit(EXIT_FAILURE);
	}

    new_thread->context.uc_stack.ss_size = STACK_SIZE;
    new_thread->context.uc_stack.ss_sp = malloc(new_thread->context.uc_stack.ss_size);
    new_thread->context.uc_stack.ss_flags = 0;
    new_thread->context.uc_link = &main_thread.context;
    makecontext(&new_thread->context,(void (*)())worker_run,2,function,arg);

    emplace_back(&run_queue,new_thread);


    // - create Thread Control Block (TCB)
    // - create and initialize the context of this worker thread
    // - allocate space of stack for this thread to run
    // after everything is set, push this thread into run queue and
    // - make it ready for the execution.
    return 0;
}

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield()
{

    // - change worker thread's state from Running to Ready
    // - save context of this thread to its thread control block
    // - switch from thread context to scheduler context
    return 0;

};

/* terminate a thread */
void worker_exit(void *value_ptr)
{
    //pushing to the back of queue
    emplace_back(&terminated_queue,running);
    running->retval = value_ptr;
    // - if value_ptr is provided, save return value
    // - de-allocate any dynamic memory created when starting this thread (could be done here or elsewhere)
}

/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr)
{

    // - wait for a specific thread to terminate
    // - if value_ptr is provided, retrieve return value from joining thread
    // - de-allocate any dynamic memory created by the joining thread
    return 0;

};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex,
                      const pthread_mutexattr_t *mutexattr)
{
    //- initialize data structures for this mutex
    return 0;

};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex)
{

    // - use the built-in test-and-set atomic function to test the mutex
    // - if the mutex is acquired successfully, enter the critical section
    // - if acquiring mutex fails, push current thread into block list and
    // context switch to the scheduler thread
    return 0;

};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex)
{
    // - release mutex and make it available again.
    // - put one or more threads in block list to run queue
    // so that they could compete for mutex later.

    return 0;
};

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex)
{
    // - make sure mutex is not being used
    // - de-allocate dynamic memory created in worker_mutex_init

    return 0;
};

/* scheduler */
static void schedule()
{
// - every time a timer interrupt occurs, your worker thread library
// should be contexted switched from a thread context to this
// schedule() function

// - invoke scheduling algorithms according to the policy (RR or MLFQ)

// - schedule policy
#ifndef MLFQ
    // Choose RR
    
#else
    // Choose MLFQ
    
#endif
}

static void sched_rr()
{
    // - your own implementation of RR
    // (feel free to modify arguments and return types)

}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq()
{
    // - your own implementation of MLFQ
    // (feel free to modify arguments and return types)

}

// Feel free to add any other functions you need.
// You can also create separate files for helper functions, structures, etc.
// But make sure that the Makefile is updated to account for the same.