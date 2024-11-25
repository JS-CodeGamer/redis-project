#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

#include "vector.h"

typedef struct Server {
  int fd;
  Vector conns;
} Server;

Server *server_new(uint32_t address, uint16_t port);
void server_init(Server *serv, uint32_t address, uint16_t port);
int server_run(Server *serv);
void server_cleanup(Server *serv);

#endif // SERVER_H
