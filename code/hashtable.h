#ifndef HASHTABLE_H
#define HASHTABLE_H

#define _GNU_SOURCE

#include <stdlib.h>

#include "dlist.h"


typedef struct hashtable {
    dlist* entries; //where we store entries
    size_t (*hash)(worker_t); //hashing function
    size_t size; //total num of elements
    size_t entry_len; //amount of lists
} hashtable;


//initializes hashtable
//size = 0, entry_len = 0, entries = NULL
void init_hashtable(hashtable* ht);

//Will either insert or replaces the value depending on whether it exists
//returns 0 on insert, 1 on replace, -1 on failure (failure to allocate memory and sets errno to ENOMEM)
int hashtable_insert(hashtable* ht,tcb* newthread);

//Removes tcb from hashtable ht
//returns either NULL or the tcb removed the table
tcb* hashtable_remove(hashtable* ht,worker_t id);

//attempts to retrieve tcb in the hashtable with the id provided
//returns either NULL or tcb in the table
tcb* hashtable_get(hashtable ht,worker_t id);

//resizes the capacity of the hashtable entry 
//returns 0 on success, 1 on failure
int hashtable_resize(hashtable* ht, size_t newsize);

//hashes an id
size_t hash(worker_t id);

//frees hashtable
void free_hashtable(hashtable* ht);

#endif