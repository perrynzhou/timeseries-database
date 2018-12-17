/*************************************************************************
  > File Name: net_wrapper.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: 四 12/13 08:19:36 2018
 ************************************************************************/

#include "io.h"
#include "util.h"
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
void net_select_run(const char *host, int port, int backlog) {
  int sfd = init_net_socket(host, port, backlog);
  if (sfd != -1) {
    fd_set read_set[2];
    FD_ZERO(&read_set[0]);
    FD_SET(sfd, &read_set[0]);
    int maxfd = sfd;
    while (1) {
      read_set[1] = read_set[0];
      int ret = select(maxfd + 1, &read_set[1], , NULL, NULL, NULL);
      if (ret == -1) {
        break;
      }
      for (int i = sfd; i <= maxfd; i++) {
        if (FD_ISSET(i, &read_set[1])) {
          if (i == sfd) {
            int cfd = accept(sfd, NULL, NULL);
            if (cfd == -1) {
              close(sfd);
              return;
            }
            FD_SET(cfd, &read_set[0]);
            maxfd = (maxfd < cfd) ? cfd : maxfd;
          } else {
            char buf[1024] = {'\0'};
            ssize_t ret = read_n(i, &buf, 1024);
            if (ret < 0) {
              continue;
            }
            size_t len = strlen(buf);
            if (len == 0) {
              FD_CLR(i, &read_set[0]);
            }
          }
        }
      }
    }
  }
  if (sfd != -1) {
    close(sfd);
  }
}
void net_epoll_run(const char *host, int port, int backlog) {

  int elfd = epoll_create(backlog);
  int max_events = backlog;
  struct epoll_event evts[max_events];
  memset(&evnts,0,sizeof(struct epoll_event)*max_events);
  int sfd = init_net_socket(host, port, backlog);
  struct epoll_event ev;
  ev.data.fd = sfd;
  ev.events = EPOLLIN;
  epoll_ctl(elfd, EPOLL_CTL_ADD, sfd, &ev);
  while (1) {
    int ret = epoll_wait(elfd, &evts, max_events, -1);
    if (ret == -1) {
      break;
    }
    for (int i = 0; i < ret; i++) {
      int fd = evts[i].data.fd;
      if (fd == sfd) {
        int cfd = accept(fd, NULL, NULL);
        if (cfd == -1) {
          close(sfd);
          return;
        }
        ev.data.fd = cfd;
        ev.events = EPOLLIN;
        epoll_ctl(elfd, EPOLL_CTL_ADD, cfd, &ev);
      } else {
        char buf[1024] = {'\0'};
        ssize_t ret = ead_n(evts[i].data.fd, &buf, 1024);
        if (len < 0) {
          continue;
        }
        size_t len = strlen(buf);
        if (len == 0) {
          epoll_ctl(elfd, EPOLL_CTL_DEL, fd, NULL);
        }
      }
    }
  }
  if (sfd != -1) {
    close(sfd);
  }
}