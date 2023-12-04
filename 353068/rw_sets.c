#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "rw_sets.h"


set_t *set_t_init()
{
    set_t *set = (set_t *)malloc(sizeof(set_t));
    if (unlikely(!set))
    {
        return NULL;
    }

    set->head = NULL;
    set->tail = NULL;

    return set;
}

void set_t_destroy(set_t *set)
{
    set_node_t *curr = set->head;
    set_node_t *next = NULL;

    while (curr)
    {
        next = curr->next;

        if (curr->val != NULL)
        {
            free(curr->val);
        }
        curr->val = NULL;

        free(curr);
        curr = next;
    }

    free(set);
}

set_node_t *set_t_allocate_node(void *addr, void *val, size_t size)
{
    set_node_t *node = (set_node_t *)malloc(sizeof(set_node_t));
    if (unlikely(!node))
    {
        return NULL;
    }

    node->addr = addr;
    node->size = size;
    node->next = NULL;
    node->val = NULL;

    // Allocate val
    if (val != NULL)
    {
        node->val = (void *)malloc(size);
        if (unlikely(!node->val))
        {
            return NULL;
        }
        memcpy(node->val, val, size);
    }

    return node;
}

bool set_t_add_or_update(set_t *set, void *addr, void *val, size_t size)
{
    set_node_t *curr = set->head;
    set_node_t *prev = NULL;

    while (curr)
    {
        if (curr->addr == addr)
        {
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

    set_node_t *node = set_t_allocate_node(addr, val, size);
    if (unlikely(!node))
    {
        return false;
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

    if (!curr)
    {
        set->tail = node;
    }

    return true;
}

void *set_t_get_val_or_null(set_t *set, void *addr)
{
    set_node_t *curr = set->head;

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
