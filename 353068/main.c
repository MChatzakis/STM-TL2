# include <stdio.h>
# include <stdbool.h>


# include "../include/tm.h"

// run as make clean , make build-libs run  , gcc ../353068.so  main.c

int main(){

    shared_t stm = tm_create(32, 8);
    void * start = tm_start(stm);

    tx_t t = tm_begin(stm, true);

    int x = 22;
    //tm_write(stm, )

    tm_end(stm, t);

    tm_destroy(stm);

    return 0;
}