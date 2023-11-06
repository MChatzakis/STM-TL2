#pragma once

#include <stdlib.h>
#include <stdbool.h>

#include "bloom_filter.h"

typedef struct set_node
{
    void *val;   // unused for read sets
    size_t size; // unused for read sets

    void *addr;

    struct set_node *next;
} set_node_t;

typedef struct set
{
    set_node_t *head;
    set_node_t *tail;
    bloom_filter_t *bloom_filter;
} set_t;

typedef set_t read_set_t;
typedef set_t write_set_t;

set_t *set_t_init();
void set_t_destroy(set_t *set);

bool set_t_add(set_t *set, void *addr, void *val, size_t size);
bool set_t_remove(set_t *set, void *addr);
bool set_t_add_or_update(set_t *set, void *addr, void *val, size_t size);
void *set_t_get_val_or_null(set_t *set, void *addr);

void set_t_print(set_t *set, bool print_bloom);
