// File:	thread-worker.c

// List all group member's name:
/* Nicholas Dundas npd59
 * Liam O'Neill ljo38
 */
// username of iLab: npd59
// iLab Server: ilab1.cs.rutgers.edu

#ifndef WORKER_T_H
#define WORKER_T_H

#define _GNU_SOURCE

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "mutex_types.h"

/* Function Declarations: */

/* create a new thread */
int worker_create(worker_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg);

/* give CPU pocession to other user level worker threads voluntarily */
int worker_yield();

/* terminate a thread */
void worker_exit(void *value_ptr);

/* wait for thread termination */
int worker_join(worker_t thread, void **value_ptr);

/* initial the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t
												 *mutexattr);

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex);

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex);

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex);

#ifdef WORKER_DEBUG
#include <signal.h>
#include <sys/time.h>
#include <string.h>

size_t q_size(const tcb* queue)
tcb* q_find_elem(tcb* queue, worker_t id);
tcb* q_back(const tcb* queue);
tcb* q_pop_front(tcb** queue);
tcb* q_emplace_back(tcb** queue, tcb* thread);
tcb* q_emplace_front(tcb** queue, tcb* thread);
tcb* q_remove_elem(tcb** queue, tcb* thread);
tcb* get_thread(worker_t id);
worker_t get_unique_id();

#endif

#endif
