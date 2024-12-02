#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "connection.h"
#include "utils.h"
#include "vector.h"

typedef struct sockaddr_in ipv4_addr;
const size_t MAX_MSG_SIZE = 1024;

Conn *connection_create(int fd) {
  Conn *conn = malloc(sizeof(Conn));
  if (!conn) {
    return NULL;
  }
  conn->fd = fd;
  conn->state = STATE_REQ;
  conn->rbuf_size = 0;
  conn->wbuf_sent = 0;
  vector_initialize(&conn->rbuf, MAX_MSG_SIZE + 4, sizeof(uint8_t));
  vector_initialize(&conn->wbuf, MAX_MSG_SIZE + 4, sizeof(uint8_t));
  return conn;
}

static bool is_read_complete(Conn *conn) {
  size_t len = 0;
  if (*(char *)vector_get_at(&conn->rbuf, 0) == '*') {
    len = atol((char *)vector_get_at(&conn->rbuf, 1));
  } else {
    len = atol((char *)vector_get_at(&conn->rbuf, 0));
  }

  if (!len || len > conn->rbuf_size) {
    return false;
  }
  if (len < conn->rbuf_size) {
    ERROR(false, "invalid message sent")
    conn->state = STATE_END;
    return false;
  }

  return true;
}

// force if connection terminated before complete
//  command data transfer
static void build_response(Conn *conn, bool force) {
  vector_copy(&conn->rbuf, &conn->wbuf);
  vector_clear(&conn->rbuf);
  vector_resize(&conn->rbuf, MAX_MSG_SIZE + 4);
  conn->rbuf_size = 0;
}

// return:
//  true   read success (can try again)
//  false  read fail (dont try again)
static bool connection_read(Conn *conn) {
  assert(conn->rbuf_size < vector_length(&conn->rbuf));

  ssize_t rv = 0;
  do {
    size_t cap = vector_length(&conn->rbuf) - conn->rbuf_size;
    rv = read(conn->fd, vector_get_at(&conn->rbuf, conn->rbuf_size), cap);
  } while (rv < 0 && errno == EINTR);

  if (rv < 0 && errno == EAGAIN) {
    // got EAGAIN, stop.
    return false;
  }

  if (rv < 0) {
    ERROR(false, "error reading from connection")
    conn->state = STATE_END;
    return false;
  }

  if (rv == 0) {
    if (conn->rbuf_size > 0) {
      ERROR(false, "unexpected EOF")
      build_response(conn, true);
    } else {
      LOG(2, "EOF")
    }
    conn->state = STATE_END;
    return false;
  }

  conn->rbuf_size += (size_t)rv;
  assert(conn->rbuf_size <= vector_length(&conn->rbuf));

  return true;
}

static void handle_req(Conn *conn) {
  LOG(3, "Conn(%d): reading from req", conn->fd)

  while (conn->state == STATE_REQ && connection_read(conn)) {
    if (is_read_complete(conn)) {
      build_response(conn, false);
      conn->state = STATE_RES;
    }
  }

  LOG(3, "Conn(%d): recieved:\n%s", conn->fd, vector_get_at(&conn->wbuf, 0))
}

static bool connection_write(Conn *conn) {
  ssize_t rv = 0;
  do {
    size_t remain = vector_length(&conn->wbuf) - conn->wbuf_sent;
    rv = write(conn->fd, vector_get_at(&conn->wbuf, conn->wbuf_sent), remain);
  } while (rv < 0 && errno == EINTR);

  if (rv < 0 && errno == EAGAIN) {
    // got EAGAIN, stop.
    return false;
  }

  if (rv < 0) {
    ERROR(false, "error writing to connection")
    conn->state = STATE_END;
    return false;
  }

  conn->wbuf_sent += (size_t)rv;
  assert(conn->wbuf_sent <= vector_length(&conn->wbuf));

  if (conn->wbuf_sent == vector_length(&conn->wbuf)) {
    // response was fully sent, change state back
    conn->state = STATE_REQ;
    conn->wbuf_sent = 0;
    vector_clear(&conn->wbuf);
    return false;
  }
  // still got some data in wbuf, could try to write again
  return true;
}

static void send_res(Conn *conn) {
  LOG(3, "Conn(%d): writing to conn", conn->fd)

  bool continue_writing = false;
  do {
    continue_writing = connection_write(conn);
  } while (continue_writing);

  LOG(3, "Conn(%d): writing done", conn->fd)
}

void connection_io(Conn *conn) {
  LOG(3, "Conn(%d): New IO Available", conn->fd)

  if (conn->state == STATE_REQ) {
    handle_req(conn);
  } else if (conn->state == STATE_RES) {
    send_res(conn);
  } else {
    assert(0); // not expected
  }

  LOG(3, "Conn(%d): IO done", conn->fd)
}

void connection_close(Conn *conn) {
  LOG(2, "Conn(%d): Closing", conn->fd)

  close(conn->fd);
  vector_cleanup(&conn->wbuf);
  vector_cleanup(&conn->rbuf);
  free(conn);

  LOG(1, "Conn: Closed")
}
