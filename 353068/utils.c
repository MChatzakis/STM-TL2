#include "utils.h"
#include <string.h>

versioned_write_spinlock_t *utils_get_mapped_lock(versioned_write_spinlock_t *locks, void *addr)
{
    return &locks[(uintptr_t)addr % VWSL_NUM];
}

bool utils_try_lock_set(region_t *region, set_t *set)
{
    set_node_t *curr = set->head;

    while (curr)
    {
        versioned_write_spinlock_t *vwsl = utils_get_mapped_lock(region->versioned_write_spinlock, curr->addr);
        if (!versioned_write_spinlock_t_lock(vwsl))
        {
            utils_unlock_set(region, set, set->head, curr);
            return false;
        }

        curr = curr->next;
    }

    return true;
}

void utils_unlock_set(region_t *region, set_t *unused(set), set_node_t *start, set_node_t *end)
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

        versioned_write_spinlock_t *vwsl = utils_get_mapped_lock(region->versioned_write_spinlock, curr->addr);
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

    txn->wv = atomic_fetch_add(&(region->global_versioned_clock).clock, 1) + 1;

    if (txn->wv != txn->rv + 1)
    {
        if (!utils_validate_read_set(region, txn->read_set, txn->rv))
        {
            utils_unlock_set(region, txn->write_set, txn->write_set->head, NULL);
            return false;
        }
    }

    utils_update_and_unlock_write_set(region, txn->write_set);

    return true;
}

bool utils_validate_read_set(region_t *region, read_set_t *set, int rv)
{
    set_node_t *curr = set->head;

    while (curr)
    {
        versioned_write_spinlock_t *vws = utils_get_mapped_lock(region->versioned_write_spinlock, curr->addr);
        
        if(!utils_validate_versioned_write_spinlock(vws, rv)){
            return false;
        }

        curr = curr->next;
    }

    return true;
}

bool utils_validate_versioned_write_spinlock(versioned_write_spinlock_t *vws, int rv)
{
    if (versioned_write_spinlock_t_get_state(vws) == LOCKED)
    {
        return false;
    }

    if (versioned_write_spinlock_t_get_version(vws) > rv)
    {
        return false;
    }

    return true;
}

void utils_update_and_unlock_write_set(region_t *region, write_set_t *set)
{
    set_node_t *curr = set->head;

    while (curr)
    {
        // 1. update:
        memcpy(curr->addr, curr->val, curr->size);

        // 2. release lock
        versioned_write_spinlock_t *vws = utils_get_mapped_lock(region->versioned_write_spinlock, curr->addr);
        versioned_write_spinlock_t_unlock(vws);

        curr = curr->next;
    }
}