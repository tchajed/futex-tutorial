#include "mutex.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define errExit(msg)                                                           \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#define NTHREADS 10
#define NLOOPS 10000

int main() {
  mutex_t *m = new_mutex();
  int mutex_held = 0;

  for (int child_index = 0; child_index < NTHREADS; child_index++) {
    int childPid = fork();
    if (childPid == -1) {
      errExit("fork");
    }
    if (childPid == 0) { /* Child */
      for (int j = 0; j < NLOOPS; j++) {
        mutex_lock(m);
        if (mutex_held == 1) {
          errExit("lock already held");
        }
        mutex_held = 1;
        printf("Child  (%jd) %d\n", (intmax_t)getpid(), j);
        mutex_held = 0;
        mutex_unlock(m);
      }
      printf("Child  (%jd) done\n", (intmax_t)getpid());
      return 0;
    }
  }

  /* Parent falls through to here. */
  printf("threads spawned\n");
  for (int child_index = 0; child_index < NTHREADS; child_index++) {
    wait(NULL);
  }
  printf("Parent (%jd) done\n", (intmax_t)getpid());

  return 0;
}
