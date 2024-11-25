#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection.h"
#include "server.h"
#include "utils.h"
#include "vector.h"

const int POLL_TIMEOUT = 1000; // 1 sec

Server *server_new(uint32_t address, uint16_t port) {
  LOG(1, "Server Creation: Started")

  Server *serv = malloc(sizeof(Server));
  server_init(serv, address, port);

  LOG(1, "Server Creation: Completed")

  return serv;
}

static int setup_sock(uint32_t address, uint16_t port) {
  LOG(1, "Socket setup: Started")

  // create socket
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    ERROR(true, "socket creation failed")
  }
  int opt = 1;
  (void)setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  // bind socket to address
  ipv4_addr addr = {.sin_family = AF_INET,
                    .sin_port = ntohs(port),
                    .sin_addr.s_addr = ntohl(address)};
  int fail = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (fail) {
    ERROR(true, "error binding socket")
  }

  LOG(1, "Socket setup: Completed")

  return fd;
}

void server_init(Server *serv, uint32_t address, uint16_t port) {
  LOG(1, "Server setup: Started")

  // create and bind socket
  serv->fd = setup_sock(address, port);

  // listen
  int fail = listen(serv->fd, SOMAXCONN);
  if (fail) {
    ERROR(true, "error listening on socket")
  }

  // make socket non blocking
  fd_to_nonblocking(serv->fd);

  // initialize conns
  vector_initialize(&serv->conns, 0, sizeof(Conn *));

  LOG(1, "Server setup: Completed")
}

static int connection_accept(Server *serv) {
  LOG(2, "Conn: New")

  ipv4_addr client_addr;
  uint addr_size = sizeof(client_addr);
  int fd = accept(serv->fd, (struct sockaddr *)&client_addr, &addr_size);
  if (fd < 0) {
    ERROR(true, "error accepting connection")
  }

  fd_to_nonblocking(fd);

  Conn *conn_ptr = connection_create(fd);
  if (!conn_ptr) {
    (void)close(fd);
    return -1;
  }

  if (vector_length(&serv->conns) <= fd) {
    vector_resize(&serv->conns, fd + 1);
  }
  vector_set_at(&serv->conns, (const uint8_t *)&conn_ptr, fd);

  LOG(1, "Conn(%d): Accepted", fd)

  return 0;
}

static void prepare_poll_args(Vector *poll_args, Server *serv) {
  vector_clear(poll_args);
  vector_push_back(poll_args,
                   (const uint8_t *)(&(poll_arg){serv->fd, POLLIN, 0}));
  for (size_t i = 0; i < vector_length(&serv->conns); i++) {
    Conn *conn = *(Conn **)vector_get_at(&serv->conns, i);
    if (conn == NULL) {
      continue;
    }
    poll_arg tmp;
    tmp.fd = conn->fd;
    // NOLINTNEXTLINE(bugprone-narrowing-conversions)
    tmp.events = ((conn->state == STATE_REQ) ? POLLIN : POLLOUT) | POLLERR;
    vector_push_back(poll_args, (const uint8_t *)&tmp);
  }
}

int server_run(Server *serv) {
  LOG(0, "Server: Started")

  Vector poll_args;
  vector_initialize(&poll_args, 0, sizeof(poll_arg));

  // manage connections
  while (1) {
    // prepare for polling
    LOG(3, "Connection Polling: Started")

    // poll for new activity on connections or socket
    prepare_poll_args(&poll_args, serv);
    size_t n_poll_args = vector_length(&poll_args);
    int res = poll((poll_arg *)vector_get_at(&poll_args, 0), n_poll_args,
                   POLL_TIMEOUT);
    if (res < 0) {
      ERROR(true, "error polling connections / socket")
    }

    LOG(3, "Connection Polling: Completed")

    // process active connections
    for (size_t i = 1; i < n_poll_args; ++i) {
      poll_arg *curr = (poll_arg *)vector_get_at(&poll_args, i);
      Conn **conn = (Conn **)vector_get_at(&serv->conns, curr->fd);
      if (curr->revents) {
        connection_io(*conn);
      }
      if ((*conn)->state == STATE_END) {
        // destroy this connection
        connection_close(*conn);
        *conn = NULL;
      }
    }

    // accept new connection
    poll_arg *serv_poll_arg = (poll_arg *)vector_get_at(&poll_args, 0);
    if (serv_poll_arg->revents) {
      (void)connection_accept(serv);
    }
  }

  vector_cleanup(&poll_args);

  LOG(0, "Server: Closing")

  return 0;
}

void server_cleanup(Server *serv) {
  LOG(1, "Server Cleanup: Started")

  close(serv->fd);
  for (size_t i = 0; i < vector_length(&serv->conns); i++) {
    connection_close(*(Conn **)vector_get_at(&serv->conns, i));
  }
  vector_cleanup(&serv->conns);
  free(serv);

  LOG(1, "Server Cleanup: Completed")
}
