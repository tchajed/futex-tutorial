#include "mutex.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void mutex_ops(mutex_t *m) {
  mutex_lock(m);
  mutex_unlock(m);
  mutex_lock(m);
  mutex_unlock(m);
  mutex_lock(m);
  mutex_unlock(m);
  mutex_lock(m);
  mutex_unlock(m);
  mutex_lock(m);
  mutex_unlock(m);
}

double time_iters(uint64_t iters) {
  mutex_t *m = new_mutex();

  clock_t start = clock();
  for (int i = 0; i < iters; i++) {
    mutex_ops(m);
  }
  double time_s = (clock() - start) / (double)CLOCKS_PER_SEC;

  free(m);

  return time_s;
}

uint64_t min(uint64_t n, uint64_t m) { return n < m ? n : m; }
uint64_t max(uint64_t n, uint64_t m) { return n < m ? m : n; }

// run time_iters till it takes at least a second
double search_time_iters(uint64_t *iters) {
  uint64_t n = 100000;
  double time_s = time_iters(n);
  while (time_s < 1) {
    uint64_t last = n;
    // follow the growth algorithm in
    // https://cs.opensource.google/go/go/+/refs/tags/go1.18.2:src/testing/benchmark.go
    n = (uint64_t)(last / time_s);
    n += n / 5;
    n = min(n, 100 * last);
    n = max(n, last + 1);
    time_s = time_iters(n);
  }
  // each run counts for 5 unrolled iterations
  *iters = n * 5;
  return time_s;
}

int main() {
  uint64_t iters;
  double time_s = search_time_iters(&iters);
  printf("%0.2f ns/acquire\n", time_s / iters * 1e9);
}
