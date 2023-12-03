#pragma once

#include <stdatomic.h>
#include <stdbool.h>
#include "macros.h"
#include "globals.h"

typedef struct versioned_write_spinlock
{
    _Atomic int lock_and_version;
} versioned_write_spinlock_t;

void versioned_write_spinlock_t_init(versioned_write_spinlock_t *lock);
void versioned_write_spinlock_t_destroy(versioned_write_spinlock_t *lock);

bool versioned_write_spinlock_t_lock(versioned_write_spinlock_t *lock);
void versioned_write_spinlock_t_unlock(versioned_write_spinlock_t *lock);

int versioned_write_spinlock_t_load(versioned_write_spinlock_t *lock);
void versioned_write_spinlock_t_update_version(versioned_write_spinlock_t *lock, int new_version);

