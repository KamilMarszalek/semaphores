#include "consumer.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

int consume(struct consumer *self)
{
    return rand() % (self->d - self->c + 1) + self->c;
}

void consumer_write_to_file(char *file_name, int item, int status) {
    FILE *file = fopen(file_name, "a");
    if (status == 0) 
        fprintf(file, "Unsuccessful consumption: %d\n", item);
    else 
        fprintf(file, "Successful consumption: %d\n", item);
    fclose(file);
}