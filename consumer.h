#ifndef CONSUMER_H
#define CONSUMER_H
struct consumer
{
    int c; //begin of range
    int d; //end of range
};

int consume(struct consumer *self);
void consumer_write_to_file(char* file_name, int item, int status);
#endif
