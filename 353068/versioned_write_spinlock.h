#pragma once

typedef struct versioned_write_spinlock
{
    _Atomic bool lock;
    int version;
} versioned_write_spinlock_t;