#include "batcher.h"

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
