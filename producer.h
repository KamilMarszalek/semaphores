#ifndef PRODUCER_H
#define PRODUCER_H
struct producer
{
    int a; //begin of range
    int b; //end of range
};
int produce(struct producer *self);
#endif