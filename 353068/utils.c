#include "utils.h"


versioned_write_spinlock_t *utils_get_mapped_lock(versioned_write_spinlock_t *locks, void *addr)
{
    return &locks[(uintptr_t)addr % VWSL_NUM];
}

bool utils_try_lock_set(region_t *region, set_t *set)
{
    set_node_t *curr = set->head;

    while (curr)
    {
        versioned_write_spinlock_t *vwsl = get_mapped_lock(region->versioned_write_spinlock, curr->addr);
        if (!versioned_write_spinlock_t_lock(vwsl))
        {
            unlock_set(region, set, set->head, curr);
            return false;
        }

        curr = curr->next;
    }

    return true;
}

void utils_unlock_set(region_t *region, set_t *set, set_node_t *start, set_node_t *end)
{
    // Note: to unlock the full set, start=set->head and end=NULL.
    set_node_t *curr = start;

    while (curr)
    {
        // The last node is the node that the lock failed
        // Thus, we dont need to unlock anything and we stop.
        if (curr == end)
        {
            return;
        }

        versioned_write_spinlock_t *vwsl = get_mapped_lock(region->versioned_write_spinlock, curr->addr);
        versioned_write_spinlock_t_unlock(vwsl);

        curr = curr->next;
    }
}

bool utils_check_commit(region_t *region, txn_t *txn)
{
    if (!utils_try_lock_set(region, txn->write_set))
    {
        return false;
    }

    txn->wv = __atomic_add_fetch(&region->global_versioned_clock.clock, 1, 0); //???

    if (txn->wv != txn->rv + 1)
    {
        if (!utils_validate_read_set())
        {
            return false;
        }
    }

    if (!utils_update_and_unlock_write_set())
    {
        return false;
    }

    return true;
}
