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

#define NLOOPS 100

int main() {

  mutex_t *m = new_mutex();

  int childPid = fork();
  if (childPid == -1)
    errExit("fork");

  if (childPid == 0) { /* Child */
    for (int j = 0; j < NLOOPS; j++) {
      mutex_lock(m);
      printf("Child  (%jd) %d\n", (intmax_t)getpid(), j);
      mutex_unlock(m);
    }

    return 0;
  }

  /* Parent falls through to here. */

  for (int j = 0; j < NLOOPS; j++) {
    mutex_lock(m);
    printf("Parent (%jd) %d\n", (intmax_t)getpid(), j);
    mutex_unlock(m);
  }

  wait(NULL);

  return 0;
}
