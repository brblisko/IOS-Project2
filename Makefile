.PHONY: all semaphore clean

all: semaphore
	gcc main.o -std=gnu99 -pthread -lrt -Wall -Wextra -Werror -pedantic -o proj2

semaphore: main.c
	gcc -c main.c

clean:
	rm -rf main.o proj2 proj2.out
