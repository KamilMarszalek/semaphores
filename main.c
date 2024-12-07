#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stddef.h>
#include <semaphore.h>
#include "consumer.h"  
#include "producer.h"


struct store {
    char* file;
    int size;
    int begin;
    int end;
    sem_t mutex;
    sem_t producer_ready;
    sem_t consumer_ready;
};

void store_init(struct store* self, char* file_name, int size) {
    FILE *file = fopen(file_name, "w");
    self->file = file_name;
    self->size = size;
    self->begin = 0;
    self->end = 0;
    if (sem_init(&self->mutex, 0, 1) != 0 || 
        sem_init(&self->producer_ready, 0, 1) != 0 || 
        sem_init(&self->consumer_ready, 0, 0) != 0) {
        perror("Failed to initialize semaphores");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "0\n");
    fclose(file);
}

void* producer_thread(void* arg) {
    void** args = (void**)arg;
    struct store *store = (struct store*)args[0];
    struct producer *producer = (struct producer*)args[1];
    int thread_index = *((int*)args[2]);
    int timeout = *((int*)args[3]);
    char log_file_name[256];
    snprintf(log_file_name, 256, "producer_%d.txt", thread_index);
    free(args[2]);
    free(args[3]);
    free(args);
    int saved = 0;
    int item = produce(producer);
    producer_write_prod_info(log_file_name, item);

    while (1) {
        if (saved) {
            item = produce(producer);
            producer_write_prod_info(log_file_name, item);
            saved = 0;
        }

        sem_wait(&store->producer_ready);
        sem_wait(&store->mutex);

        FILE *file = fopen(store->file, "r+");
        int taken;
        fscanf(file, "%d", &taken);
        if (store->size - taken >= item) {
            saved = 1;
            fseek(file, 0, SEEK_SET);
            fprintf(file, "%d\n", taken + item);
            printf("Producer %d: loaded %d || Amount in store: %d\n", thread_index, item, taken + item);
        } else {
            printf("Producer %d: failed to load %d || Amount in store: %d\n", thread_index, item, taken);
        }
        fclose(file);
        sleep(timeout);
        sem_post(&store->mutex);
        if (saved) {
            if (taken + item > store->size / 2) {
                sem_post(&store->consumer_ready);
            } else {
                sem_post(&store->producer_ready);
            }
            producer_write_to_file(log_file_name, item, saved, taken + item);
        } else {
            sem_post(&store->producer_ready);
            producer_write_to_file(log_file_name, item, saved, taken);
        }
        sleep(timeout);
    }
    return NULL;
}

void* consumer_thread(void* arg) {
    void** args = (void**)arg;
    struct store *store = (struct store*)args[0];
    struct consumer *consumer = (struct consumer*)args[1];
    int thread_index = *((int*)args[2]);
    int timeout = *((int*)args[3]);
    char log_file_name[256];
    snprintf(log_file_name, 256, "consumer_%d.txt", thread_index);
    free(args[2]);
    free(args[3]);
    free(args);
    int saved = 0;
    int to_be_consumed = consume(consumer);
    consumer_write_cons_info(log_file_name, to_be_consumed);
    
    while (1) {
        if (saved) {
            to_be_consumed = consume(consumer);
            consumer_write_cons_info(log_file_name, to_be_consumed);
            saved = 0;
        }

        sem_wait(&store->consumer_ready);
        sem_wait(&store->mutex);

        FILE* file = fopen(store->file, "r+");

        int taken;
        fscanf(file, "%d", &taken);
        if (taken >= to_be_consumed) {
            saved = 1;
            fseek(file, 0, SEEK_SET);
            fprintf(file, "%d\n", taken - to_be_consumed);
            printf("Consumer %d: consumes %d || Amount in store: %d\n", thread_index, to_be_consumed, taken - to_be_consumed);
        } else {
            printf("Consumer %d: failed to consume %d || Amount in store: %d\n", thread_index, to_be_consumed, taken);
        }
        fclose(file);
        sleep(timeout);
        sem_post(&store->mutex);
        if (saved) {
            if (taken - to_be_consumed < store->size / 2) {
                sem_post(&store->producer_ready);
            } else {
                sem_post(&store->consumer_ready);
            }
            consumer_write_to_file(log_file_name, to_be_consumed, saved, taken - to_be_consumed);
        } else {
            sem_post(&store->consumer_ready);
            consumer_write_to_file(log_file_name, to_be_consumed, saved, taken);
        }
        sleep(timeout);
    }
    return NULL;
}

int main(int argc, char const *argv[]) {
    if (argc != 9) {
        printf("Usage: %s <number of producers> <number of consumers> <begin> <end> <begin> <end> <capacity> <timeout>\n", argv[0]);
        return 1;
    }
    system("rm -f *.txt"); // clear all log files
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int a = atoi(argv[3]);
    int b = atoi(argv[4]);
    int c = atoi(argv[5]);
    int d = atoi(argv[6]);
    int k = atoi(argv[7]);
    int timeout = atoi(argv[8]);

    struct store store;
    store_init(&store, "store.txt", k);

    pthread_t producers[n];
    pthread_t consumers[m];

    struct producer prod = {a, b};
    struct consumer cons = {c, d};

    for (int i = 0; i < n; i++) {
        void** args = malloc(4 * sizeof(void*));
        args[0] = &store;
        args[1] = &prod;
        int* thread_index = malloc(sizeof(int));
        *thread_index = i;
        args[2] = thread_index;
        int* timeout_ptr = malloc(sizeof(int));
        *timeout_ptr = timeout;
        args[3] = timeout_ptr;
        pthread_create(&producers[i], NULL, producer_thread, args);
    }

    for (int i = 0; i < m; i++) {
        void** args = malloc(4 * sizeof(void*));
        args[0] = &store;
        args[1] = &cons;
        int* thread_index = malloc(sizeof(int));
        *thread_index = i;
        args[2] = thread_index;
        int* timeout_ptr = malloc(sizeof(int));
        *timeout_ptr = timeout;
        args[3] = timeout_ptr;
        pthread_create(&consumers[i], NULL, consumer_thread, args);
    }

    for (int i = 0; i < n; i++) {
        pthread_join(producers[i], NULL);
    }

    for (size_t i = 0; i < m; i++) {
        pthread_join(consumers[i], NULL);
    }
    sem_destroy(&store.mutex);
    sem_destroy(&store.producer_ready);
    sem_destroy(&store.consumer_ready);
    return 0;
}