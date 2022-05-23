#ifndef MUTEX_H_
#define MUTEX_H_

#include <stdatomic.h>
#include <stdint.h>

typedef _Atomic uint32_t mutex_t;

mutex_t *new_mutex();
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);

#endif // MUTEX_H_
