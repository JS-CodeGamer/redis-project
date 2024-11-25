#ifndef UTILS_H
#define UTILS_H

#include <assert.h>
#include <stdbool.h>

typedef struct pollfd poll_arg;
typedef struct sockaddr_in ipv4_addr;

extern int LOG_LEVEL;

#define isint(x) _Generic((x), int: 1, default: 0)
#define isbool(x) _Generic((x), int: 1, bool: 1, default: 0)
#define LOG(log_level, msg...)                                                 \
  {                                                                            \
    static_assert(isint((log_level)) && (log_level) >= 0, "Error in LOG");     \
    if ((log_level) <= LOG_LEVEL) {                                            \
      printf("LOG(%d) >> ", log_level);                                        \
      printf(msg);                                                             \
      printf("\n");                                                            \
    }                                                                          \
  }
#define ERROR(_exit, msg)                                                      \
  {                                                                            \
    static_assert(isbool(_exit), "Error in ERROR");                            \
    perror(msg);                                                               \
    if (_exit) {                                                               \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  }

void fd_to_nonblocking(int fd);

#endif // UTILS_H
