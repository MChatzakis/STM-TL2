#pragma once

#include <tm.h>
#include <batcher.h>
#include <lock.h>
#include <macros.h>
#include <word.h>

/**
 * @brief Segment of dynamically allocated memory.
 *
 */
typedef struct segment
{
    struct segment *prev;
    struct segment *next;
    long words_num;
    // uint8_t segment[] // segment of dynamic size
} segment_t;

/**
 * @brief Struct representing a transactional shared-memory region.
 *
 */
typedef struct region
{
    void *start;

    size_t size;
    size_t align;

    segment_t *allocs;

    batcher_t batcher;

    tx_t tx_count;
    bool *tx_type; // 0 for read only, 1 for read write

} region_t;


