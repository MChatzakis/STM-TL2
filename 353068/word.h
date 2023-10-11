#pragma once

#include <stdbool.h>

typedef struct tm_word{
    bool modified;
    bool valid_copy;
    
    tx_t write_set;

    void* copyA;
    void* copyB;
}word_t;

bool word_t_read(word_t* word, void* target, bool is_ro);
bool word_t_write(word_t* word, void* target, bool is_ro);
