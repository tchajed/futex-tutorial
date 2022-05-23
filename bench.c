#include "mutex.h"
#include <stdio.h>
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

#define ITERS 1000000

int main() {
  mutex_t *m = new_mutex();
  mutex_ops(m);

  clock_t start = clock();
  for (int i = 0; i < ITERS / 5; i++) {
    mutex_ops(m);
  }
  double time_s = (clock() - start) / (double)CLOCKS_PER_SEC;
  printf("%0.2f us/acquire\n", time_s / ITERS * 1000000);
}
