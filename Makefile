.PHONY: all semaphore clean

all: semaphore
	gcc main.o -std=gnu99 -Wall -Wextra -Werror -pedantic -o proj2

semaphore: main.c
	gcc -c main.c

clean:
	rm -rf main.o proj2