#include "versioned_write_spinlock.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

void versioned_write_spinlock_t_init(versioned_write_spinlock_t *lock)
{
    atomic_init(&lock->lock, UNLOCKED);
    //lock->version = 0;
    atomic_init(&lock->version, 0);
}

void versioned_write_spinlock_t_destroy(versioned_write_spinlock_t *unused(lock))
{
    return;
}

bool versioned_write_spinlock_t_lock(versioned_write_spinlock_t *lock)
{
    bool expected = UNLOCKED;
    int attempt = 0;
    while (atomic_exchange(&lock->lock, LOCKED) != expected)
    {
        // Bounding the lock attempts
        attempt++;
        if (attempt == MAX_LOCK_ATTEMPTS)
        {
            return UNLOCKED;
        }

        // Spinning with increasing backoff mechanism (linear backoff)
        for (int i = 0; i < attempt * BACKOFF_FACTOR; i++)
        {
            // do nothing here
            //pthread_yield_np();
        } 
    }

    return LOCKED;
}

void versioned_write_spinlock_t_update_and_unlock(versioned_write_spinlock_t *lock, int new_version)
{
    //lock->version = new_version;
    atomic_store(&lock->version, new_version);
    atomic_store(&lock->lock, UNLOCKED);
}

void versioned_write_spinlock_t_unlock(versioned_write_spinlock_t *lock)
{
    atomic_store(&lock->lock, UNLOCKED);
}

bool versioned_write_spinlock_t_get_state(versioned_write_spinlock_t *lock)
{
    return atomic_load(&lock->lock);
}

int versioned_write_spinlock_t_get_version(versioned_write_spinlock_t *lock)
{
    return atomic_load(&lock->version);
}

