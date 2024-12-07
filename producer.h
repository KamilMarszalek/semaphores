#ifndef PRODUCER_H
#define PRODUCER_H
struct producer
{
    int a; //begin of range
    int b; //end of range
};

int produce(struct producer *self);
void producer_write_to_file(char* file_name, int item, int status, int state_of_store);
void producer_write_prod_info(char* file_name, int item);
#endif