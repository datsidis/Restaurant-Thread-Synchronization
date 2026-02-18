/*
Created by atsid on 11/9/2025.
*/

#ifndef RESTAURANT_H
#define RESTAURANT_H

#include <pthread.h>
#include <semaphore.h>

typedef struct {
    int id;
    int capacity;
    int occupied;
    pthread_mutex_t lock;
} table_t;

typedef struct {
    int id;
    int size;
    int assigned_table;
    sem_t sem;
    int has_eaten;
} group_t;

typedef struct {
    table_t *tables;
    group_t *groups;
    int num_tables;
    int num_groups;
    int table_capacity;
    pthread_mutex_t queue_lock;
    sem_t waiter_sem;
    int groups_remaining;
} restaurant_t;

void restaurant_init(restaurant_t *r, int num_tables, int num_groups, int table_capacity);
void restaurant_destroy(restaurant_t *r);

void *group_thread_fn(void *arg);
void *waiter_thread_fn(void *arg);

int find_table_for_group(restaurant_t *r, int group_size);
void seat_group_on_table(restaurant_t *r, int group_id, int table_id);
void leave_table(restaurant_t *r, int group_id);

#endif /* RESTAURANT_H */