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
    // Wait for a futex_wake signal on m. The kernel will immediately bail out
    // if it finds mutex value is not LOCKED, which might occur if between the
    // compare-exchange and futex_wait the mutex is released. Without this
    // feature, lock would wait infinitely for a futex_wait that it has already
    // missed.
    futex_wait(m, LOCKED);
  }
}

void mutex_unlock(mutex_t *m) {
  atomic_store(m, UNLOCKED);
  // Wake up any thread waiting in mutex_lock.
  futex_wake(m, 1);
}
