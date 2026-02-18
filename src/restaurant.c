/*
Created by atsid on 11/9/2025.
*/


#include "restaurant.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

static restaurant_t * global_restaurant = NULL;

/*circular queue*/
static int *waiting_queue = NULL;
static int queue_front = 0;
static int queue_rear = 0;
static int queue_size = 0;
static int queue_capacity = 0;


static void enqueue(int group_id) {
    if (queue_size == queue_capacity) {
        fprintf(stderr, "Queue full\n");
        return;
    }
    waiting_queue[queue_rear] = group_id;
    queue_rear = (queue_rear + 1) % queue_capacity;
    queue_size++;
}

static int dequeue() {
    if (queue_size > 0) {
        int group_id = waiting_queue[queue_front];
        queue_front = (queue_front + 1) % queue_capacity;
        queue_size--;
        return group_id;
    }
    return -1;
}

static int peek_front() {
    if (queue_size > 0) {
        return waiting_queue[queue_front];
    }
    return -1;
}

void restaurant_init(restaurant_t *r, int num_tables, int num_groups, int table_capacity) {
    int i;
    r->num_tables = num_tables;
    r->num_groups = num_groups;
    r->table_capacity = table_capacity;
    r->groups_remaining = num_groups;

    r->tables = calloc(num_tables, sizeof(table_t));
    r->groups = calloc(num_groups, sizeof(group_t));
    if (!r->tables || !r->groups) {
        perror("calloc");
        exit(1);
    }

    for (i = 0; i < num_tables; ++i) {
        r->tables[i].id = i;
        r->tables[i].capacity = table_capacity;
        r->tables[i].occupied = 0;
        pthread_mutex_init(&r->tables[i].lock, NULL);
    }

    for (i = 0; i < num_groups; ++i) {
        r->groups[i].id = i;
        r->groups[i].size = 0;
        r->groups[i].assigned_table = -1;
        r->groups[i].has_eaten = 0;
        sem_init(&r->groups[i].sem, 0, 0);
    }

    pthread_mutex_init(&r->queue_lock, NULL);
    sem_init(&r->waiter_sem, 0, 0);

    queue_capacity = num_groups;
    waiting_queue = malloc(sizeof(int) * queue_capacity);
    if (!waiting_queue) {
        perror("malloc"); exit(1);
    }
    queue_front = 0;
    queue_rear = 0;
    queue_size = 0;
    global_restaurant = r;

    srand((unsigned)time(NULL));
}


void restaurant_destroy(restaurant_t *r) {
    int i;
    for (i = 0; i < r->num_tables; ++i) {
        pthread_mutex_destroy(&r->tables[i].lock);
    }
    for (i = 0; i < r->num_groups; ++i) {
        sem_destroy(&r->groups[i].sem);
    }
    pthread_mutex_destroy(&r->queue_lock);
    sem_destroy(&r->waiter_sem);

    free(r->tables);
    free(r->groups);
    free(waiting_queue);
    waiting_queue = NULL;
    queue_capacity = queue_size = queue_front = queue_rear = 0;
}

int find_table_for_group(restaurant_t *r, int group_size) {
    int i;
    for (i = 0; i < r->num_tables; ++i) {
        pthread_mutex_lock(&r->tables[i].lock);
        int free_space = r->tables[i].capacity - r->tables[i].occupied;
        pthread_mutex_unlock(&r->tables[i].lock);
        if (free_space >= group_size) {
            return i;
        }
    }

    return -1;
}

void seat_group_on_table(restaurant_t *r, int group_id, int table_id) {
    pthread_mutex_lock(&r->tables[table_id].lock);
    r->tables[table_id].occupied += r->groups[group_id].size;
    pthread_mutex_unlock(&r->tables[table_id].lock);

    r->groups[group_id].assigned_table = table_id;
    sem_post(&r->groups[group_id].sem);
}

void leave_table(restaurant_t *r, int group_id) {
    int t = r->groups[group_id].assigned_table;
    if (t < 0) {
        return;
    }
    pthread_mutex_lock(&r->tables[t].lock);
    r->tables[t].occupied -= r->groups[group_id].size;
    pthread_mutex_unlock(&r->tables[t].lock);

    r->groups[group_id].assigned_table = -1;
    r->groups[group_id].has_eaten = 1;

    /*update groups count*/
    pthread_mutex_lock(&r->queue_lock);
    r->groups_remaining--;
    pthread_mutex_unlock(&r->queue_lock);

    sem_post(&r->waiter_sem);
}

void *group_thread_fn(void *arg) {
    int gid = *(int *)arg;
    free(arg);

    if (!global_restaurant) {
        fprintf(stderr, "Global restaurant not initialized!\n");
        return NULL;
    }

    restaurant_t *r = global_restaurant;

    /*arrival with random size*/
    int group_size = (rand() % r->table_capacity) + 1;
    r->groups[gid].size = group_size;

    printf("[Group %d] (size: %d) arrived at the restaurant\n", gid, group_size);

    /*add to queue*/
    pthread_mutex_lock(&r->queue_lock);
    enqueue(gid);
    pthread_mutex_unlock(&r->queue_lock);

    /*notify waiter that group arrived*/
    sem_post(&r->waiter_sem);

    /*wait to be seated*/
    sem_wait(&r->groups[gid].sem);

    /*ocupied tables*/
    int ocupied_tables = 0;

    int i;

    for (i = 0; i < r->num_tables; ++i) {
        pthread_mutex_lock(&r->tables[i].lock);
        if (r->tables[i].occupied > 0) {
            ocupied_tables++;
        }
        pthread_mutex_unlock(&r->tables[i].lock);
    }

    /*start eating*/
    printf("[Waiter] assigned [Group %d] [size: %d] at table %d (%d/%d occupied)\n",
           gid,group_size , r->groups[gid].assigned_table, ocupied_tables, r->num_tables);

    /*simulate eating time*/
    int eat_time = 5 + (rand() % 11);
    sleep(eat_time);

    printf("[Group %d] finished eating after %d seconds\n", gid, eat_time);

    /*leave table*/
    leave_table(r, gid);

    return NULL;
}

void *waiter_thread_fn(void *arg) {
    int i;
    restaurant_t *r = global_restaurant;
    if (!r) {
        return NULL;
    }
    while (1) {
        /*check if all have eaten*/
        pthread_mutex_lock(&r->queue_lock);
        int remaining = r->groups_remaining;
        pthread_mutex_unlock(&r->queue_lock);

        if (remaining <= 0) {
            printf("[Waiter] All groups have been served\n");
            break;
        }

        int found_group = 0;

        pthread_mutex_lock(&r->queue_lock);

        /*check each group in queue to see if we can seat them*/
        for (i = 0; i < queue_size; i++) {
            int current_index = (queue_front + i) % queue_capacity;
            int group_id = waiting_queue[current_index];

            /*skip if group already seated or finished*/
            if (group_id == -1 || r->groups[group_id].assigned_table != -1) {
                continue;
            }

            int table_id = find_table_for_group(r, r->groups[group_id].size);
            if (table_id != -1) {
                /*found a table for this group*/
                seat_group_on_table(r, group_id, table_id);

                /*remove from queue by marking as processed*/
                waiting_queue[current_index] = -1;
                found_group = 1;
                break;
            }
        }

        pthread_mutex_unlock(&r->queue_lock);

        if (!found_group) {
            /*wait for notification (new group arrived or table freed up)*/
            sem_wait(&r->waiter_sem);
        }
    }

    return NULL;
}