#include "utils.h"

versioned_write_spinlock_t *get_mapped_lock(versioned_write_spinlock_t *locks, void *addr)
{
    return &locks[(uintptr_t)addr % VWSL_NUM];
}

bool try_lock_set(region_t *region, set_t *set)
{
    set_node_t *curr = set->head;

    while (curr)
    {
        versioned_write_spinlock_t *vwsl = get_mapped_lock(region->versioned_write_spinlock, curr->addr);
        if (!versioned_write_spinlock_t_lock(vwsl))
        {
            return false;
        }

        curr = curr->next;
    }

    return true;
}