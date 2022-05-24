# Illustrative examples of futexes

Futexes ("fast user-space mutexes") are a set of syscalls useful for
implementing synchronization primitives like mutexes. The mutex itself is stored
in userspace shared memory, which is why futexes can be extremely fast.

A futex is a 32-bit integer stored in userspace memory. Such a shared integer
can be used to implement synchronization primitives using the hardware's atomic
instructions (like compare-and-exchange or fetch-and-add). Where the kernel gets
involved is that it is the only part of the system that can put threads to sleep
and wake them up; otherwise threads would have to spin while waiting, which is
inefficient for many use cases (when this isn't a problem, you can instead use a
spin lock).

The basic API consists of two system calls, where `uint32_t *futexp` is a
pointer to shared memory:

```c
// Put this thread to sleep until another thread calls futex_wait on the same futex. If the value currently stored is not expect_val, immediately returns with -EEAGAIN.
int futex_wait(uint32_t *futexp, uint32 expect_val);

// Wake threads waiting on futexp, up to a maximum of num_waiters (which is typically either 1 or INT_MAX).
int futex_wake(uint32_t *futexp, uint32 num_waiters);
```

In their most basic form, mutexes can be implemented using futexes like the
following (see [mutex.c](mutex.c) for a full-fledged version that compiles):

```c
typedef mutex_t uint32_t;

void mutex_lock(mutex_t *m) {
  while (!atomic_cas(m, UNLOCKED, LOCKED)) {
    futex_wait(m, LOCKED);
  }
}

void mutex_unlock(mutex_t *m) {
  atomic_store(m, UNLOCKED);
  futex_wake(m, 1);
}
```

However, this version is inefficient for an uncontended lock: releasing the lock
involves making a system call to `futex_wake`, even though there are no thread
waiting for the lock. We can do better and avoid any system calls for
uncontended locks by using the futex state to track whether there are waiters,
but the implementation is a bit intricate; see [mutex_better.c](mutex_better.c)
for a complete C implementation of the pseudo-code in the paper. In my (fairly
unscientific) benchmarks, this brings the cost of acquiring and releasing an
uncontended mutex from 420 ns down to only 15 ns.

I found this [Collabora blog
post](https://www.collabora.com/news-and-blog/blog/2022/02/08/landing-a-new-syscall-part-what-is-futex/)
useful for an initial explanation of futexes, and the classic ["Futexes
are hard"](https://www.akkadia.org/drepper/futex.pdf) useful (in particular for
the optimized mutex implementation).
