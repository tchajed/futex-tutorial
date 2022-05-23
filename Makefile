default: test

%.o: %.c
	clang -Wall -c $< -o $@

test: futex.o mutex.o test.o
	clang futex.o mutex.o test.o -o $@

bench: futex.o mutex.o bench.o
	clang futex.o mutex.o bench.o -o $@
