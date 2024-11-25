#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdint.h>
#include <stdlib.h>

#include "vector.h"

enum ConnectionState { STATE_REQ, STATE_RES, STATE_END };

typedef struct Conn {
  int fd;
  enum ConnectionState state;
  size_t rbuf_size;
  Vector rbuf;
  size_t wbuf_sent;
  Vector wbuf;
} Conn;

Conn *connection_create(int fd);
void connection_io(Conn *conn);
void connection_close(Conn *conn);

#endif // CONNECTION_H
