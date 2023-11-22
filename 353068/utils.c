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

/**
 * @brief Validates whether or not a write transaction is able to commit
 * 
 * @param region Shared memory region associated with the transaction
 * @param txn Write transaction trying to commit
 * @return COMMIT(true)/ABORT(false) if the transaction can commit or not 
 */
bool utils_check_commit(region_t *region, txn_t *txn)
{
    //
    // TL2 Algorithm (Commiting a write txn)
    //
    // In order to commit:
    //  - Try to lock the write set of the txn (using spinning). 
    //      > ABORT if not all locks are succesfully acquired
    //  - Increment and Fetch the Global version clock and store in txn.wv
    //  - Validate read-set. Check that for each entry in the read set the versioned number of the lock:
    //      a. Has version number <= rv
    //      b. Has not been locked
    //      > ABORT if either of a or b are true
    //      SPECIAL CASE: If wv = rv + 1, no need for validation of the read-set.
    //  - Commit. Iterate over the write set and:
    //      a. Apply the writing to the memory location
    //      b. Release the lock
    //

    // Try to lock the write set using bounded spinning
    if (!utils_try_lock_set(region, txn->write_set))
    {
        return ABORT;
    }

    // Increment and Fetch the value of the global_versioned_clock
    txn->wv = global_versioned_clock_t_increment_and_fetch(&region->global_versioned_clock);

    // If the values were modified by another txn, try to vadiate the read set
    if (txn->wv != txn->rv + 1)
    {
        // Validate read set
        if (!utils_validate_read_set(region, txn->read_set, txn->rv))
        {
            // Never forget to release the locks, even if the validation was not succesful
            utils_unlock_set(region, txn->write_set, txn->write_set->head, NULL);
            return ABORT;
        }
    }

    // Write the new values to the words of the write set, and release the locks
    utils_update_and_unlock_write_set(region, txn->write_set, txn->wv);

    return COMMIT;
}

bool utils_validate_read_set(region_t *region, read_set_t *set, int rv)
{
    set_node_t *curr = set->head;

    while (curr)
    {
        versioned_write_spinlock_t *vws = utils_get_mapped_lock(region->versioned_write_spinlock, curr->addr);

        if (!utils_validate_versioned_write_spinlock(vws, rv))
        {
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

void utils_update_and_unlock_write_set(region_t *region, write_set_t *set, int wv)
{
    set_node_t *curr = set->head;

    while (curr)
    {
        // 1. update:
        memcpy(curr->addr, curr->val, curr->size);

        // 2. release lock
        versioned_write_spinlock_t *vws = utils_get_mapped_lock(region->versioned_write_spinlock, curr->addr);
        versioned_write_spinlock_t_update_and_unlock(vws, wv);

        curr = curr->next;
    }
}

void utils_segment_list_insert(region_t *region, segment_t *sn)
{
    def_lock_t_lock(&region->segment_list_lock);
    sn->prev = NULL;
    sn->next = region->allocs;
    if (sn->next)
        sn->next->prev = sn;
    region->allocs = sn;
    def_lock_t_unlock(&region->segment_list_lock);
}

void utils_segment_list_remove(region_t *region, segment_t *sn)
{
    def_lock_t_lock(&region->segment_list_lock);
    if (sn->prev)
        sn->prev->next = sn->next;
    else
       region->allocs = sn->next;
    if (sn->next)
        sn->next->prev = sn->prev;
    def_lock_t_unlock(&region->segment_list_lock);

}