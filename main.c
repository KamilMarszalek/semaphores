#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stddef.h>
#include <semaphore.h>
#include "consumer.h"  
#include "producer.h"


int someone_waits_on_producer = 0;
int someone_waits_on_consumer = 0;

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
        sem_init(&self->producer_ready, 0, 0) != 0 || 
        sem_init(&self->consumer_ready, 0, 0) != 0) {
        perror("Failed to initialize semaphores");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "0\n");
    fclose(file);
}

int read_store_state(const char* file_name) {
    FILE *file = fopen(file_name, "r");
    int state;
    fscanf(file, "%d", &state);
    fclose(file);
    return state;
}

void write_store_state(const char* file_name, int state) {
    FILE *file = fopen(file_name, "w");
    fprintf(file, "%d\n", state);
    fflush(file);
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
        sem_wait(&store->mutex);
        int taken;
        taken = read_store_state(store->file);
        printf("Producer %d: tries to load: %d\n", thread_index, item);
        sleep(timeout);
        if (store->size - taken < item || taken > store->size / 2) {
            printf("Producer %d: failed to load %d || Amount in store: %d\n", thread_index, item, taken);
            sleep(timeout);
            if (taken > store->size / 2) {
                if (someone_waits_on_consumer > 0) {
                    printf("post status: %d consumer: %d producer: %d\n", sem_post(&store->consumer_ready), someone_waits_on_consumer, someone_waits_on_producer);
                } else {
                    printf("post status: %d consumer: %d producer: %d\n", sem_post(&store->mutex), someone_waits_on_consumer, someone_waits_on_producer);
                }
            } else {
                if (someone_waits_on_producer > 0) {
                    printf("post status: %d consumer: %d producer: %d\n", sem_post(&store -> producer_ready), someone_waits_on_consumer, someone_waits_on_producer);
                } else {
                    printf("post status: %d consumer: %d producer: %d\n", sem_post(&store->mutex), someone_waits_on_consumer, someone_waits_on_producer);
                }
            }
            producer_write_to_file(log_file_name, item, saved, taken);
            someone_waits_on_producer += 1;
            sem_wait(&store->producer_ready);
            someone_waits_on_producer -= 1;
            taken = read_store_state(store->file);
            if (store->size - taken < item) {
                sem_post(&store->mutex);
                continue;
            }
        }
        taken = read_store_state(store->file);
        saved = 1;
        write_store_state(store->file, taken + item);
        printf("Producer %d: loaded %d || Amount in store: %d\n", thread_index, item, taken + item);
        printf("post status: %d\n", sem_post(&store->mutex));
        sleep(timeout);
        producer_write_to_file(log_file_name, item, saved, taken + item);
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

        sem_wait(&store->mutex);
        int taken;
        taken = read_store_state(store->file);
        printf("Consumer %d: tries to consume: %d\n", thread_index, to_be_consumed);
        sleep(timeout);
        if (taken < to_be_consumed || taken <= store->size / 2) {
            printf("Consumer %d: failed to consume %d || Amount in store: %d\n", thread_index, to_be_consumed, taken);
            sleep(timeout);
            if (taken > store->size / 2) {
                if (someone_waits_on_consumer > 0) {
                    printf("post status: %d consumer: %d producer: %d\n", sem_post(&store->consumer_ready), someone_waits_on_consumer, someone_waits_on_producer);
                } else {
                    printf("post status: %d consumer: %d producer: %d\n", sem_post(&store->mutex), someone_waits_on_consumer, someone_waits_on_producer);
                }
            } else {
                if (someone_waits_on_producer > 0) {
                    printf("post status: %d consumer: %d producer: %d\n", sem_post(&store -> producer_ready), someone_waits_on_consumer, someone_waits_on_producer);
                } else {
                    printf("post status: %d consumer: %d producer: %d\n", sem_post(&store->mutex), someone_waits_on_consumer, someone_waits_on_producer);
                }
            }
            consumer_write_to_file(log_file_name, to_be_consumed, saved, taken);
            someone_waits_on_consumer += 1;
            sem_wait(&store->consumer_ready);
            someone_waits_on_consumer -= 1;
            taken = read_store_state(store->file);
            if (taken < to_be_consumed) {
                sem_post(&store->mutex);
                continue;
            }
        }
        taken = read_store_state(store->file);
        saved = 1;
        write_store_state(store->file, taken - to_be_consumed);
        printf("Consumer %d: consumes %d || Amount in store: %d\n", thread_index, to_be_consumed, taken - to_be_consumed);
        sleep(timeout);
        printf("post status: %d\n", sem_post(&store->mutex));
        sleep(timeout);
        consumer_write_to_file(log_file_name, to_be_consumed, saved, taken - to_be_consumed);
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