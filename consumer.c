#include "consumer.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

int consume(struct consumer *self) {
    return rand() % (self->d - self->c + 1) + self->c;
}

void consumer_write_to_file(char *file_name, int item, int status, int state_of_store) {
    FILE *file = fopen(file_name, "a");
    if (status == 0) 
        fprintf(file, "Unsuccessful consumption: %d Store: %d\n", item, state_of_store);
    else 
        fprintf(file, "Successful consumption: %d Store: %d\n", item, state_of_store);
    fclose(file);
}

void consumer_write_cons_info(char *file_name, int item) {
    FILE *file = fopen(file_name, "a");
    fprintf(file, "Consumed: %d\n", item);
    fclose(file);
}