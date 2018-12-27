/*************************************************************************
  > File Name: expire.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 四 12/27 18:02:53 2018
 ************************************************************************/

#ifndef _EXPIRE_H
#define _EXPIRE_H
#include <pthread.h>
static pthread_t  expire_threads[32];
void expire_delete(struct skiplist *st);
#endif
