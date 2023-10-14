#pragma once

#include <stdatomic.h>
#include <stdbool.h>

#define LOCKED true
#define UNLOCKED false

typedef struct versioned_write_spinlock
{
    _Atomic bool lock; // When lock==false, the lock is free. When lock==true, the lock is held.
    int version;
} versioned_write_spinlock_t;

void versioned_write_spinlock_t_init(versioned_write_spinlock_t *lock);
void versioned_write_spinlock_t_destroy(versioned_write_spinlock_t *lock);
void versioned_write_spinlock_t_lock(versioned_write_spinlock_t *lock);
void versioned_write_spinlock_t_unlock(versioned_write_spinlock_t *lock);
