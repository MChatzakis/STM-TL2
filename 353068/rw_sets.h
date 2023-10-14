#pragma once

typedef struct write_set_node
{
    void *addr;
    void *val;
    struct write_set_node *next;
} write_set_node_t;

typedef struct write_set
{
    write_set_node_t *head;
    write_set_node_t *tail;
} write_set_t;

typedef struct read_set_node
{
    void *addr;
    struct read_set_node *next;
} read_set_node_t;

typedef struct read_set
{
    read_set_node_t *head;
    read_set_node_t *tail;
} read_set_t;

