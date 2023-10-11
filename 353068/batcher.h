#pragma once

#include <pthread.h>
#include "lock.h"

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


