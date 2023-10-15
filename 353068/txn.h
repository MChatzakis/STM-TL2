#pragma once

#include <stdlib.h>
#include <stdbool.h>

#include "globals.h"
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
