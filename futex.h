#ifndef FUTEX_H_
#define FUTEX_H_

#include <stdatomic.h>
#include <stdint.h>

typedef _Atomic uint32_t futex_t;

int futex_wait(futex_t *futexp, uint32_t expect_val);

int futex_wake_num(futex_t *futexp, uint32_t num_waiters);

int futex_wake(futex_t *futexp);

int futex_wake_all(futex_t *futexp);

#endif // FUTEX_H_
