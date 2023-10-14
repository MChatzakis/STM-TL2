#include "versioned_write_spinlock.h"
#include <stdatomic.h>
#include <stdbool.h>

void versioned_write_spinlock_t_init(versioned_write_spinlock_t *lock)
{
    atomic_init(&lock->lock, UNLOCKED);
    lock->version = 0;
}

void versioned_write_spinlock_t_destroy(versioned_write_spinlock_t *lock)
{
    return;
}

void versioned_write_spinlock_t_lock(versioned_write_spinlock_t *lock)
{
    bool expected = UNLOCKED;
    while (atomic_exchange(&lock->lock, true) != expected)
    {
        // Implement back off HERE
    }

    return LOCKED;    
}

void versioned_write_spinlock_t_unlock_and_update(versioned_write_spinlock_t *lock)
{
    lock->version++;
    atomic_store(&lock->lock, UNLOCKED);
} 

void versioned_write_spinlock_t_unlock_and_update(versioned_write_spinlock_t *lock)
{
    atomic_store(&lock->lock, UNLOCKED);
}

bool versioned_write_spinlock_t_get_state(versioned_write_spinlock_t *lock){
    return atomic_load(&lock->lock);
}

int versioned_write_spinlock_t_get_version(versioned_write_spinlock_t *lock){
    return lock->version;
}




