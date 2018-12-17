/*************************************************************************
  > File Name: io.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 三 12/12 11:12:51 2018
 ************************************************************************/

#include "io.h"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
ssize_t write_n(int fd, void *ptr, size_t n)
{
  char *p = (char *)ptr;
  size_t nwriten = 0;
  while (nwriten < n)
  {
    int wn = write(fd, p, n - nwriten);
    if (wn > 0)
    {
      nwriten += wn;
    }
    else if (wn == -1)
    {
      if (errno != EINTR)
      {
        return -1;
      }
      break;
    }
  }
  return n-nwriten;
}
ssize_t read_n(int fd, void *ptr, size_t n)
{
  char *p = (char *)ptr;
  size_t readn = 0;
  while (readn < n)
  {
    int rn = read(fd, p, n - readn);
    if (rn > 0)
    {
      readn += rn;
    }
    else
    {
      if (errno != EINTR)
      {
        return -1;
      }
      break;
    }
  }
  return n-readn;
}