#pragma once


typedef struct tm_word{
    char modified;
    char valid_copy;
    
    int write_set;

    void* copyA;
    void* copyB;
}word_t;

