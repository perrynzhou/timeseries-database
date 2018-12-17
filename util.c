#include "util.h"
#include "log.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef NC_HAVE_BACKTRACE
#include <execinfo.h>
#endif

void stacktrace_fd(int fd)
{
#ifdef NC_HAVE_BACKTRACE
    void *stack[64];
    int size;

    size = backtrace(stack, 64);
    backtrace_symbols_fd(stack, size, fd);
#endif
}
int set_socket_blocking(int sd)
{
    int flags;

    flags = fcntl(sd, F_GETFL, 0);
    if (flags < 0)
    {
        return flags;
    }

    return fcntl(sd, F_SETFL, flags & ~O_NONBLOCK);
}

int set_socket_nonblocking(int sd)
{
    int flags;

    flags = fcntl(sd, F_GETFL, 0);
    if (flags < 0)
    {
        return flags;
    }

    return fcntl(sd, F_SETFL, flags | O_NONBLOCK);
}
int init_net_socket(const char *host, int port, int backlog)
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1)
    {
        printf("socket error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, host, &addr.sin_addr) < 0)
    {
        close(sock);
        return -1;
    }
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
    {
        printf("bind  error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }
    if (listen(sock, backlog) < 0)
    {
        printf("listen error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }
    int yes;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
    {
        printf("setsockopt error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }
    return sock;
}
int set_socket_reuseaddr(int sd)
{
    int reuse;
    socklen_t len;

    reuse = 1;
    len = sizeof(reuse);

    return setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse, len);
}

int set_socket_tcpnodelay(int sd)
{
    int nodelay;
    socklen_t len;

    nodelay = 1;
    len = sizeof(nodelay);

    return setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &nodelay, len);
}

int set_socket_linger(int sd, int timeout)
{
    struct linger linger;
    socklen_t len;

    linger.l_onoff = 1;
    linger.l_linger = timeout;

    len = sizeof(linger);

    return setsockopt(sd, SOL_SOCKET, SO_LINGER, &linger, len);
}

int set_socket_tcpkeepalive(int sd)
{
    int val = 1;
    return setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
}

int set_socket_sndbuf(int sd, int size)
{
    socklen_t len;

    len = sizeof(size);

    return setsockopt(sd, SOL_SOCKET, SO_SNDBUF, &size, len);
}

int set_socket_rcvbuf(int sd, int size)
{
    socklen_t len;

    len = sizeof(size);

    return setsockopt(sd, SOL_SOCKET, SO_RCVBUF, &size, len);
}

int get_socket_soerror(int sd)
{
    int status, err;
    socklen_t len;

    err = 0;
    len = sizeof(err);

    status = getsockopt(sd, SOL_SOCKET, SO_ERROR, &err, &len);
    if (status == 0)
    {
        errno = err;
    }

    return status;
}

int get_socket_sndbuf(int sd)
{
    int status, size;
    socklen_t len;

    size = 0;
    len = sizeof(size);

    status = getsockopt(sd, SOL_SOCKET, SO_SNDBUF, &size, &len);
    if (status < 0)
    {
        return status;
    }

    return size;
}

int str_atoi(uint8_t *line, size_t n)
{
    int value;

    if (n == 0)
    {
        return -1;
    }

    for (value = 0; n--; line++)
    {
        if (*line < '0' || *line > '9')
        {
            return -1;
        }

        value = value * 10 + (*line - '0');
    }

    if (value < 0)
    {
        return -1;
    }

    return value;
}

bool valid_port(int n)
{
    if (n < 1 || n > UINT16_MAX)
    {
        return false;
    }

    return true;
}


