CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lm

all: main

main: main.o consumer.o producer.o
	$(CC) $(CFLAGS) -o main main.o consumer.o producer.o $(LDFLAGS)

main.o: main.c consumer.h producer.h
	$(CC) $(CFLAGS) -c main.c -o main.o

consumer.o: consumer.c consumer.h
	$(CC) $(CFLAGS) -c consumer.c -o consumer.o

producer.o: producer.c producer.h
	$(CC) $(CFLAGS) -c producer.c -o producer.o

clean:
	rm -f *.o main consumer producer

