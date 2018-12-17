/*************************************************************************
  > File Name: util.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 四 12/13 08:19:27 2018
 ************************************************************************/
#ifndef _UTIL_H_
#define _UTIL_H_
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#define ASSERT(_x)
#define NOT_REACHED()
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
void stacktrace_fd(int fd);
int init_net_socket(const char *host, int port, int backlog);
int set_socket_blocking(int sd);
int set_socket_nonblocking(int sd);
int set_socket_tcpnodelay(int sd);
int set_socket_reuseaddr(int sd);
int str_atoi(uint8_t *line, size_t n);
int set_socket_linger(int sd, int timeout);
int set_socket_tcpkeepalive(int sd);
int set_socket_sndbuf(int sd, int size);
int set_socket_rcvbuf(int sd, int size);
int get_socket_soerror(int sd);
int get_socket_sndbuf(int sd);
bool valid_port(int n);
int file_exists(const char *path);
#endif
