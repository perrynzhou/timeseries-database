/*************************************************************************
  > File Name: io.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 三 12/12 11:12:44 2018
 ************************************************************************/

#ifndef _IO_H
#define _IO_H
#include <stdio.h>
ssize_t  write_n(int fd,void *ptr,size_t n);
ssize_t  read_n(int fd,void *ptr,size_t n);
#endif
