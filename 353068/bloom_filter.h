#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "globals.h"
#include "dprint.h"


typedef struct
{
    bool filter[BLOOM_FILTER_SIZE];
} bloom_filter_t;

bloom_filter_t *bloom_filter_t_create();
void bloom_filter_t_destroy(bloom_filter_t *bloom_filter);

void bloom_filter_t_add(bloom_filter_t *bloom_filter, uintptr_t address);
bool bloom_filter_t_contains(bloom_filter_t *bloom_filter, uintptr_t address);

void bloom_filter_t_print(bloom_filter_t *bloom_filter);
