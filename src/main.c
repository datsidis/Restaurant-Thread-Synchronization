#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "restaurant.h"


int main() {
    int i;
    restaurant_t restaurant;
    int num_tables = 2;
    int num_groups = 4;
    int table_capacity = 6;

    restaurant_init(&restaurant, num_tables, num_groups, table_capacity);

    pthread_t waiter_thread;
    pthread_t group_threads[num_groups];

    /*waiter thread*/
    pthread_create(&waiter_thread, NULL, waiter_thread_fn, NULL);

    /*group threads*/
    for (i = 0; i < num_groups; i++) {
        int *gid = malloc(sizeof(int));
        *gid = i;
        pthread_create(&group_threads[i], NULL, group_thread_fn, gid);

        /*group arrivals*/
        sleep(1 + (rand() % 3));
    }

    /*wait for roups to finish*/
    for (i = 0; i < num_groups; i++) {
        pthread_join(group_threads[i], NULL);
    }

    /*wait for waiter to finish*/
    pthread_join(waiter_thread, NULL);

    restaurant_destroy(&restaurant);
    printf("All groups have left. Closing restaurant.\n");

    return 0;
}