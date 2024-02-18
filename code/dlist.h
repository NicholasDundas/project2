#ifndef DLIST_H
#define DLIST_H

#define _GNU_SOURCE

#include <stddef.h>

#include "thread-worker_types.h"

typedef struct dlist_node {
    tcb* data;
    struct dlist_node *next, *prev;
} dlist_node;

typedef struct dlist {
    size_t size;
    dlist_node *head, *back;
} dlist;

//creates and sets list variables
//size = 0, head = NULL
void init_list(dlist* l);

//adds data to front of list
//returns 0 on success and 1 on failure
int list_front_insert(dlist* l, tcb* data);

//adds data to back of list
//returns 0 on success and 1 on failure
int list_back_insert(dlist* l, tcb* data);

//adds to at index
//returns 0 on success and 1 on failure
int list_insert(dlist* l, tcb* data,size_t index);

//sets the data at the given index
//returns 0 on success and 1 on failure
int list_set(dlist* l, tcb* data, size_t index);

//removes data from front of list and pops
//returns data popped
tcb* list_pop_front(dlist* l);

//removes data from back of list and pops
//returns data popped
tcb* list_pop_back(dlist* l);

//removes data from list at index and pops
//returns data popped
tcb* list_pop_index(dlist* l,size_t index);

//gets a tcb using the worker_id
//returns NULL if doesnt exist, else returns tcb pointer
tcb* list_get(const dlist* l,worker_t id);

//frees all nodes
//DOES NOT FREE DATA
void free_list(dlist* list);

#endif