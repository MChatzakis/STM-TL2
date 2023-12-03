#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "rw_sets.h"

#include <string.h>

/*
    ====================
    Write Set Implementations
    ====================
*/

write_set_t *write_set_t_init()
{
    write_set_t *set = (write_set_t *)malloc(sizeof(write_set_t));
    if (unlikely(!set))
    {
        return NULL;
    }

    set->head = NULL;

    return set;
}

void write_set_t_destroy(write_set_t *set)
{
    write_set_node_t *curr = set->head;
    write_set_node_t *next = NULL;

    while (curr)
    {
        next = curr->next;
        free(curr);
        curr = next;
    }

    free(set);
}

write_set_node_t *write_set_t_allocate_node(void *addr, void *val, size_t size)
{
    write_set_node_t *node = (write_set_node_t *)malloc(sizeof(write_set_node_t));
    if (unlikely(!node))
    {
        return NULL;
    }

    node->addr = addr;
    node->size = size;
    node->val = NULL;
    node->next = NULL;

    node->val = (void *)malloc(size);
    if (unlikely(!node->val))
    {
        return NULL;
    }
    memcpy(node->val, val, size);

    return node;
}

bool write_set_t_add(write_set_t *set, void *addr, void *val, size_t size)
{
    write_set_node_t *node = write_set_t_allocate_node(addr, val, size);
    if (unlikely(!node))
    {
        return false;
    }

    if (!set->head)
    {
        set->head = node;
        return true;
    }

    write_set_node_t *curr = set->head;
    write_set_node_t *prev = NULL;

    while (curr)
    {
        if (curr->addr == addr)
        {
            memcpy(curr->val, val, size);
            return true;
        }

        if (curr->addr > addr)
        {
            break;
        }

        prev = curr;
        curr = curr->next;
    }

    if (prev == NULL)
    {
        node->next = set->head;
        set->head = node;
    }
    else
    {
        node->next = curr;
        prev->next = node;
    }

    return true;
}

void *write_set_t_get_val(write_set_t *set, void *addr)
{
    write_set_node_t *curr = set->head;

    while (curr)
    {
        if (curr->addr == addr)
        {
            return curr->val;
        }

        if (curr->addr > addr)
        {
            return NULL;
        }

        curr = curr->next;
    }

    return NULL;
}

/*
    ====================
    Read Set Implementations
    ====================
*/

read_set_t *read_set_t_init()
{
    read_set_t *set = (read_set_t *)malloc(sizeof(read_set_t));
    if (unlikely(!set))
    {
        return NULL;
    }

    set->head = NULL;
    set->tail = NULL;

    for (int i = 0; i < BLOOM_FILTER_SIZE; i++)
    {
        set->bloom_filter[i] = false;
    }

    return set;
}

void read_set_t_destroy(read_set_t *set)
{
    read_set_node_t *curr = set->head;
    read_set_node_t *next = NULL;

    while (curr)
    {
        next = curr->next;
        free(curr);
        curr = next;
    }

    free(set);
}

bool read_set_t_append(read_set_t *set, void *addr)
{
    read_set_node_t *node = (read_set_node_t *)malloc(sizeof(read_set_node_t));
    if (unlikely(!node))
    {
        return false;
    }

    node->addr = addr;
    node->next = NULL;

    if (!set->head)
    {
        set->head = node;
        set->tail = node;
    }
    else
    {
        set->tail->next = node;
        set->tail = node;
    }

    //set->bloom_filter[(uintptr_t)addr % BLOOM_FILTER_SIZE] = true;

    return true;
}

bool read_set_t_add(read_set_t *set, void *addr)
{
    read_set_node_t *curr = set->head;

    /*if (!set->bloom_filter[(uintptr_t)addr % BLOOM_FILTER_SIZE]) // Surely not in the set
    {
        return read_set_t_append(set, addr);
    }*/

    while (curr)
    {
        if (curr->addr == addr)
        {
            return true;
        }

        curr = curr->next;
    }

    return read_set_t_append(set, addr);
}