// File:	thread-worker.c

// List all group member's name:
/* Nicholas Dundas npd59
 * Liam O'Neill ljo38
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
#include <stdbool.h>

static void schedule(bool worker_exit_called);
static void sched_rr(bool worker_exit_called);

// INITIALIZE ALL YOUR OTHER VARIABLES HERE
int initialized_threads = 0; //keeping track of whether we need to initialize variables
struct sigaction sa; //sigaction for calling schedule() during time interupt
struct itimerval timer; //used for timer interrupts for schedule()
tcb main_thread; //the main executing thread
ucontext_t schedule_context; //stores the scheduler context so any exiting threads call upon the scheduler

//QUEUES BELOW
// ####_queue->prev points to back of queue but it is not circular as the last element->next points to NULL

tcb* q_ready = NULL; //threads waiting to be run
tcb* q_block = NULL; //threads currently blocking
tcb* q_terminated = NULL; //threads finished executing

tcb* running = NULL; //current running thread

static int timer_stop() {
    return setitimer(ITIMER_PROF, NULL, NULL);
}

static int timer_start(struct itimerval* timer) {
    return setitimer(ITIMER_PROF, timer, NULL);
}

static int timer_reset(struct itimerval* timer) {
    int res = timer_stop();
    timer->it_value.tv_usec = timer->it_interval.tv_usec; 
	timer->it_value.tv_sec = timer->it_interval.tv_sec;
    if(res != 0) return res;
    return timer_start(timer);
    
}

//returns size of queue
static size_t q_size(const tcb* queue) {
    return queue ? queue->queue_size : 0;
}

static void q_init(tcb** queue) {
    *queue = NULL;
}

//returns back of queue
static tcb* q_back(const tcb* queue) { 
    return queue ? queue->prev : NULL;
}

//pops front element from queue and returns it, returns NULL if none
static tcb* q_pop_front(tcb** queue) { 
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

//pushes thread to the back and returns second argument
static tcb* q_emplace_back(tcb** queue, tcb* thread) { 
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
static tcb* q_find_elem(tcb* queue, worker_t id) { 
    tcb* cur = queue;
    while(cur) {
        if(cur->id == id) break;
        cur = cur->next;
    } 
    return cur;
}

//removes an element from the list and return seconds argument, returns NULL if none found
//if thread is NULL then the returned value is null and Nop is preformed
static tcb* q_remove_elem(tcb** queue, tcb* thread) { 
    tcb* cur = (thread == NULL) ? NULL : q_find_elem(*queue,thread->id);
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
static tcb* get_thread(worker_t id) { 
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
static worker_t get_unique_id() { 
    int *used = calloc(q_size(q_ready) + q_size(q_block) + q_size(q_terminated) 
                            + 1 /*tcb* running*/ + 1 /*for potentially no ids*/,sizeof(int));
    if(!used) {
        perror("Could not run function get_unique_id() malloc error");
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

static void worker_run(void *(*function)(void *), void *arg) {
    worker_exit(function(arg));
}

static void sig_handle(int signum) {
    switch(signum) {
        case SIGPROF: //timer
            schedule(false);
    }
}

static void init_workers() {
    memset (&sa, 0, sizeof(sa));
    sa.sa_handler = &sig_handle;
    sigaction (SIGPROF, &sa, NULL);

	timer.it_interval.tv_usec = QUANTUM; 
	timer.it_interval.tv_sec = 0;
    timer.it_value.tv_usec = QUANTUM; 
	timer.it_value.tv_sec = 0;

    if(timer_start(&timer) == -1) {
        perror("Error starting timer during worker init\n");
        exit(EXIT_FAILURE);
    }

    main_thread.id = 0;
    if (getcontext(&main_thread.context) == -1) {
		printf("Error getting scheduler thread context\n");
		exit(EXIT_FAILURE);
	}
    main_thread.next = NULL;
    main_thread.prev = NULL;
	running = &main_thread;
}

/* create a new thread */
int worker_create(worker_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg) {
    if(!initialized_threads) {
        initialized_threads++;
        init_workers();
    }

    if(timer_stop(&timer) == -1) {
        perror("Error stopping timer during worker create\n");
        exit(EXIT_FAILURE);
    }

    tcb* new_thread = malloc(sizeof(tcb));
    memset(new_thread,0,sizeof(tcb));
    *thread = (new_thread->id = get_unique_id());
    if (getcontext(&new_thread->context) == -1) {
		perror("Error getting worker thread context\n");
		exit(EXIT_FAILURE);
	}

    new_thread->context.uc_stack.ss_size = STACK_SIZE;
    new_thread->context.uc_stack.ss_sp = malloc(new_thread->context.uc_stack.ss_size);
    new_thread->context.uc_stack.ss_flags = 0;
    new_thread->context.uc_link = &schedule_context;
    makecontext(&new_thread->context,(void (*)())&worker_run,2,function,arg);

    q_emplace_back(&q_ready,new_thread);

    if(timer_start(&timer) == -1) {
        perror("Error starting timer during worker create\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
    if(timer_reset(&timer) == -1) {
        perror("Error resetting timer during worker yield\n");
        exit(EXIT_FAILURE);
    }
    schedule(false);
    return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
    if(timer_stop(&timer) == -1) {
        perror("Error stopping timer during worker exit\n");
        exit(EXIT_FAILURE);
    }
    q_emplace_back(&q_terminated,running);
    running->retval = value_ptr;
    schedule(true);
}

/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
    while(1) {
        if(timer_stop(&timer) == -1) {
            perror("Error stopping timer during worker join\n");
            exit(EXIT_FAILURE);
        }
        tcb* find = q_remove_elem(&q_terminated,get_thread(thread));
        if(find) {
            if(value_ptr) *value_ptr = find->retval;
            free(find);
            return 0;
        }
        worker_yield();
    }
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex,
                      const pthread_mutexattr_t *mutexattr) {
    //- initialize data structures for this mutex
    atomic_flag_clear(&mutex->is_locked);
    return 0;

};

/* acquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {
    // - use the built-in test-and-set atomic function to test the mutex
    // - if the mutex is acquired successfully, enter the critical section
    // - if acquiring mutex fails, push current thread into block list and
    // context switch to the scheduler thread
    while (atomic_flag_test_and_set(&mutex->is_locked)) {
        if(initialized_threads) {
            if(timer_stop() == -1) {
                perror("Error stopping timer during lock\n");
                exit(EXIT_FAILURE);
            }
            q_emplace_back(&q_block,running);
            if(timer_reset(&timer) == -1) {
                perror("Error resetting timer during worker yield in mutex lock\n");
                exit(EXIT_FAILURE);
            }
            schedule(true);
        }
    }
    return 1;

};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
    if(initialized_threads) {
        if(timer_stop() == -1) {
            perror("Error stopping timer during unlock\n");
            exit(EXIT_FAILURE);
        }
    }
    atomic_flag_clear(&mutex->is_locked);
    if(initialized_threads) {
        // - release mutex and make it available again.
        // - put one or more threads in block list to run queue
        // so that they could compete for mutex later.
        while (q_block) {
            q_emplace_back(&q_ready, q_pop_front(&q_block));
        }
        schedule(false);
    }
    return 0;
};

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
    // - make sure mutex is not being used
    worker_mutex_lock(mutex);
    // - de-allocate dynamic memory created in worker_mutex_init
    return 0;
};

/* scheduler */
static void schedule(bool emplaced) {
    // - every time a timer interrupt occurs, your worker thread library
    // should be contexted switched from a thread context to this
    // schedule() function

    // - invoke scheduling algorithms according to the policy (RR or MLFQ)

    // - schedule policy
    if (getcontext(&schedule_context) == -1 ) {
		printf("Error saving scheduler context\n");
		exit(EXIT_FAILURE);
	}
    if(q_ready) {
        if(timer_stop() == -1) {
            perror("Error stopping timer during sched_rr\n");
            exit(EXIT_FAILURE);
        }
#ifndef MLFQ
    // Choose RR
    sched_rr(emplaced);
#else
    // Choose MLFQ
    
#endif
    } else {
        if(timer_reset(&timer) == -1) {
            perror("Error resetting timer during sched_rr\n");
            exit(EXIT_FAILURE);
        }
    }
}

static void sched_rr(bool emplaced) {
    tcb* last = running;
    
    running = q_pop_front(&q_ready);
    if(!running) { 
        /*running is NULL implying no threads left so we can finally exit*/
        exit(EXIT_SUCCESS);
    }

    if(!emplaced) {
        q_emplace_back(&q_ready,last);
    } //we put it in q_terminated already from worker_exit

    if(timer_start(&timer) == -1) {
        perror("Error starting timer during sched_rr\n");
        exit(EXIT_FAILURE);
    }
    if (swapcontext(&last->context, &running->context) == -1) {
        perror("Error swapping context to next thread in RR schedule\n"); 
        exit(EXIT_FAILURE);
    }  
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