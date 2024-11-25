#include <stdlib.h>

#include "server.h"

const int PORT = 6379;
int LOG_LEVEL = -1;

int main(void) {
  char *dbg_lvl = getenv("DEBUG");
  if (dbg_lvl) {
    LOG_LEVEL = atoi(dbg_lvl);
  } else {
    LOG_LEVEL = 0;
  }

  Server *serv = server_new(0, PORT);
  server_run(serv);
  server_cleanup(serv);

  return EXIT_SUCCESS;
}
