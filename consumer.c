#include "consumer.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

int consume(struct consumer *self)
{
    return rand() % (self->d - self->c + 1) + self->c;
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    if (argc != 4)
    {
        printf("Usage: %s <number of consumers> <begin> <end>\n", argv[0]);
        return 1;
    }
    int m = atoi(argv[1]);
    int c = atoi(argv[2]);
    int d = atoi(argv[3]);
    
    while (1) {
        for (int i = 0; i < m; i++) {
            struct consumer consumer = {c, d};
            printf("Consumer %d: %d\n", i, consume(&consumer));
        }
        sleep(1);
    }
    return 0;
}