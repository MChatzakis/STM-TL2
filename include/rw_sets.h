#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "globals.h"

/**
 * @brief Struct representing a node in a set.
 * 
 * Some fields are only used for write sets.
 * 
 */
typedef struct set_node
{
    void *val;   // unused for read sets
    size_t size; // unused for read sets

    void *addr;

    struct set_node *next;
} set_node_t;

/**
 * @brief Struct representing an ordered set.
 * 
 */
typedef struct set
{
    set_node_t *head;
    set_node_t *tail;
} set_t;

typedef set_t read_set_t;
typedef set_t write_set_t;

/**
 * @brief Initialize a new set.
 * 
 * @return set_t* Pointer to the newly initialized set
 */
set_t *set_t_init();

/**
 * @brief Destroy a set.
 * 
 * @param set Pointer to the set to destroy
 */
void set_t_destroy(set_t *set/*, bool is_write_set*/);

/**
 * @brief Add an element to a set.
 * 
 * @param set Pointer to the set to add to
 * @param addr Address of the element to add
 * @param val Value of the element to add
 * @param size Size of the element to add
 * @return true If the element was added successfully
 * @return false If the element was not added successfully (in case of an error)
 */
bool set_t_add(set_t *set, void *addr, void *val, size_t size);

/**
 * @brief Add or update an element in a set.
 * 
 * @param set Pointer to the set to add to
 * @param addr Address of the element to add
 * @param val Value of the element to add (NULL if the set is a read set)
 * @param size Size of the element to add (0 if the set is a read set)
 * @return true If the element was added successfully
 * @return false If the element was not added successfully (in case of an error)
 */
bool set_t_add_or_update(set_t *set, void *addr, void *val, size_t size);

/**
 * @brief Get the value of an element in a set.
 * 
 * This is used only for write sets in the implementation.
 * 
 * @param set Pointer to the set to get the value from
 * @param addr Address of the element to get the value of
 * @return void* Pointer to the value of the element (NULL when not found)
 */
void *set_t_get_val_or_null(set_t *set, void *addr);