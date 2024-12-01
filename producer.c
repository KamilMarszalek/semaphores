#include "producer.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

int produce(struct producer *self)
{
    return rand() % (self->a - self->b + 1) + self->a;
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    if (argc != 4)
    {
        printf("Usage: %s <number of producers> <begin> <end>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
    int c = atoi(argv[2]);
    int d = atoi(argv[3]);
    
    while (1) {
        for (int i = 0; i < n; i++) {
            struct producer producer = {c, d};
            printf("Producer %d: %d\n", i, produce(&producer));
        }
        sleep(1);
    }
    return 0;
}