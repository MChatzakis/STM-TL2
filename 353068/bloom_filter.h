#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "globals.h"

#define BLOOM_FILTER_SIZE 1000

typedef struct
{
    bool filter[BLOOM_FILTER_SIZE];
} bloom_filter_t;

bloom_filter_t *bloom_filter_create();
void bloom_filter_destroy(bloom_filter_t *bloom_filter);

void bloom_filter_add(bloom_filter_t *bloom_filter, uintptr_t address);
bool bloom_filter_contains(bloom_filter_t *bloom_filter, uintptr_t address);
