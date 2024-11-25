#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

void fd_to_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl error");
    exit(EXIT_FAILURE);
  }
  flags |= O_NONBLOCK;
  int return_val = fcntl(fd, F_SETFL, flags);
  if (return_val == -1) {
    perror("fcntl error");
    exit(EXIT_FAILURE);
  }
}
