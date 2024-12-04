#include "producer.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

int produce(struct producer *self)
{
    return rand() % (self->a - self->b + 1) + self->a;
}

void producer_write_to_file(char *file_name, int item, int status) {
    FILE *file = fopen(file_name, "a");
    if (status == 0)
        fprintf(file, "Unsuccessful production: %d\n", item);
    else
        fprintf(file, "Successful production: %d\n", item);
    fclose(file);
}