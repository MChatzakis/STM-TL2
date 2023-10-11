#pragma once

#include <pthread.h>

typedef struct lock
{
    pthread_mutex_t mutex;
} lock_t;

int lock_t_init(lock_t *lock);
int lock_t_destroy(lock_t *lock);
int lock_t_get(lock_t *lock);
int lock_t_release(lock_t *lock);

int lock_t_condition(pthread_cond_t *cond_var, lock_t *lock);
int lock_t_wake_up(pthread_cond_t *cond_var);
