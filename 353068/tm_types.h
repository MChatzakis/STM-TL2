#pragma once

#include <tm.h>
#include <macros.h>

#include "globals.h"
#include "def_lock.h"
#include "versioned_write_spinlock.h"
#include "global_versioned_clock.h"


/**
 * @brief Segment of dynamically allocated memory.
 *
 */
typedef struct segment
{
    struct segment *prev;
    struct segment *next;
} segment_t;

typedef segment_t * segment_list;

/**
 * @brief Struct representing a transactional shared-memory region.
 *
 */
typedef struct region
{
    global_versioned_clock_t global_versioned_clock;
    versioned_write_spinlock_t versioned_write_spinlock[VWSL_NUM]; 
    def_lock_t segment_list_lock;

    void *start;

    size_t size;
    size_t align;
    
    segment_list *allocs;
} region_t;


