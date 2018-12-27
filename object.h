/*************************************************************************
  > File Name: object.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 四 12/27 17:46:45 2018
 ************************************************************************/

#ifndef _OBJECT_H
#define _OBJECT_H
#include "cstring.h"
struct object
{
  struct string key;
  struct string *value;
  uint32_t value_len;
  long long timestamp;
};

void object_init(struct object *obj,const char *key,...);
struct  object  *object_create();
struct object *object_search(struct object *object,const char *key);
void object_free(struct object *object);
#endif
