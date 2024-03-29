default: test test-better bench bench-better

CC ?= clang
CFLAGS ?= -Wall -O2

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: futex.o mutex.o test.o
	$(CC) $(CFLAGS)  $^ -o $@

test-better: futex.o mutex_better.o test.o
	$(CC) $(CFLAGS) $^ -o $@

bench: futex.o mutex.o bench.o
	$(CC) $(CFLAGS) $^ -o $@

bench-better: futex.o mutex_better.o bench.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f test test-better bench bench-better
