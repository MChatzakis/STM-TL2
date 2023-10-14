#pragma once

#include <tm.h>
#include <macros.h>

#include "./deflock.h"

#define VWSL_NUM 100

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
    //global_versioned_clock_t *global_versioned_clock;
    //versioned_write_spinlock_t versioned_write_spinlock[VWSL_NUM]; 

    void *start;

    size_t size;
    size_t align;

    def_lock_t segment_list_lock;
    segment_list *allocs;
} region_t;


