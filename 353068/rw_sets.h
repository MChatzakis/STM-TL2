#pragma once

#include <stdlib.h>
#include <stdbool.h>

typedef struct set_node
{  
    void *val; // unused for read set

    void *addr;
    struct set_node *next;
} set_node_t;

typedef struct set
{
    set_node_t *head;
    set_node_t *tail;
} set_t;

set_t *set_t_init();
void set_t_destroy(set_t *set);
bool set_t_add(set_t *set, void *addr, void *val);
bool set_t_remove(set_t *set, void *addr);

typedef set_t read_set_t;
typedef set_t write_set_t;