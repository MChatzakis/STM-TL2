#include "txn.h"

txn_t *txn_t_init(bool is_ro){
    txn_t * txn = (txn_t *) malloc(sizeof(txn_t));
    if(!txn){
        return NULL;
    }

    txn->is_ro = is_ro;
    txn->read_set = set_t_init();
    if(!txn->read_set){
        free(txn);
        return NULL;
    }

    txn->write_set = set_t_init();
    if(!txn->write_set){
        set_t_destroy(txn->read_set);
        free(txn);
        return NULL;
    }

    txn->rv = -1;
    txn->wr = -1;

    return txn;
}

void txn_t_destroy(txn_t *txn){
    set_t_destroy(txn->read_set);
    set_t_destroy(txn->write_set);

    free(txn);
}