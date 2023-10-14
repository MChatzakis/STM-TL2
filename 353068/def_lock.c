#include "def_lock.h"

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