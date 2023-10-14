#include <stdlib.h>
#include <stdbool.h>

#include "rw_sets.h"

set_t *set_t_init()
{
    set_t *set = (set_t *)malloc(sizeof(set_t));
    if (!set)
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
        free(curr);
        curr = next;
    }

    free(set);
}

bool set_t_add(set_t *set, void *addr, void *val)
{
    set_node_t *node = (set_node_t *)malloc(sizeof(set_node_t));
    if (!node)
    {
        return false;
    }

    node->addr = addr;
    node->val = val;
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

    return true;
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

            free(curr);
            return true;
        }

        prev = curr;
        curr = curr->next;
    }

    return false;
}
