#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "globals.h"
#include "tm_types.h"
#include "rw_sets.h"
#include "txn.h"

versioned_write_spinlock_t *utils_get_mapped_lock(versioned_write_spinlock_t *locks, void *addr);
bool utils_try_lock_set(region_t *region, set_t *set);
void utils_unlock_set(region_t *region, set_t *set, set_node_t *start, set_node_t *end);
bool utils_check_commit(region_t *region, txn_t *txn);
bool utils_validate_versioned_write_spinlock(versioned_write_spinlock_t *vws, int rv);
