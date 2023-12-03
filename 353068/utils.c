#include "utils.h"

#include <string.h>

txn_t *txn_t_init(bool is_ro, int rv, int wv)
{
    txn_t *txn = (txn_t *)malloc(sizeof(txn_t));
    if (unlikely(!txn))
    {
        return NULL;
    }

    txn->is_ro = is_ro;
    txn->rv = rv;
    txn->wv = wv;
    txn->read_set = set_t_init();
    if (unlikely(!txn->read_set))
    {
        free(txn);
        return NULL;
    }

    txn->write_set = set_t_init();
    if (unlikely(!txn->write_set))
    {
        set_t_destroy(txn->read_set);
        free(txn);
        return NULL;
    }

    return txn;
}

void txn_t_destroy(txn_t *txn)
{
    set_t_destroy(txn->read_set);
    set_t_destroy(txn->write_set);

    free(txn);
}

versioned_write_spinlock_t *utils_get_mapped_lock(versioned_write_spinlock_t *locks, void *addr)
{
    // Choose a prime number for better distribution
    
    uintptr_t x = (uintptr_t)addr;
    
    //x = ((x >> 16) ^ x) * 0x45d9f3b;
    //x = ((x >> 16) ^ x) * 0x45d9f3b;
    //x = (x >> 16) ^ x;

    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);

    return &locks[x % VWSL_NUM];
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
        
        assert(atomic_load(&vwsl->lock_and_version) & 0x1);
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

    // The write set it ordered here.

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
    int l = versioned_write_spinlock_t_load(vws);
    if (l & 0x1 || ((l >> 1) > rv))
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
        memcpy(curr->addr, curr->val, curr->size);

        versioned_write_spinlock_t *vws = utils_get_mapped_lock(region->versioned_write_spinlock, curr->addr);
        assert(atomic_load(&vws->lock_and_version) & 0x1);
        versioned_write_spinlock_t_update_version(vws, wv); // Updates and unlocks

        curr = curr->next;
    }
}

void dprint_clog(char *color, FILE *stream, const char *str, ...)
{
    if (!DEBUG_PRINT)
    {
        return;
    }

    va_list args;
    va_start(args, str);
    vfprintf(stream, color, args);
    vfprintf(stream, str, args);
    vfprintf(stream, COLOR_RESET, args);
    va_end(args);
}

void dprint_cwarn(char *color, FILE *stream, const char *str, ...)
{
    if (!ENABLE_WARNINGS)
    {
        return;
    }

    va_list args;
    va_start(args, str);
    vfprintf(stream, color, args);
    vfprintf(stream, str, args);
    vfprintf(stream, COLOR_RESET, args);
    va_end(args);
}
