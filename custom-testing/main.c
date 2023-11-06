#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#include "../include/tm.h"

// run as make clean , make build-libs run  , gcc ../353068.so  main.c

void simple_write_read()
{
    shared_t stm = tm_create(8, 8);
    void *start = tm_start(stm);

    // Transaction 1
    tx_t t1 = tm_begin(stm, false);
    int val1 = 22;
    tm_write(stm, t1, (void *)&val1, 8, start);

    tm_end(stm, t1);

    // Transaction 2
    tx_t t2 = tm_begin(stm, false);
    int read_val;
    tm_read(stm, t2, start, 8, (void *)&read_val);
    tm_end(stm, t2);

    printf("[MAIN] Read result: %d\n", read_val);

    tm_destroy(stm);
}

void test_alloc()
{
    shared_t stm = tm_create(8, 8);

    tx_t t1 = tm_begin(stm, false);
    void *alloc = NULL;
    tm_alloc(stm, t1, 8, &alloc);

    printf("New alloc address: %lu\n",(uintptr_t) alloc);

    tm_free(stm, t1, alloc);

    tm_end(stm, t1);
}

typedef struct
{
    shared_t stm;
} stm_thread_data;

void * f1(void *arg)
{
    stm_thread_data *t_data = (stm_thread_data *)arg;

    void *start = tm_start(t_data->stm);
    tx_t t = tm_begin(t_data->stm, false);

    int val1 = 22;
    tm_write(t_data->stm, t, (void *)&val1, 8, start);

    printf("\n");

    int val2 = 32;
    tm_write(t_data->stm, t, (void *)&val2, 8, start);

    printf("\n");

    int val3 = 42;
    tm_write(t_data->stm, t, (void *)&val3, 8, start);

    printf("\n");

    tm_end(t_data->stm, t);

    pthread_exit(NULL);
}

void * f2(void *arg)
{
    stm_thread_data *t_data = (stm_thread_data *)arg;

    void *start = tm_start(t_data->stm);
    tx_t t = tm_begin(t_data->stm, false);

    int val1;
    //tm_write(t_data->stm, t, (void *)&val1, 8, start);
    tm_read(t_data->stm, t, start, 8, (void *)&val1);

    printf("\n");

    int val2;
    tm_read(t_data->stm, t, start, 8, (void *)&val2);

    printf("\n");

    int val3;
    tm_read(t_data->stm, t, start, 8, (void *)&val3);

    printf("\n");

    tm_end(t_data->stm, t);

    pthread_exit(NULL);
}

void conc_test(){
    shared_t stm = tm_create(8, 8);

    stm_thread_data tdata;
    tdata.stm = stm;

    pthread_t t1,t2;
    pthread_create(&t1, NULL, f1, &tdata);
    pthread_create(&t2, NULL, f2, &tdata);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    tm_destroy(stm);
}

int main()
{

    //simple_write_read();
    //test_alloc();
    conc_test();

    return 0;
}