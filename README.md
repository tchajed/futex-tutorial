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
// Put this thread to sleep until another thread calls futex_wait on the same
// futex. If the value currently stored is not expect_val, immediately returns
// with -EEAGAIN.
int futex_wait(uint32_t *futexp, uint32 expect_val);

// Wake threads waiting on futexp, up to a maximum of num_waiters (which is
// typically either 1 or INT_MAX).
int futex_wake(uint32_t *futexp, uint32 num_waiters);
```

A basic implementation of mutexes using futexes looks like the following (see
[mutex.c](mutex.c) for a full-fledged version that compiles):

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

The implementation of lock is interesting. A loop is required because between
the `futex_wait` and the subsequent `atomic_cas`, another thread might try to
acquire the lock and succeed (it wouldn't be another thread waiting from before,
since `futex_wake` only wakes one thread). The `LOCKED` argument to `futex_wait`
is required to avoid starvation: without this feature, if the current thread
owner _releases_ the lock between the `atomic_cas` and `futex_wait`, then we
will be in a situation where the lock is free, but we are sleeping in
`futex_wait` and uselessly waiting for a `futex_wake` that will (probably) never
come. In this situation `futex_wait(m, LOCKED)` immediately returns with an
error and the CAS will succeed. To summarize, the `expect_val` argument ensures
that we wait only if there really is another thread that will call `futex_wake`
eventually, to avoid deadlock. When `futex_wait` returns this is no guarantee
that the condition we are waiting for holds, so it is still surrounded by a loop
that checks the lock state.

While this implementation works, it is inefficient when _releasing_ an uncontended lock: releasing the lock
involves making a system call to `futex_wake`, even though there is no thread
waiting for the lock. We can do better and avoid any system calls for
uncontended locks by using the futex state to track whether there are waiters,
but the implementation is a bit intricate; see [mutex_better.c](mutex_better.c)
for a complete C implementation of the pseudo-code in the paper. In my (fairly
unscientific) benchmarks, this brings the cost of acquiring and releasing an
uncontended mutex from 420 ns down to only 15 ns.

---

I found this [Collabora blog
post](https://www.collabora.com/news-and-blog/blog/2022/02/08/landing-a-new-syscall-part-what-is-futex/)
useful for an initial explanation of futexes, and the classic ["Futexes
are hard"](https://www.akkadia.org/drepper/futex.pdf) useful (in particular for
the optimized mutex implementation).

I was initially quite confused about what futexes are, even though I'd read
something about them. In the end the best resource is probably the man page,
[futex(2)](https://man7.org/linux/man-pages/man2/futex.2.html). One point of
confusion is that you actually don't want to start with _what a futex is_ but
rather _how a futex is used_. Even when explained, it's hard to make sense of
what the API is for. Instead, the following flow is better:

1. A mutex can be implemented using only the kernel (which is expensive because
   it requires syscalls), or only in userspace (which is expensive since waiting
   requires spinning, and it is hard to implement things like queueing and
   fairness). Futexes provide a middle ground where the kernel implements the
   thing only the kernel can do: put threads to sleep and wake them up
   appropriately.
2. A mutex can be implemented using the futex API around a shared word of
   memory; the simple explanation above highlights `futex_wait` and `futex_wake`
   as the core wait/wake primitives. Waiting requires a current value to avoid a
   race condition, illustrated in the code above.
3. So what _is_ a futex? It is a family of system calls for managing kernel wait queues
   related to user-space memory. These system calls implement the core tasks of
   putting threads to sleep and waking them up, which are useful for
   implementing higher-level synchronization primitives on top.

One cool thing about this investigation is that you can see that _there is no
magic_ even for something as low-level as a mutex. The example in
[test.c](test.c) (a loose adaption of the demo in the
[futex(2)](https://man7.org/linux/man-pages/man2/futex.2.html) man page)
essentially uses only the `fork()` and `futex()` syscalls, and even though the
latter has to be issued directly with `syscall()` because for some reason glibc
doesn't provide wrapper functions for `futex` (it also does use `wait()`).
