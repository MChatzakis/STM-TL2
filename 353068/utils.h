#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "globals.h"
#include "tm_types.h"
#include "rw_sets.h"

/**
 * @brief Structure representing a transaction.
 * 
 */
typedef struct txn
{
    bool is_ro;

    read_set_t *read_set;
    write_set_t *write_set;

    int rv;
    int wv;
} txn_t;

/**
 * @brief Initialize a transaction.
 * 
 * @param is_ro Whether the transaction is read-only.
 * @param rv Read version of the transaction.
 * @param wv Write version of the transaction. (Only used on commit)
 * @return txn_t* Pointer to the initialized transaction.
 */
txn_t *txn_t_init(bool is_ro, int rv, int wv);

/**
 * @brief Destroy a transaction.
 * 
 * @param txn The transaction to destroy.
 */
void txn_t_destroy(txn_t *txn);

/**
 * @brief Get the mapped lock for a given address.
 * 
 * @param locks The array of locks.
 * @param addr The address to get the lock for.
 * @return versioned_write_spinlock_t* The lock for the given address.
 */
versioned_write_spinlock_t *utils_get_mapped_lock(versioned_write_spinlock_t *locks, void *addr);

/**
 * @brief Try to lock a set. Used for the write-set.
 * 
 * @param region The shared memory region.
 * @param set The set to lock.
 * @return true If the set was locked.
 * @return false If the set could not be locked.
 */
bool utils_try_lock_set(region_t *region, set_t *set);

/**
 * @brief Unlock a set. Used for the write-set. It unlocks the locks from start to end.
 * 
 * @param region The shared memory region.
 * @param set The set to unlock.
 * @param start The start node of the set.
 * @param end The end node of the set.
 */
void utils_unlock_set(region_t *region, set_t *set, set_node_t *start, set_node_t *end);

/**
 * @brief Check if a transaction can commit.
 * 
 * @param region The shared memory region.
 * @param txn The transaction to check.
 * @return true If the transaction can commit.
 * @return false If the transaction cannot commit.
 */
bool utils_check_commit(region_t *region, txn_t *txn);

/**
 * @brief Validate a read-set.
 * 
 * @param region The shared memory region.
 * @param set The read-set to validate.
 * @param rv The read-version of the transaction.
 * @return true If the read-set is valid.
 * @return false If the read-set is invalid.
 */
bool utils_validate_read_set(region_t *region, read_set_t *set, int rv);

/**
 * @brief Validate a versioned-write-spinlock.
 * 
 * @param vws The versioned-write-spinlock to validate.
 * @param rv The read-version of the transaction.
 * @return true If the versioned-write-spinlock is valid.
 * @return false If the versioned-write-spinlock is invalid (locked by another thread or version number changed). 
 */
bool utils_validate_versioned_write_spinlock(versioned_write_spinlock_t *vws, int rv);

/**
 * @brief Update the write-set and unlock the locks.
 * 
 * @param region The shared memory region.
 * @param set The write-set to update.
 * @param wv The write-version of the transaction.
*/
void utils_update_and_unlock_write_set(region_t *region, write_set_t *set, int wv);

/**
 * @brief Log a message.
 * 
 * @param color Color of message (not thread-safe). Use COLOR_RESET when called from a thread.
 * @param stream Stream to log to.
 * @param str Message to log
 * @param ... Parameters to log. (printf-like)
 */
void dprint_clog(char *color, FILE *stream, const char *str, ...);

/**
 * @brief Log a warning.
 * 
 * @param color Color of message (not thread-safe). Use COLOR_RESET when called from a thread.
 * @param stream Stream to log to.
 * @param str Message to log as a warning.
 * @param ... Parameters to log. (printf-like)
 */
void dprint_cwarn(char *color, FILE *stream, const char *str, ...);
