#include "consumer.h"  
#include "producer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stddef.h>

struct semaphore {
    volatile int value;
};

void semaphore_init(struct semaphore *self, int value) {
    self->value = value;
}

void p(struct semaphore *self) {
    while (self->value <= 0) {
        sleep(1);
    }
    self->value = 0;
}

void v(struct semaphore *self) {
    self->value = 1;
}

struct store {
    char* file;
    int size;
    int begin;
    int end;
    struct semaphore *mutex;
    struct semaphore *producer_ready;
    struct semaphore *consumer_ready;
};

void store_init(struct store* self, char* file_name, int size) {
    FILE *file = fopen(file_name, "w");
    self->file = file_name;
    self->size = size;
    self->begin = 0;
    self->end = 0;
    self->mutex = malloc(sizeof(struct semaphore));
    self->producer_ready = malloc(sizeof(struct semaphore));
    self->consumer_ready = malloc(sizeof(struct semaphore));
    semaphore_init(self->mutex, 1);
    semaphore_init(self->producer_ready, 1);
    semaphore_init(self->consumer_ready, 0);
    fprintf(file, "0\n");
    fclose(file);
}

void* producer_thread(void* arg) {
    void** args = (void**)arg;
    struct store *store = (struct store*)args[0];
    struct producer *producer = (struct producer*)args[1];
    int thread_index = *((int*)args[2]);
    char log_file_name[256];
    snprintf(log_file_name, 256, "producer_%d.txt", thread_index);
    free(args[2]);
    free(args);

    while (1) {
        int success = 0;
        int item = produce(producer);

        p(store->producer_ready);
        p(store->mutex);

        FILE *file = fopen(store->file, "r+");
        int taken;
        fscanf(file, "%d", &taken);
        if (store->size - taken >= item) {
            success = 1;
            fseek(file, 0, SEEK_SET);
            fprintf(file, "%d\n", taken + item);
        }
        fclose(file);

        producer_write_to_file(log_file_name, item, success);

        v(store->mutex);
        if (taken + item >= store->size / 2) {
            v(store->consumer_ready);
        } else {
            v(store->producer_ready);
        }
        sleep(1);
    }
    return NULL;
}

void* consumer_thread(void* arg) {
    void** args = (void**)arg;
    struct store *store = (struct store*)args[0];
    struct consumer *consumer = (struct consumer*)args[1];
    int thread_index = *((int*)args[2]);
    char log_file_name[256];
    snprintf(log_file_name, 256, "consumer_%d.txt", thread_index);
    free(args[2]);
    free(args);
    
    while (1) {
        int success = 0;

        p(store->consumer_ready);
        p(store->mutex);

        FILE* file = fopen(store->file, "r+");
        int to_be_consumed = consume(consumer);
        int taken;
        fscanf(file, "%d", &taken);
        if (taken >= to_be_consumed) {
            success = 1;
            fseek(file, 0, SEEK_SET);
            fprintf(file, "%d\n", taken - to_be_consumed);
        }
        fclose(file);

        consumer_write_to_file(log_file_name, to_be_consumed, success);
        
        v(store->mutex);
        if (taken - to_be_consumed >= store->size / 2) {
            v(store->consumer_ready);
        } else {
            v(store->producer_ready);
        }
        sleep(1);
    }
    return NULL;
}

int main(int argc, char const *argv[]) {
    if (argc != 8) {
        printf("Usage: %s <number of producers> <number of consumers> <begin> <end> <begin> <end> <capacity>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int a = atoi(argv[3]);
    int b = atoi(argv[4]);
    int c = atoi(argv[5]);
    int d = atoi(argv[6]);
    int k = atoi(argv[7]);

    struct store store;
    store_init(&store, "store.txt", k);

    pthread_t producers[n];
    pthread_t consumers[m];

    struct producer prod = {a, b};
    struct consumer cons = {c, d};

    for (int i = 0; i < n; i++) {
        void** args = malloc(3 * sizeof(void*));
        args[0] = &store;
        args[1] = &prod;
        int* thread_index = malloc(sizeof(int));
        *thread_index = i;
        args[2] = thread_index;
        pthread_create(&producers[i], NULL, producer_thread, args);
    }

    for (int i = 0; i < m; i++) {
        void** args = malloc(3 * sizeof(void*));
        args[0] = &store;
        args[1] = &cons;
        int* thread_index = malloc(sizeof(int));
        *thread_index = i;
        args[2] = thread_index;
        pthread_create(&consumers[i], NULL, consumer_thread, args);
    }

    for (int i = 0; i < n; i++) {
        pthread_join(producers[i], NULL);
    }

    for (size_t i = 0; i < m; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    return 0;
}