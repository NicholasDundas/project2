// File:	thread-worker.c

// List all group member's name:
/* Nicholas Dundas npd59
 * Liam O'Neil ljo38
 */
// username of iLab: npd59
// iLab Server: ilab1.cs.rutgers.edu


#include "thread-worker.h"
#include "thread-worker_types.h"



#define STACK_SIZE 16 * 1024
#define QUANTUM 10 * 1000


#include <signal.h>
#include <sys/time.h>
#include <string.h>

// INITIALIZE ALL YOUR OTHER VARIABLES HERE
int initialized_threads = 0; //keeping track of whether we need to initialize variables
struct sigaction sa; //sigaction for calling schedule() during time interupt
struct itimerval timer; //used for timer interrupts for schedule()
tcb main_thread; //the main executing thread
int totalthread = 0; //total count of threads

// ####_queue->prev points to back of queue but it is not circular as the last element->next points to NULL

tcb* q_ready = NULL; //threads waiting to be run
tcb* q_block = NULL; //threads currently blocking
tcb* q_terminated = NULL; //threads finished executing

tcb* running = NULL; //current running thread


static void schedule();
static void sched_rr();

//returns size of queue
size_t q_size(const tcb* queue) {
    return queue ? queue->queue_size : 0;
}

//returns back of queue
tcb* q_back(const tcb* queue) { 
    return queue ? queue->prev : NULL;
}

//pops front element from queue and returns it, returns NULL if none
tcb* q_pop_front(tcb** queue) { 
    tcb* temp = *queue;
    if(q_back(*queue) == *queue) {
        *queue = NULL;
    } else {
        temp->next->queue_size = (*queue)->queue_size - 1;
        (*queue) = temp->next;
        (*queue)->prev = temp->prev;
        temp->prev = NULL;
        temp->next = NULL;
    }
    return temp;
}

//pushing element to the back and returns second argument
tcb* q_emplace_back(tcb** queue, tcb* thread) { 
    if(*queue) {
        thread->next = NULL; 
        thread->prev = q_back(*queue);
        (*queue)->prev->next = thread;
        (*queue)->prev = thread;
        (*queue)->queue_size++;
    } else { //length 0 condition
        *queue = thread;
        (*queue)->next = NULL;
        (*queue)->queue_size = 1;
        thread->prev = thread;
    }
    return thread;
}

//finds a element in a specific queue, returns NULL if none
tcb* q_find_elem(const tcb* queue, worker_t id) { 
    tcb* cur = queue;
    while(cur) {
        if(cur->id == id) break;
        cur = cur->next;
    } 
    return cur;
}

//removes an element from the list and return seconds argument, returns NULL if none found
tcb* q_remove_elem(tcb** queue, tcb* thread) { 
    tcb* cur = q_find_elem(*queue,thread->id);
    if(cur) { 
        if(cur == *queue) { //1st element 
            cur = q_pop_front(queue);
        } else if (cur == q_back(*queue)) { //Last element
            (*queue)->queue_size--;
            (*queue)->prev = cur->prev;
            (*queue)->prev->next = NULL;
        } else { //In the middle
            (*queue)->queue_size--;
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
        }
        cur->prev = NULL;
        cur->next = NULL;
    }
    return cur;
}

//returns tcb for the given id or NULL if none found
tcb* get_thread(worker_t id) { 
    if(id == running->id)
        return running;

    tcb* cur = q_ready;
    while(cur) {
        if(cur->id == id) return cur;
        cur = cur->next;
    }
    
    cur = q_block;
    while(cur) {
        if(cur->id == id) return cur;
        cur = cur->next;
    }
        
    cur = q_terminated;
    while(cur) {
        if(cur->id == id) return cur;
        cur = cur->next;
    }

    return NULL;
}

//used to get the next lowest available id
worker_t get_unique_id() { 
    int *used = calloc(q_size(q_ready) + q_size(q_block) + q_size(q_terminated) 
                        + 1 /*tcb* running*/ + 1 /*for potentially no ids*/,sizeof(int));
    if(!used) {
        perror("Could not run function get_unique_id()");
        exit(EXIT_FAILURE);
    }
    if(running)
        used[running->id] = 1;
    tcb* cur = q_ready;
    while(cur) {
        used[cur->id] = 1;
        cur = cur->next;
    }
    
    cur = q_block;
    while(cur) {
        used[cur->id] = 1;
        cur = cur->next;
    }
        
    cur = q_terminated;
    while(cur) {
        used[cur->id] = 1;
        cur = cur->next;
    }
    worker_t new_id = 0;
    while(used[new_id]) {
        new_id++;
    }
    free(used);
    return new_id;
}

void worker_run(void *(*function)(void *), void *arg) {
    worker_exit(function(arg));
}

void init_workers() {
    memset (&sa, 0, sizeof(sa));
    sa.sa_handler = schedule;
    sigaction (SIGPROF, &sa, NULL);

    timer.it_interval.tv_usec = QUANTUM; 
	timer.it_interval.tv_sec = 0;
    if(setitimer(ITIMER_PROF, &timer, NULL) == -1) {
        perror("Error setting timer during worker init\n");
        exit(EXIT_FAILURE);
    }

    main_thread.id = 0;
    makecontext(&main_thread.context, (void *)&schedule, 0);
	running = &main_thread;
    main_thread.next = NULL;
    main_thread.prev = NULL;
}

/* create a new thread */
int worker_create(worker_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg) {
    if(!initialized_threads) {
        initialized_threads++;
        init_workers();
    }
    tcb* new_thread = malloc(sizeof(tcb));
    totalthread++;
    new_thread->id = get_unique_id();
    if (getcontext(&new_thread->context) == -1) {
		perror("Error getting worker thread context\n");
		exit(EXIT_FAILURE);
	}

    new_thread->context.uc_stack.ss_size = STACK_SIZE;
    new_thread->context.uc_stack.ss_sp = malloc(new_thread->context.uc_stack.ss_size);
    new_thread->context.uc_stack.ss_flags = 0;
    new_thread->context.uc_link = &main_thread.context;
    makecontext(&new_thread->context,(void *)&worker_run,2,function,arg);

    q_emplace_back(&q_ready,new_thread);


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
    q_emplace_back(&q_terminated,running);
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
static void schedule() {
// - every time a timer interrupt occurs, your worker thread library
// should be contexted switched from a thread context to this
// schedule() function

// - invoke scheduling algorithms according to the policy (RR or MLFQ)

// - schedule policy
setitimer(ITIMER_PROF, NULL, NULL);

#ifndef MLFQ
    // Choose RR
    sched_rr();
#else
    // Choose MLFQ
    
#endif
}

static void sched_rr() {
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