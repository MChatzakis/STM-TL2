#pragma once

typedef struct global_versioned_lock{
    _Atomic int clock;
} global_versioned_lock_t;

