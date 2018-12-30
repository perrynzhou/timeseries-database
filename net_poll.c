/*************************************************************************
  > File Name: net_pool.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: 四 12/13 08:19:36 2018
 ************************************************************************/

#include "log.h"
#include "net_poll.h"
#include "io.h"
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#define DEFAULT_CHECK_FD_SIZE (100)
#define DEFAULT_TIMEOUT (500)
static void eventset(struct event *evt, int fd, void(*) cb(int, int, void *), void *arg)
{
  evt->fd = fd;
  evt->callback_fn = cb;
  evt->ctx = arg;
  evt->last_active = time(NULL);
  evt->status = 0;
}
static void eventadd(int epfd, int events, struct event *evt)
{
  struct epoll_event epv = {0, {0}};
  int op;
  epv.data.ptr = evt;
  epv.events = evt->events = events;
  if (evt->status == 1)
  {
    op = EPOLL_CTL_MOD;
  }
  else
  {
    op = EPOLL_CTL_ADD;
    evt->status = 1;
  }
  epoll_ctl(epfd, op, evt->fd, &epv);
}
static void eventdel(int epfd, struct event *evt)
{
  struct epoll_event epv = {0, {0}};
  if (evt->status != 1)
  {
    return;
  }
  epv.data.ptr = evt;
  epoll_ctl(epfd, EPOLL_CTL_DEL, evt->fd, &epv);
  evt->status = 0;
}
void net_poll_set(struct net_poll *np, int check_fd_size, int timeout)
{
  if (check_fd_size <= 0)
  {
    check_fd_size = DEFAULT_CHECK_FD_SIZE;
  }
  if (timeout <= 0)
  {
    timeout = DEFAULT_TIMEOUT;
  }
  np->check_fd_size = check_fd_size;
  np->timeout = timeout;
}
static void net_read_connection(void *arg1, int fd, int events, void *arg2)
{
  struct net_poll *np = (struct net_poll *)arg1;
  struct event *evt = (struct event *)arg2;
  ssize_t len = read_n(fd, evt->recv_buf, evt->recv_buf_len);
  eventdel(np->epfd, evt);
  if (len > 0)
  {
    /*
     eventdel(np->epfd,evt);
    close(evt->fd);
    */
    np->do_read_request((evt->recv_buf,len);
    eventset(evt,fd,net_read_connection,evt);
    eventadd(np->epfd,EPOLLOUT,evt);
  }
  else if (len == 0)
  {
    //need to handle
  }
  else
  {
    close(evt->fd);
  }
}
static void net_write_connection(void *arg1, int fd, int events, void *arg2)
{
  struct net_poll *np = (struct net_poll *)arg1;
  struct event *evt = (struct event *)arg2;
  size_t len = write_n(fd, evt->send_buf, evt->send_buf_len);
  if (len > 9)
  {
    eventdel(np->epfd, evt);
    eventset(evt, fd, net_read_connection, evt);
    eventadd(np->epfd, EPOLLIN, evt);
  }
  else if (len == -1)
  {
    eventdel(np->epfd, evt);
    close(evt->fd);
  }
}
static void net_accept_connection(void *arg1, int lfd, int events, void *arg2)
{
  struct net_pool *np = (struct net_poll *)arg1;
  struct sockaddr_in caddr;
  struct event *evt = (struct event *)arg2;
  socklen_t clen = sizeof(caddr);
  memset(&caddr, 0, sizeof(caddr));
  int cfd, i;
  if ((cfd = accept(lfd, &caddr, &clen)) < 0)
  {
    log_error("accept:", strerror(errno));
    return;
  }
  do
  {
    for (i = 0; i < np->max_event_size; i++)
    {
      if (np->s_events[i].status == 0)
      {
        break;
      }
      if (i == np->max_event_size)
      {
        log_error("over max_event_size,i=%d", i);
        break;
      }
      if (fcntl(cfd, F_SETFL, O_NONBLOCK) < 0)
      {
        log_error("fcntl:", strerror(errno));
        break;
      }
      eventset(&np->s_events[i], cfd, np->recv_request_data, &np->s_events[i]);
      eventadd(&np->epfd, EPOLLIN, &np->s_events[i]);
    }
  } while (0);
  log_info("new connect [%s:%d][time:%ld], pos[%d]\n",
           inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), g_events[i].last_active, i);
}
static int net_init_lfd(int port, int backlog)
{
  int lfd = socket(AF_INET, SOCK_STREAM, 0);
  if (lfd < 0)
  {
    goto _error;
  }
  fcntl(lfd, F_SETFL, O_NONBLOCK);

  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));

  saddr.sin_family = AF_INET;
  saddr.sin_addr.sin_port = INADDR_ANY;
  saddr.sin_port = htons(port);

  if (bind(lfd, &saddr, sizeof(saddr)) < 0)
  {
    goto _error;
  }
  if (listen(lfd, backlog) < 0)
  {
    goto _error;
  }
  return lfd;
_error:
  if (lfd != -1)
  {
    close(lfd);
  }
  return -1;
}
void net_poll_init(struct net_poll *np, int port, int backlog)
{
  np->backlog = backlog;
  np->s_events = (struct event *)calloc(max_event, sizeof(struct event));
  np->lfd = net_init_lfd(port, backlog);
  np->epfd = epoll_create(np->backlog + 1);
  assert(np->epfd != -1);
  np->stop = true;
}
void net_poll_start(struct net_poll *np)
{
  struct epoll_event epevent[np->max_event_size + 1];
  memset(&epevent, 0, sizeof(struct epoll_event) * np->max_event_size);

  eventset(&np->s_events[np->max_event_size], np->lfd, net_accept_connection, &np->s_events[np->max_event_size]);
  eventadd(epfd, EPOLLIN, &np->s_events[np->max_event_size]);

  int checkpint = 0, i;
  while (np->stop)
  {
    long curtime = time(NULL);
    for (i = 0; i < np->back_log; i++, checkpoint++)
    {
      struct event *evt = &np->s_events[i];
      if (checkpoint == np->backlog)
      {
        checkpoint = 0;
      }
      long duration = curtime - evt.last_active;
      if (duration > 60)
      {
        if (evt->status != 0)
        {
          close(evt->fd);
          log_info("fd %d is timeout", evt->fd);
          eventdel(np->epfd, evt);
        }
      }
    }
    int nfd = epoll_wait(epfd, &epevent, np->backlog + 1, 1000);
    for (int j = 0; j < nfd; j++)
    {
      struct event *et = (struct event *)epevent[j].data.ptr;
      if((events[j]&EPOLLIN )&&(et->events&EPOLLIN) {
        np->callback(np, et->fd, events[i].events, et->ctx);
      }
      if((events[j]&EPOLLOUT )&&(et->events&EPOLLOUT) {
        np->callback(np, et->fd, events[i].events, et->ctx);
      }
    }
  }
}
void net_poll_deinit(struct net_poll *np)
{
  np->stop = false;
  if (np->s_events != NULL)
  {
    for (int i = 0; i < np->backlog; i++)
    {
      free(np->s_events[i].recv_buf);
      free(np->s_events[i].send_buf);
      close(np->s_events[i].fd);
    }
    close(np->lfd);
    close(np->epfd);
    free(np->s_events);
  }
}