#pragma once

#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

#include "macros.h"
#include "globals.h"

/**
 * @brief Represents a versioned write spinlock, as described in the TL2 paper.
 * It stores the lock state and version
 * 32bits: 1 bit for lock state, 31 bits for version: [-------version_bits--------,-lock_bit-]
 */
typedef struct versioned_write_spinlock
{
    _Atomic int lock_and_version;
} versioned_write_spinlock_t;

/**
 * @brief Initialize a versioned write spinlock with 0.
 * 
 * @param lock The lock to initialize.
 */
void versioned_write_spinlock_t_init(versioned_write_spinlock_t *lock);

/**
 * @brief Destroy a versioned write spinlock. This is a no-op (since it is an atomic type)
 * It is declared to keep the API consistent with other possible implementations.
 * 
 * @param lock The lock to destroy.
 */
void versioned_write_spinlock_t_destroy(versioned_write_spinlock_t *lock);

/**
 * @brief Try to take a versioned write spinlock. If the lock is already taken, return false.
 * 
 * @param lock The lock to take.
 * @return true Success: lock taken.
 * @return false Failure: lock already taken.
 */
bool versioned_write_spinlock_t_lock(versioned_write_spinlock_t *lock);

/**
 * @brief Unlock a versioned write spinlock. It is necessary to have the lock locked by the calling thread.
 * In case were the lock is not locked by the calling thread, the behavior is undefined for TL2.
 * 
 * @param lock The lock to unlock.
 */
void versioned_write_spinlock_t_unlock(versioned_write_spinlock_t *lock);

/**
 * @brief Load the version of a versioned write spinlock.
 * 
 * @param lock The lock to load the version from.
 * @return int The 32-bit integer holding the lock state and version.
 */
int versioned_write_spinlock_t_load(versioned_write_spinlock_t *lock);

/**
 * @brief Store a new version to a versioned write spinlock and unlock it. Must be called only by the thread that locked the lock. 
 * In the case where the lock is not locked by the calling thread, the behavior is undefined for TL2.
 * 
 * @param lock The lock to update the version and release the lock bit.
 * @param new_version The new version to store.
 */
void versioned_write_spinlock_t_update_version(versioned_write_spinlock_t *lock, int new_version);

/**
 * @brief Atomic integer representing the global versioned clock. 
 * 
 */
typedef struct global_versioned_clock
{
    _Atomic int clock;
} global_versioned_clock_t;

/**
 * @brief Initialize a global versioned clock with 0.
 * 
 * @param global_versioned_clock The global versioned clock to initialize.
 */
void global_versioned_clock_t_init(global_versioned_clock_t *global_versioned_clock);

/**
 * @brief Destroy a global versioned clock. This is a no-op (since it is an atomic type)
 * It is declared to keep the API consistent with other possible implementations.
 * 
 * @param global_versioned_clock The global versioned clock to destroy.
 */
void global_versioned_clock_t_destroy(global_versioned_clock_t *global_versioned_clock);

/**
 * @brief Load the current value of the global versioned clock.
 * 
 * @param global_versioned_clock The global versioned clock to load the value from.
 * @return int The current value of the global versioned clock.
 */
int global_versioned_clock_t_get_clock(global_versioned_clock_t *global_versioned_clock);

/**
 * @brief Perform an atomic increment and fetch on the global versioned clock.
 * 
 * @param global_versioned_clock The global versioned clock to increment and fetch.
 * @return int The value of the global versioned clock after the increment.
 */
int global_versioned_clock_t_increment_and_fetch(global_versioned_clock_t *global_versioned_clock);

/**
 * @brief A default pthread lock. This is used only to add segments to the segment list.
 * It is not used by any other part in TL2.
 */
typedef struct def_lock
{
    pthread_mutex_t mutex;
} def_lock_t;

/**
 * @brief Initialize a default lock.
 * 
 * @param lock The lock to initialize.
 * @return int Whether the initialization was successful.
 */
int def_lock_t_init(def_lock_t *lock);

/**
 * @brief Destroy a default lock.
 * 
 * @param lock The lock to destroy.
 * @return int Whether the destruction was successful.
 */
int def_lock_t_destroy(def_lock_t *lock);

/**
 * @brief Lock a default lock in a blocking manner.
 * 
 * @param lock The lock to lock.
 * @return int Whether the lock was successful.
 */
int def_lock_t_lock(def_lock_t *lock);

/**
 * @brief Unlock a default lock. It is necessary to have the lock locked by the calling thread.
 * 
 * @param lock The lock to unlock.
 * @return int Whether the unlock was successful.
 */
int def_lock_t_unlock(def_lock_t *lock);