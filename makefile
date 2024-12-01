CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lm

all: consumer producer

consumer: consumer.o
	$(CC) $(CFLAGS) -o consumer consumer.o $(LDFLAGS)

producer: producer.o
	$(CC) $(CFLAGS) -o producer producer.o $(LDFLAGS)

consumer.o: consumer.c consumer.h
	$(CC) $(CFLAGS) -c consumer.c -o consumer.o

producer.o: producer.c producer.h
	$(CC) $(CFLAGS) -c producer.c -o producer.o

clean:
	rm -f *.o consumer producer

