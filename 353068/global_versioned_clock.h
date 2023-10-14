#pragma once

#include <stdatomic.h>
#include <stdbool.h>

typedef struct global_versioned_clock{
    _Atomic int clock;
} global_versioned_clock_t;

void global_versioned_clock_t_init(global_versioned_clock_t *global_versioned_clock);
void global_versioned_clock_t_destroy(global_versioned_clock_t *global_versioned_clock);
int global_versioned_clock_t_get_clock(global_versioned_clock_t *global_versioned_clock);
