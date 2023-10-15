/**
 * @file   tm.c
 * @author Emmanouil (Manos) Chatzakis
 *
 * @section LICENSE
 *
 * [...]
 *
 * @section Software transactional memory implementing TL2 algorithm. 
 *
 * Implementation of your own transaction manager.
 * You can completely rewrite this file (and create more files) as you wish.
 * Only the interface (i.e. exported symbols and semantic) must be preserved.
 **/

// Requested features
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#ifdef __STDC_NO_ATOMICS__
#error Current C11 compiler does not support atomic operations
#endif

// External headers
#include <pthread.h>

// Internal headers
#include <tm.h>
#include <assert.h>
#include <string.h>

#include "tm_types.h"
#include "txn.h"
#include "utils.h"

#include "macros.h"

/** Create (i.e. allocate + init) a new shared memory region, with one first non-free-able allocated segment of the requested size and alignment.
 * @param size  Size of the first shared segment of memory to allocate (in bytes), must be a positive multiple of the alignment
 * @param align Alignment (in bytes, must be a power of 2) that the shared memory region must support
 * @return Opaque shared memory region handle, 'invalid_shared' on failure
 **/
shared_t tm_create(size_t size, size_t align)
{
    // TODO: tm_create(size_t, size_t)

    // Allocate memory for the region struct fields
    region_t *region = (region_t *)malloc(sizeof(region_t));
    if (unlikely(!region))
    {
        return invalid_shared;
    }

    // Allign and allocate memory for the shared memory region
    if (posix_memalign(&(region->start), align, size) != 0)
    {
        free(region);
        return invalid_shared;
    }

    // Initialize the region struct fields
    memset(region->start, 0, size);
    region->size = size;
    region->align = align;
    region->allocs = NULL;

    // Initialize the locks related to this region
    global_versioned_clock_t_init(&region->global_versioned_clock);
    def_lock_t_init(&region->segment_list_lock);
    for (int i = 0; i < VWSL_NUM; i++)
    {
        versioned_write_spinlock_t_init(&region->versioned_write_spinlock[i]);
    }

    return region;
}

/** Destroy (i.e. clean-up + free) a given shared memory region.
 * @param shared Shared memory region to destroy, with no running transaction
 **/
void tm_destroy(shared_t shared)
{
    // TODO: tm_destroy(shared_t)
    region_t *region = (region_t *)shared;
    free(region->start);

    // Destroy the locks related to this region
    global_versioned_clock_t_destroy(&region->global_versioned_clock);
    def_lock_t_destroy(&region->segment_list_lock);
    for (int i = 0; i < VWSL_NUM; i++)
    {
        versioned_write_spinlock_t_destroy(&region->versioned_write_spinlock[i]);
    }

    assert(region->allocs == NULL);

    free(region);
}

/** [thread-safe] Return the start address of the first allocated segment in the shared memory region.
 * @param shared Shared memory region to query
 * @return Start address of the first allocated segment
 **/
void *tm_start(shared_t shared)
{
    // TODO: tm_start(shared_t)
    return ((region_t *)shared)->start;
}

/** [thread-safe] Return the size (in bytes) of the first allocated segment of the shared memory region.
 * @param shared Shared memory region to query
 * @return First allocated segment size
 **/
size_t tm_size(shared_t shared)
{
    // TODO: tm_size(shared_t)
    return ((region_t *)shared)->size;
}

/** [thread-safe] Return the alignment (in bytes) of the memory accesses on the given shared memory region.
 * @param shared Shared memory region to query
 * @return Alignment used globally
 **/
size_t tm_align(shared_t shared)
{
    // TODO: tm_align(shared_t)
    return ((region_t *)shared)->align;
}

/** [thread-safe] Begin a new transaction on the given shared memory region.
 * @param shared Shared memory region to start a transaction on
 * @param is_ro  Whether the transaction is read-only
 * @return Opaque transaction ID, 'invalid_tx' on failure
 **/
tx_t tm_begin(shared_t shared, bool is_ro)
{
    // TODO: tm_begin(shared_t)
    region_t *region = (region_t *)shared;

    // 1. TL2: Sample load the current value of the global version clock
    txn_t *txn = txn_t_init(is_ro, global_versioned_clock_t_get_clock(&region->global_versioned_clock), -1);
    if (unlikely(!txn))
    {
        return invalid_tx;
    }

    return (tx_t)txn;
}

/** [thread-safe] End the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to end
 * @return Whether the whole transaction committed
 **/
bool tm_end(shared_t shared, tx_t tx)
{
    // TODO: tm_end(shared_t, tx_t)
    region_t *region = (region_t *)shared;
    txn_t *txn = (txn_t *)tx;

    bool commit_result;
    if (txn->is_ro)
    {
        commit_result = true;
    }
    else
    {
        commit_result = utils_check_commit(region, txn);
    }

    txn_t_destroy(txn);
    return commit_result;
}

/** [thread-safe] Read operation in the given transaction, source in the shared region and target in a private region.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param source Source start address (in the shared region)
 * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
 * @param target Target start address (in a private region)
 * @return Whether the whole transaction can continue
 **/
bool tm_read(shared_t shared, tx_t tx, void const *source, size_t size, void *target)
{
    // TODO: tm_read(shared_t, tx_t, void const*, size_t, void*)
    region_t *region = (region_t *)shared;
    txn_t *txn = (txn_t *)tx;

    size_t align = region->align < sizeof(struct segment_node *) ? sizeof(void *) : region->align;

    if (txn->is_ro)
    {
        for (size_t i = 0; i < size; i += align)
        {
            void *word_addr = (void *) source + i;
            versioned_write_spinlock_t *vws = utils_get_mapped_lock(region->versioned_write_spinlock, word_addr);
            if (!utils_validate_versioned_write_spinlock(vws, txn->rv))
            {
                return false;
            }
        }

        memcpy(target, source, size);
    }
    else
    {
        for (size_t i = 0; i < size; i += align)
        {
            void *word_addr = (void *) source + i;
            void *targ_addr = target + i;
            size_t word_size = align;

            // Add the entry to the read set
            set_t_add_or_update(txn->read_set, word_addr, NULL, align);

            void *val;
            if ((val = set_t_get_val_or_null(txn->write_set, word_addr)) != NULL)
            {
                memcpy(targ_addr, val, word_size);
                return true;
            }
            else{
                memcpy(targ_addr, word_addr, word_size); //write the old value
            }

            versioned_write_spinlock_t *vws = utils_get_mapped_lock(region->versioned_write_spinlock, word_addr);
            if (!utils_validate_versioned_write_spinlock(vws, txn->rv))
            {
                return false;
            }
        }
    }

    return true;
}

/** [thread-safe] Write operation in the given transaction, source in a private region and target in the shared region.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param source Source start address (in a private region)
 * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
 * @param target Target start address (in the shared region)
 * @return Whether the whole transaction can continue
 **/
bool tm_write(shared_t shared, tx_t tx, void const *source, size_t size, void *target)
{
    // TODO: tm_write(shared_t, tx_t, void const*, size_t, void*)
    region_t *region = (region_t *)shared;
    txn_t *txn = (txn_t *)tx;

    size_t align = region->align < sizeof(struct segment_node *) ? sizeof(void *) : region->align;

    // iterate the words of source (word_size = align)
    for (size_t i = 0; i < size; i += align)
    {
        void *word_addr = target + i;
        void *source_addr = (void *) source + i;
        size_t word_size = align;

        // Saves the pair (address,value)
        set_t_add_or_update(txn->write_set, word_addr, source_addr, word_size);
    }

    return true;
}

/** [thread-safe] Memory allocation in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param size   Allocation requested size (in bytes), must be a positive multiple of the alignment
 * @param target Pointer in private memory receiving the address of the first byte of the newly allocated, aligned segment
 * @return Whether the whole transaction can continue (success/nomem), or not (abort_alloc)
 **/
alloc_t tm_alloc(shared_t shared, tx_t unused(tx), size_t size, void **target)
{
    // TODO: tm_alloc(shared_t, tx_t, size_t, void**)
    region_t *region = (region_t *)shared;

    size_t align = region->align;
    align = align < sizeof(segment_t *) ? sizeof(void *) : align;

    segment_t *sn;
    if (unlikely(posix_memalign((void **)&sn, align, sizeof(segment_t) + size) != 0)) // Allocation failed
        return nomem_alloc;

    // Insert in the linked list
    def_lock_t_lock(&region->segment_list_lock);
    sn->prev = NULL;
    sn->next = region->allocs;
    if (sn->next)
        sn->next->prev = sn;
    region->allocs = sn;
    def_lock_t_unlock(&region->segment_list_lock);

    void *segment = (void *)((uintptr_t)sn + sizeof(segment_t));
    memset(segment, 0, size);
    *target = segment;

    return success_alloc;
}

/** [thread-safe] Memory freeing in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param target Address of the first byte of the previously allocated segment to deallocate
 * @return Whether the whole transaction can continue
 **/
bool tm_free(shared_t shared, tx_t unused(tx), void *target)
{
    // TODO: tm_free(shared_t, tx_t, void*)
    region_t *region = (region_t *)shared;
    segment_t *sn = (segment_t *)((uintptr_t)target - sizeof(segment_t));

    // Remove from the linked list
    def_lock_t_lock(&region->segment_list_lock);
    if (sn->prev)
        sn->prev->next = sn->next;
    else
        region->allocs = sn->next;
    if (sn->next)
        sn->next->prev = sn->prev;
    def_lock_t_unlock(&region->segment_list_lock);

    free(sn);
    return true;
}
