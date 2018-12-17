/*************************************************************************
  > File Name: db.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 三 12/12 10:58:51 2018
 ************************************************************************/

#include "conf.h"
#include "log.h"
#include "net_wrapper.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
inline void init_server_instance(struct conf_pool *cp)
{
  net_epoll_run((const char *)cp->addr.data, cp->port, cp->backlog);
}
int main(int argc, char *argv[])
{
  log_init(LOG_INFO, NULL);
  struct conf *f = conf_create("./test.yml", false);
  pid_t pid;
  if (f != NULL)
  {
    int cpn = array_n(&f->pool);
    for (int i = 0; i < cpn; i++)
    {
      if ((pid = fork()) == 0)
      {
        struct conf_pool *cp = array_get(&f->pool, i);
        init_server_instance(cp);
      }
    }
    wait(NULL);
    conf_dump(f);
    conf_destroy(f);
  }
}