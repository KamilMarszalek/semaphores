#include "producer.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

int produce(struct producer *self) {
    return rand() % (self->a - self->b + 1) + self->a;
}

void producer_write_to_file(char *file_name, int item, int status, int tries) {
    FILE *file = fopen(file_name, "a");
    if (status == 0)
        fprintf(file, "Unsuccessful production: %d Tries: %d\n", item, tries);
    else
        fprintf(file, "Successful production: %d\n", item);
    fclose(file);
}

void producer_write_prod_info(char *file_name, int item) {
    FILE *file = fopen(file_name, "a");
    fprintf(file, "Produced: %d\n", item);
    fclose(file);
}