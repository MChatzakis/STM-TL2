#include "locks.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

/*
    Versioned write spinlock implementation
*/


void versioned_write_spinlock_t_init(versioned_write_spinlock_t *lock)
{
    atomic_init(&lock->lock_and_version, 0);
}

void versioned_write_spinlock_t_destroy(versioned_write_spinlock_t *unused(lock))
{
    return;
}

bool versioned_write_spinlock_t_lock(versioned_write_spinlock_t *lock)
{
    int l = atomic_load(&lock->lock_and_version);

    // Check if lock is already taken
    if (l & 0x1) {
        return false;
    }

    // Try to take lock
    return atomic_compare_exchange_strong(&lock->lock_and_version, &l, l | 0x1);
}

void versioned_write_spinlock_t_unlock(versioned_write_spinlock_t *lock)
{
    atomic_fetch_sub(&lock->lock_and_version, 1); // Remove one to make sure lock is 0
}

void versioned_write_spinlock_t_update_version(versioned_write_spinlock_t *lock, int new_version)
{
    atomic_store(&lock->lock_and_version, new_version << 1);
}

int versioned_write_spinlock_t_load(versioned_write_spinlock_t *lock)
{
    return atomic_load(&lock->lock_and_version);
}

/*
    Global versioned clock implementation
*/

void global_versioned_clock_t_init(global_versioned_clock_t *global_versioned_clock)
{
    atomic_init(&global_versioned_clock->clock, 0);
}

void global_versioned_clock_t_destroy(global_versioned_clock_t *unused(global_versioned_clock))
{
    return;
}

int global_versioned_clock_t_get_clock(global_versioned_clock_t *global_versioned_clock)
{
    return atomic_load(&global_versioned_clock->clock);
}

int global_versioned_clock_t_increment_and_fetch(global_versioned_clock_t *global_versioned_clock)
{
    return atomic_fetch_add(&global_versioned_clock->clock, 1) + 1;
}

/*
    Default lock implementation
*/
int def_lock_t_lock(def_lock_t *lock)
{
    return (pthread_mutex_lock(&lock->mutex) == 0);
}

int def_lock_t_unlock(def_lock_t *lock)
{
    return (pthread_mutex_unlock(&lock->mutex) == 0);
}

int def_lock_t_init(def_lock_t *lock)
{
    return (pthread_mutex_init(&lock->mutex, NULL) == 0);
}

int def_lock_t_destroy(def_lock_t *lock)
{
    return (pthread_mutex_destroy(&lock->mutex) == 0);
}
