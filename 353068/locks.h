#pragma once

#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

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

typedef struct global_versioned_clock
{
    _Atomic int clock;
} global_versioned_clock_t;

void global_versioned_clock_t_init(global_versioned_clock_t *global_versioned_clock);
void global_versioned_clock_t_destroy(global_versioned_clock_t *global_versioned_clock);
int global_versioned_clock_t_get_clock(global_versioned_clock_t *global_versioned_clock);
int global_versioned_clock_t_increment_and_fetch(global_versioned_clock_t *global_versioned_clock);

typedef struct def_lock
{
    pthread_mutex_t mutex;
} def_lock_t;

int def_lock_t_init(def_lock_t *lock);
int def_lock_t_destroy(def_lock_t *lock);
int def_lock_t_lock(def_lock_t *lock);
int def_lock_t_unlock(def_lock_t *lock);