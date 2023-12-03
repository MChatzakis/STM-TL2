#include "global_versioned_clock.h"

void global_versioned_clock_t_init(global_versioned_clock_t *global_versioned_clock)
{
    atomic_init(&global_versioned_clock->clock, 0);
}

void global_versioned_clock_t_destroy(global_versioned_clock_t *unused(global_versioned_clock))
{
    return;
}

int global_versioned_clock_t_get_clock(global_versioned_clock_t *global_versioned_clock)
{
    return atomic_load(&global_versioned_clock->clock);
}

int global_versioned_clock_t_increment_and_fetch(global_versioned_clock_t *global_versioned_clock)
{
    return atomic_fetch_add(&global_versioned_clock->clock, 1) + 1;
}
