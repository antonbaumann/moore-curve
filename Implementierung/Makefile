CC=gcc
CFLAGS=-O3

.PHONY: clean disable_avx
all: moore
moore: main.c svg.c moore.c benchmark.c moore.S moore_avx.S
	$(CC) $(CFLAGS) -march=native -o $@ $^
disable_avx:
	$(CC) $(CFLAGS) -o moore main.c svg.c moore.c benchmark.c moore.S
clean:
	rm -f moore
