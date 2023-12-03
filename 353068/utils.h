#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "globals.h"
#include "tm_types.h"
#include "rw_sets.h"

typedef struct txn
{
    bool is_ro;

    read_set_t *read_set;
    write_set_t *write_set;

    int rv;
    int wv;
} txn_t;

txn_t *txn_t_init(bool is_ro, int rv, int wv);
void txn_t_destroy(txn_t *txn);

versioned_write_spinlock_t *utils_get_mapped_lock(versioned_write_spinlock_t *locks, void *addr);
bool utils_try_lock_set(region_t *region, set_t *set);
void utils_unlock_set(region_t *region, set_t *set, set_node_t *start, set_node_t *end);
bool utils_check_commit(region_t *region, txn_t *txn);
bool utils_validate_read_set(region_t *region, read_set_t *set, int rv);
bool utils_validate_versioned_write_spinlock(versioned_write_spinlock_t *vws, int rv);
void utils_update_and_unlock_write_set(region_t *region, write_set_t *set, int wv);
void utils_segment_list_insert(region_t *region, segment_t *sn);
void utils_segment_list_remove(region_t *region, segment_t *sn);
