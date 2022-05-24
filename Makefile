default: test test-better bench bench-better

%.o: %.c
	clang -Wall -c $< -o $@

test: futex.o mutex.o test.o
	clang futex.o mutex.o test.o -o $@

test-better: futex.o mutex_better.o test.o
	clang futex.o mutex_better.o test.o -o $@

bench: futex.o mutex.o bench.o
	clang futex.o mutex.o bench.o -o $@

bench-better: futex.o mutex_better.o bench.o
	clang futex.o mutex_better.o bench.o -o $@
