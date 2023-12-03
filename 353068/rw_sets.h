#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "globals.h"

/**
 * @brief Represents a node in the write set.
 * 
 */
typedef struct write_set_node
{
    void *val;
    size_t size;

    void *addr;

    struct write_set_node *next;
} write_set_node_t;

/**
 * @brief Struct representing the write set. The set is considered to be ordered.
 * 
 */
typedef struct write_set
{
    write_set_node_t *head;
} write_set_t;

/**
 * @brief Initialize a write set.
 * 
 * @return write_set_t* pointer to the initialized write set.
 */
write_set_t *write_set_t_init();

/**
 * @brief Deallocate a write set.
 * 
 * @param set The set to deallocate.
 */
void write_set_t_destroy(write_set_t *set);

/**
 * @brief Add an address to the write set. The addition keeps the set ordered.
 * 
 * @param set The set to add the address to. 
 * @param addr The address to add.
 * @param val The value to add.
 * @param size The size of the value.
 * @return true If the address was added.
 * @return false If the address could not be added.
 */
bool write_set_t_add(write_set_t *set, void *addr, void *val, size_t size);

/**
 * @brief Get the value for a given address.
 * 
 * @param set The set to get the value from.
 * @param addr The address to get the value for.
 * @return void* The value for the given address. NULL if the address is not in the set.
 */
void *write_set_t_get_val(write_set_t *set, void *addr);

/**
 * @brief Represents a node in the read set.
 * 
 */
typedef struct read_set_node
{
    void *addr;
    struct read_set_node *next;
} read_set_node_t;

/**
 * @brief Struct representing the read set. The set is considered to be unordered.
 * 
 */
typedef struct read_set
{
    read_set_node_t *head;
} read_set_t;

/**
 * @brief Initialize a read set.
 * 
 * @return read_set_t* pointer to the initialized read set.
 */
read_set_t *read_set_t_init();

/**
 * @brief Deallocate a read set.
 * 
 * @param set The set to deallocate.
 */
void read_set_t_destroy(read_set_t *set);

/**
 * @brief Add an address to the read set.
 * 
 * Optimization (not necessary usage): Add duplicates in the set, to skip the set traversal.
 * 
 * @param set The set to add the address to. 
 * @param addr The address to add.
 * @return true If the address was added.
 * @return false If the address could not be added. (Represents an error)
 */
bool read_set_t_add(read_set_t *set, void *addr);
