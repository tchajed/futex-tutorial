#include "futex.h"
#include "mutex.h"

#include <stdatomic.h>
#include <stdlib.h>

#define UNLOCKED 0
#define LOCKED_NO_WAIT 1
#define LOCKED_WAIT 2

// implementation based on "Futexes are tricky"
// https://www.akkadia.org/drepper/futex.pdf

mutex_t *new_mutex() {
  mutex_t *m = (mutex_t *)aligned_alloc(4, 4);
  atomic_store(m, UNLOCKED);
  return m;
}

uint32_t atomic_compare_swap(mutex_t *m, uint32_t expected, uint32_t desired) {
  uint32_t v = expected;
  atomic_compare_exchange_strong(m, &v, desired);
  return v;
}

void mutex_lock(mutex_t *m) {
  uint32_t c = UNLOCKED;
  if (atomic_compare_exchange_strong(m, &c, LOCKED_NO_WAIT)) {
    // successfully acquired the lock
    return;
  }
  // someone else holds the lock
  do {
    // bump c up to LOCKED_WAIT, if necessary
    if (c == LOCKED_WAIT ||
        // we're the first waiter, set m to LOCKED_WAIT
        // XXX: the comparison in this line is extremely confusing
        atomic_compare_swap(m, LOCKED_NO_WAIT, LOCKED_WAIT) != UNLOCKED) {
      futex_wait(m, LOCKED_WAIT);
    }
    // subtlety: sets the expected value for the next call to cmpxchg in the
    // while condition
    c = UNLOCKED;
  }
  // change the futex state from UNLOCKED to LOCKED_WAIT (fails if futex is
  // locked in between loop body and this check); we set it to LOCKED_WAIT
  // because we know there are other waiters even after we acquire the lock
  while (!atomic_compare_exchange_weak(m, &c, LOCKED_WAIT));
}

void mutex_unlock(mutex_t *m) {
  // note that subtracting 1 either turns LOCKED_WAIT into LOCKED_NO_WAIT (in
  // which case the body runs because we need to wait up at least one waiter),
  // or it turns LOCKED_NO_WAIT into UNLOCKED (in which case we've already
  // released the lock)
  if (atomic_fetch_sub(m, 1) != LOCKED_NO_WAIT) {
    atomic_store(m, UNLOCKED);
    futex_wake(m, 1);
  }
}
