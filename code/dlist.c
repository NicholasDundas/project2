#include "dlist.h"

#include <stdlib.h>
#include <errno.h>

void init_list(dlist* l) {
    l->size = 0;
    l->head = NULL;
    l->back = NULL;
}

dlist_node* init_lnode(dlist_node* prev, dlist_node* next, tcb* data) {
    dlist_node* temp_node = malloc(sizeof(dlist_node));
    if(temp_node) {
        temp_node->data = data;
        temp_node->next = next;
        temp_node->prev = prev;
    } else {
        errno = ENOMEM;
    }
    return temp_node;
}

int list_front_insert(dlist* l, tcb* data) {
    dlist_node* temp_node = init_lnode(l->back,l->head, data);
    if(!temp_node) {
        return 1;
    }

    l->head = temp_node;
    if(l->back == NULL) {
        l->back = l->head;
    } else {
        l->back->next = l->head;
    }
    ++l->size;
    return 0;
}

int list_back_insert(dlist *l, tcb *data) {
    if(l->size == 0) {
        return list_front_insert(l, data);
    } else {
        dlist_node* temp_node = init_lnode(l->back,l->head, data);
        if(!temp_node) {
            return 1;
        }

        l->back->next = temp_node;
        l->head->prev = temp_node;
        l->back = temp_node;
        l->size++;
        return 0;
    }
}

int list_insert(dlist *l, tcb *data, size_t index) {
    if(index == 0) {
        return list_front_insert(l,data);
    } else if (index == l->size) {
        return list_back_insert(l,data);
    } else if (index < l->size) {

        dlist_node* traverse = l->head; 
        while(--index) {
            traverse = traverse->next;
        }

        dlist_node* temp_node = init_lnode(traverse, traverse->next, data);
        if(!temp_node) {
            return 1;
        }
        traverse->next->prev = temp_node;
        traverse->next = temp_node;
        ++l->size;
        return 0;
    }
    return 1;
}

int list_set(dlist *l, tcb *data, size_t index) {
    if(l->size != 0 && index < l->size) {
        dlist_node* transverse = l->head;
        while(index--) {
            transverse = transverse->next;
        }
        transverse->data = data;
        return 0;
    }
    return 1;
}



tcb* list_pop_front(dlist* l) {
    if (l->size == 0) {
        return NULL;
    }
    dlist_node* node_to_remove = l->head;
    tcb* data = node_to_remove->data;
    l->head = node_to_remove->next;
    l->back->next = l->head;
    l->head->prev = l->back;
    free(node_to_remove);
    --l->size;
    return data;
}

tcb* list_pop_back(dlist* l) {
    if (l->size == 0) {
        return NULL;
    }
    dlist_node* node_to_remove = l->back;
    tcb* data = node_to_remove->data;
    l->back = node_to_remove->prev;
    l->back->next = l->head;
    l->head->prev = l->back;
    free(node_to_remove);
    --l->size;
    return data;
}

tcb* list_pop_index(dlist* l,size_t index) {
    if (l->size == 0 || index > l->size) {
        return NULL;
    }
    if(index == 0) {
        return list_pop_front(l);
    } else if (index == l->size)  {
        return list_pop_back(l);
    } else {
        dlist_node* traverse = l->head; 
        while(--index) {
            traverse = traverse->next;
        }
        traverse->prev->next = traverse->next;
        traverse->next->prev = traverse->prev;
        tcb* data = traverse->data;
        free(traverse);
        return data;
    }
}

tcb* list_get(const dlist* l, worker_t id) {   
    dlist_node* cur = l->head;
    while(cur && cur->data->id != id) {
        cur = cur->next;
    }
    return cur ? cur->data : NULL;
}


void free_list(dlist* l) {
    if(l) {
        dlist_node* node = l->head;
        while (l->size--) {
            dlist_node* next_node = node->next;
            free(node);
            node = next_node;
        }
        l->head = NULL;
    }
}
