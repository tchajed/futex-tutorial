#include "mutex.h"
#include "futex.h"

#include <stdatomic.h>
#include <stdlib.h>

#define LOCKED 1
#define UNLOCKED 0

mutex_t *new_mutex() {
  mutex_t *m = (mutex_t *)aligned_alloc(4, sizeof(mutex_t));
  atomic_store(m, UNLOCKED);
  return m;
}

void mutex_lock(mutex_t *m) {
  uint32_t expected = UNLOCKED;
  while (!atomic_compare_exchange_weak(m, &expected, LOCKED)) {
    // wait for an unlock on m, and immediately bail out if mutex is not
    // locked (due to race between cmpxchg and this wait)
    futex_wait(m, LOCKED);
  }
}

void mutex_unlock(mutex_t *m) {
  atomic_store(m, UNLOCKED);
  futex_wake(m, 1);
}
