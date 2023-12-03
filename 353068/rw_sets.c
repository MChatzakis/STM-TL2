#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "rw_sets.h"

#include <string.h>

bloom_filter_t *bloom_filter_t_create()
{
    bloom_filter_t *bf = (bloom_filter_t *)malloc(sizeof(bloom_filter_t));
    if (!bf)
    {
        return NULL;
    }

    for (int i = 0; i < BLOOM_FILTER_SIZE; i++)
    {
        bf->filter[i] = false;
    }
    return bf;
}

void bloom_filter_t_destroy(bloom_filter_t *bloom_filter)
{
    free(bloom_filter);
}

void bloom_filter_t_add(bloom_filter_t *bloom_filter, uintptr_t address)
{
    uintptr_t x = address;

    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);

    int index = x % BLOOM_FILTER_SIZE;
    bloom_filter->filter[index] = true;
}

bool bloom_filter_t_contains(bloom_filter_t *bloom_filter, uintptr_t address)
{
    uintptr_t x = address;

    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);

    return bloom_filter->filter[x % BLOOM_FILTER_SIZE];
}

set_t *set_t_init()
{
    set_t *set = (set_t *)malloc(sizeof(set_t));
    if (!set)
    {
        return NULL;
    }

    set->head = NULL;
    set->tail = NULL;
    set->bloom_filter = bloom_filter_t_create();
    if (!set->bloom_filter)
    {
        return NULL;
    }

    return set;
}

void set_t_destroy(set_t *set)
{
    set_node_t *curr = set->head;
    set_node_t *next = NULL;

    while (curr)
    {
        next = curr->next;
        free(curr);
        curr = next;
    }

    bloom_filter_t_destroy(set->bloom_filter);

    free(set);
}

set_node_t * set_t_allocate_node(void *addr, void *val, size_t size){
    set_node_t *node = (set_node_t *)malloc(sizeof(set_node_t));
    if (!node)
    {
        return NULL;
    }

    node->addr = addr;
    node->size = size;
    node->next = NULL;

    // Allocate val
    if (val != NULL)
    {
        node->val = (void *)malloc(size);
        if (!node->val)
        {
            return NULL;
        }
        memcpy(node->val, val, size);
    }

    return node;
}

bool set_t_remove(set_t *set, void *addr)
{
    set_node_t *curr = set->head;
    set_node_t *prev = NULL;

    while (curr)
    {
        if (curr->addr == addr)
        {
            if (prev)
            {
                prev->next = curr->next;
            }
            else
            {
                set->head = curr->next;
            }

            if (!curr->next)
            {
                set->tail = prev;
            }

            free(curr->val);
            free(curr);

            return true;
        }

        prev = curr;
        curr = curr->next;
    }

    return false;
}

bool set_t_add_or_update(set_t *set, void *addr, void *val, size_t size)
{
    set_node_t *node = set_t_allocate_node(addr, val, size);
    if (!node)
    {
        return false;
    }

    if (!set->head)
    {
        
        set->head = node;
        set->tail = node;

        return true;
    }

    set_node_t *curr = set->head;
    set_node_t *prev = NULL;

    while (curr)
    {
        if (curr->addr == addr){
            if (val != NULL)
            {
                memcpy(curr->val, val, size);
            }

            return true;
        }

        if (curr->addr > addr)
        {
            break;
        }

        prev = curr;
        curr = curr->next;
    }

    if (prev == NULL){
        node->next = set->head;
        set->head = node;
    } else {
        node->next = curr;
        prev->next = node;
    }

    if (!curr)
    {
        set->tail = node;
    }
    
    return true;
}

void *set_t_get_val_or_null(set_t *set, void *addr)
{
    set_node_t *curr = set->head;

    /*if (!bloom_filter_t_contains(set->bloom_filter, (uintptr_t)addr))
    {
        return NULL;
    }*/

    while (curr)
    {
        if (curr->addr == addr)
        {
            return curr->val;
        }

        curr = curr->next;
    }

    return NULL;
}

void set_t_delete_if_exists(set_t *set, void *addr)
{
    set_node_t *curr = set->head;
    set_node_t *prev = NULL;

    while (curr)
    {
        if (curr->addr == addr)
        {
            if (prev)
            {
                prev->next = curr->next;
            }
            else
            {
                set->head = curr->next;
            }

            if (!curr->next)
            {
                set->tail = prev;
            }

            free(curr->val);
            free(curr);

            return;
        }

        prev = curr;
        curr = curr->next;
    }

    return;
}
