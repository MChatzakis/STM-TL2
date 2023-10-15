#pragma once

#include <pthread.h>

#include "globals.h"

typedef struct def_lock
{
    pthread_mutex_t mutex;
} def_lock_t;

int def_lock_t_init(def_lock_t *lock);
int def_lock_t_destroy(def_lock_t *lock);
int def_lock_t_lock(def_lock_t *lock);
int def_lock_t_unlock(def_lock_t *lock);
