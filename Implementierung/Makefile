CC=gcc
CFLAGS=-O3 -Wall -Wextra -ggdb

.PHONY: clean
all: moore
moore: main.c svg.c moore.c moore.S
	$(CC) $(CFLAGS) -o $@ $^
clean:
	rm -f moore
