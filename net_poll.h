/*************************************************************************
  > File Name: net_poll.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 四 12/13 08:19:27 2018
 ************************************************************************/

#ifndef _NET_WRAPPER_H
#define _NET_WRAPPER_H
#include <stdio.h>
#define NET_BUFFER_SIZE (4096)
struct event
{
  int fd;
  int status;
  int events;
  void *ctx;
  char *recv_buf;
  char *send_buf;
  int send_buf_len;
  int recv_buf_len;
  long last_active;
  void (*callback)(void *, int, int, void *);
};
struct net_poll
{
  struct event *s_events;
  int backlog;
  int lfd;
  int epfd;
  int check_fd_size;
  bool stop;
  void (*do_read_request)(void *,int);
  void (*do_write_response)(void *,int);
  void (*event_setbuf_size)(int,int);
};
void net_poll_init(struct net_poll *np, int backlog, int port,int check_fd_size);
void net_poll_set(struct net_poll *np,int check_fd_size,int timeout);
void net_poll_start(struct net_poll *np);
void net_poll_deinit(struct net_poll *np);
#endif
