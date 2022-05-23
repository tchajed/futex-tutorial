#include "futex.h"

#include <limits.h>
#include <linux/futex.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

static int futex(uint32_t *uaddr, int futex_op, uint32_t val,
                 const struct timespec *timeout, uint32_t *uaddr2,
                 uint32_t val3) {
  return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

// Sleep until a wake-up on the same futexp. If the futexp does not point to the
// value expect_val, instead fail immediately with EAGAIN.
int futex_wait(futex_t *futexp, uint32_t expect_val) {
  return futex((uint32_t *)futexp, FUTEX_WAIT, expect_val, NULL, NULL, 0);
}

// Wake up threads sleeping in futex_wait, up to a maximum of num_waiters
// threads.
int futex_wake_num(futex_t *futexp, uint32_t num_waiters) {
  return futex((uint32_t *)futexp, FUTEX_WAKE, num_waiters, NULL, NULL, 0);
}

// Wake up the first thread waiting on futexp.
int futex_wake(futex_t *futexp) { return futex_wake_num(futexp, 1); }

// Wake up all threads waiting on futexp.
int futex_wake_all(futex_t *futexp) { return futex_wake_num(futexp, INT_MAX); }
