/*************************************************************************
  > File Name: net_wrapper.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 四 12/13 08:19:27 2018
 ************************************************************************/

#ifndef _NET_WRAPPER_H
#define _NET_WRAPPER_H
void net_epoll_run(const char *host,int port, int backlog);
void net_select_run(const char *host, int port, int backlog);
#endif
