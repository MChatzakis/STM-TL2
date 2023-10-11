#include "lock.h"

int lock_t_get(lock_t *lock)
{
    return (pthread_mutex_lock(&lock->mutex) == 0);
}

int lock_t_release(lock_t *lock)
{
    return (pthread_mutex_unlock(&lock->mutex) == 0);
}

int lock_t_init(lock_t *lock)
{
    return (pthread_mutex_init(&lock->mutex, NULL) == 0);
}

int lock_t_destroy(lock_t *lock)
{
    return (pthread_mutex_destroy(&lock->mutex) == 0);
}

int lock_t_condition(pthread_cond_t *cond_var, lock_t *lock)
{
    return (pthread_cond_wait(cond_var, &lock->mutex) == 0);
}

int lock_t_wake_up(pthread_cond_t *cond_var)
{
    return (pthread_cond_broadcast(cond_var) == 0);
}
