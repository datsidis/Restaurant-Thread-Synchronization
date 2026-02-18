CC = gcc
CFLAGS = -Wall -Wextra -pthread -g

all: restaurant

restaurant: main.o restaurant.o
	$(CC) $(CFLAGS) main.o restaurant.o -o restaurant

main.o: main.c restaurant.h
	$(CC) $(CFLAGS) -c main.c

restaurant.o: restaurant.c restaurant.h
	$(CC) $(CFLAGS) -c restaurant.c

clean:
	rm -f *.o restaurant
