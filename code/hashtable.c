#include "hashtable.h"
#include "thread-worker_types.h"

#include <errno.h>

typedef struct {
    tcb* data;
} hashtable_node;

void init_hashtable(hashtable *tmp) {
    tmp->hash = hash;
    tmp->size = 0;
    tmp->entry_len = 0;
    tmp->entries = NULL;
}

size_t one_one_half(size_t num) {
    return (num << 1) - (num >> 1);
}


int hashtable_insert(hashtable* ht,tcb* data) {
    if(ht->entry_len == 0) {
        if(hashtable_resize(ht,(ht->entry_len = 4)))
            return -1;
    }
    size_t index;
    dlist_node* list_index;
    while(index = ht->hash(data->id) % ht->entry_len,
    (one_one_half(ht->entry_len) >> 1) <= ht->entries[index].size) {
        if(hashtable_resize(ht,ht->entry_len << 1))
            return -1;
    }
    if(list_get(&ht->entries[index],data->id)) 
        return 1;
    list_front_insert(&ht->entries[index],data);
    ht->size++;
    return 0;
}

int hashtable_resize(hashtable *ht, size_t newsize) {
    hashtable tmp;
    tmp.hash = ht->hash;
    tmp.size = 0;
    tmp.entry_len = newsize;
    tmp.entries = malloc(sizeof(dlist) * newsize); 
    if(!tmp.entries) {
        errno = ENOMEM;
        return 1;
    }
    for(size_t i = 0; i < tmp.entry_len; ++i) {
        init_list(&tmp.entries[i]);
    }
    for(size_t i = 0; i < ht->entry_len; ++i) {
        while(ht->entries[i].size) {
            if(hashtable_insert(&tmp,ht->entries[i].head->data) == -1) {
                return 1;
            }
            list_pop_front(&ht->entries[i]);
        }
    }
    ht->entry_len = newsize;
    ht->entries = tmp.entries;
    return 0;
}

void free_hashtable(hashtable* ht) {
    while(ht->entry_len--) {
        while(ht->entries[ht->entry_len].size) {
            free(list_pop_front(&ht->entries[ht->entry_len]));
        }
    }
}

tcb *hashtable_remove(hashtable *ht,worker_t id) {
    size_t index = ht->hash(id) % ht->entry_len;
    dlist* entry = &ht->entries[index];
    dlist_node* traverse = entry->head;
    while(traverse) {
        if(traverse->data->id == id) {
            break;
        }
        traverse = traverse->next;
    }
    if(traverse) {
        tcb* data;
        if(traverse == entry->head) {
            data = list_pop_front(entry);
        } else if (traverse == entry->back) {
            data = list_pop_back(entry);
        } else {
            traverse->prev->next = traverse->next;
            traverse->next->prev = traverse->prev;
            data = traverse->data;
            free(traverse);
        }
        return data;
    }
    return NULL;
}

tcb *hashtable_get(hashtable ht,worker_t id){
    if(!ht.entries) return NULL;
    size_t index = ht.hash(id) % ht.entry_len;
    dlist_node* traverse = ht.entries[index].head;
    while(traverse) {
        if(traverse->data->id == id) {
            return traverse->data;
        }
        traverse = traverse->next;
    }
    return NULL;
}

size_t hash(worker_t id)
{
    return (size_t)id;
}