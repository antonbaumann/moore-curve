CC=gcc
CFLAGS=-O3

.PHONY: clean
all: moore
moore: main.c
	$(CC) $(CFLAGS) -o $@ $^
clean:
	rm -f moore
