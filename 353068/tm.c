/**
 * @file   tm.c
 * @author Emmanouil (Manos) Chatzakis
 *
 * @section LICENSE
 *
 * [...]
 *
 * @section DESCRIPTION
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

// Internal headers
#include <tm.h>

#include "macros.h"
#include "pthread.h"

typedef struct lock
{
    pthread_mutex_t mutex;
} lock_t;

int lock_t_get(lock_t *lock);
int lock_t_release(lock_t *lock);
int lock_t_init(lock_t *lock);
int lock_t_destroy(lock_t *lock);
int lock_t_condition(pthread_cond_t *cond_var, lock_t *lock);
int lock_t_wake_up(pthread_cond_t *cond_var);

int lock_t_get(lock_t *lock)
{
    return (pthread_mutex_lock(&lock->mutex) == 0);
}

int lock_t_release(lock_t *lock)
{
    return (pthread_mutex_unlock(&lock->mutex) == 0);
}

int lock_t_init(lock_t *lock)
{
    return (pthread_mutex_init(&lock->mutex, NULL) == 0);
}

int lock_t_destroy(lock_t *lock)
{
    return (pthread_mutex_destroy(&lock->mutex) == 0);
}

int lock_t_condition(pthread_cond_t *cond_var, lock_t *lock)
{
    return (pthread_cond_wait(cond_var, &lock->mutex) == 0);
}

int lock_t_wake_up(pthread_cond_t *cond_var)
{
    return (pthread_cond_broadcast(cond_var) == 0);
}


typedef struct batcher
{
    int counter;   // Epoch indicator
    int remaining; // How many threads are still in the epoch (after enter())
    int blocked;   // How many threads are blocked on enter()

    lock_t lock; // Lock to access the field of batcher
    pthread_cond_t blocking_condition; // Condition variable to block threads on enter()
} batcher_t;

void batcher_t_enter(batcher_t *batcher);
void batcher_t_exit(batcher_t *batcher);
void batcher_t_get_epoch(batcher_t *batcher);
void batcher_t_destroy(batcher_t *batcher);

void batcher_t_enter(batcher_t *batcher)
{
    lock_t_get(&batcher->lock);

    if (batcher->remaining == 0)
    {
        batcher->remaining = 1;
    }
    else
    {
        batcher->blocked++;

        // This function releases the lock and blocks the calling thread
        lock_t_condition(&batcher->blocking_condition, &batcher->lock);
    }

    lock_t_release(&batcher->lock);
}

void batcher_t_exit(batcher_t *batcher)
{
    get_lock(batcher->lock);
    
    // Thread exit the region
    batcher->remaining--;

    // If this is the last thread of the batch, we need special handling
    if (batcher->remaining == 0)
    {   
        // Update statistics and shared variables
        batcher->counter++; // Next epoch
        batcher->remaining = batcher->blocked; // Blocked threads will enter the next epoch
        batcher->blocked = 0; // Stopeed threads are not blocked anymore

        // Wake up all threads that are blocked on enter()
        lock_t_wake_up(&batcher->blocking_condition);
    }

    release_lock(batcher->lock);
}

void batcher_t_get_epoch(batcher_t *batcher)
{
    lock_t_get(&batcher->lock);
    int epoch = batcher->counter;
    lock_t_release(&batcher->lock);

    return epoch;
}

void batcher_t_destroy(batcher_t *batcher)
{
    lock_t_destroy(&batcher->lock);
    pthread_cond_destroy(&batcher->blocking_condition);
}


/**
 * @brief Segment of dynamically allocated memory.
 *
 */
typedef struct segment_node
{
    struct segment_node *prev;
    struct segment_node *next;
    // uint8_t segment[] // segment of dynamic size
} segment_node_t;

/**
 * @brief Struct representing a transactional shared-memory region.
 *
 */
typedef struct region
{
    void *start;

    size_t size;
    size_t align;

    segment_node_t *allocs;

    batcher_t batcher;
} region_t;

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
    ((region_t *)shared)->size;
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
tx_t tm_begin(shared_t unused(shared), bool unused(is_ro))
{
    // TODO: tm_begin(shared_t)
    return invalid_tx;
}

/** [thread-safe] End the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to end
 * @return Whether the whole transaction committed
 **/
bool tm_end(shared_t unused(shared), tx_t unused(tx))
{
    // TODO: tm_end(shared_t, tx_t)
    return false;
}

/** [thread-safe] Read operation in the given transaction, source in the shared region and target in a private region.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param source Source start address (in the shared region)
 * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
 * @param target Target start address (in a private region)
 * @return Whether the whole transaction can continue
 **/
bool tm_read(shared_t unused(shared), tx_t unused(tx), void const *unused(source), size_t unused(size), void *unused(target))
{
    // TODO: tm_read(shared_t, tx_t, void const*, size_t, void*)
    return false;
}

/** [thread-safe] Write operation in the given transaction, source in a private region and target in the shared region.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param source Source start address (in a private region)
 * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
 * @param target Target start address (in the shared region)
 * @return Whether the whole transaction can continue
 **/
bool tm_write(shared_t unused(shared), tx_t unused(tx), void const *unused(source), size_t unused(size), void *unused(target))
{
    // TODO: tm_write(shared_t, tx_t, void const*, size_t, void*)
    return false;
}

/** [thread-safe] Memory allocation in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param size   Allocation requested size (in bytes), must be a positive multiple of the alignment
 * @param target Pointer in private memory receiving the address of the first byte of the newly allocated, aligned segment
 * @return Whether the whole transaction can continue (success/nomem), or not (abort_alloc)
 **/
alloc_t tm_alloc(shared_t unused(shared), tx_t unused(tx), size_t unused(size), void **unused(target))
{
    // TODO: tm_alloc(shared_t, tx_t, size_t, void**)
    return abort_alloc;
}

/** [thread-safe] Memory freeing in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param target Address of the first byte of the previously allocated segment to deallocate
 * @return Whether the whole transaction can continue
 **/
bool tm_free(shared_t unused(shared), tx_t unused(tx), void *unused(target))
{
    // TODO: tm_free(shared_t, tx_t, void*)
    return false;
}
