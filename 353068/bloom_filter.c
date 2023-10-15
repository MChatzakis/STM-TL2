#include "bloom_filter.h"
#include <string.h>
bloom_filter_t *bloom_filter_create()
{
    bloom_filter_t *bf = (bloom_filter_t *)malloc(sizeof(bloom_filter_t));
    if (!bf)
    {
        return NULL;
    }

    memset(bf->filter, 0, sizeof(bool) * BLOOM_FILTER_SIZE); // is sizeof(bool ok??)
    return bf;
}

void bloom_filter_destroy(bloom_filter_t *bloom_filter)
{
    // filter is statically allocated, no need for free
    free(bloom_filter);
}

void bloom_filter_add(bloom_filter_t *bloom_filter, uintptr_t address)
{
    int index = address % BLOOM_FILTER_SIZE;
    bloom_filter->filter[index] = true;
}

bool bloom_filter_contains(bloom_filter_t *bloom_filter, uintptr_t address)
{
    return bloom_filter->filter[address % BLOOM_FILTER_SIZE];
}