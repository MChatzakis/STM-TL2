#pragma once

#include "globals.h"
#include "tm_types.h"
#include "rw_sets.h"

versioned_write_spinlock_t *get_mapped_lock(versioned_write_spinlock_t *locks, void *addr);
bool try_lock_set(region_t *region, set_t *set);
