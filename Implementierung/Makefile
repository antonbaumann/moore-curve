CC=gcc
CFLAGS=-O3 -mavx2 -mavx

.PHONY: clean
all: moore
moore: main.c svg.c moore.c benchmark.c moore.S moore_avx.S
	$(CC) $(CFLAGS) -o $@ $^
clean:
	rm -f moore
