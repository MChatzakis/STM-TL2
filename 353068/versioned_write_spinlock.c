#include "versioned_write_spinlock.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

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

