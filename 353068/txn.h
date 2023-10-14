#pragma once

#include <stdlib.h>
#include <stdbool.h>

#include "rw_sets.h"

typedef struct txn
{
    bool is_ro;

    read_set_t *read_set;
    write_set_t *write_set;

    int rv;
    int wr;
} txn_t;

txn_t *txn_t_init(bool is_ro);
void txn_t_free(txn_t *txn);
