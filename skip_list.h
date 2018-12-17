/*************************************************************************
  > File Name: skip_list.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 五 12/14 16:38:18 2018
 ************************************************************************/

#ifndef _SKIP_LIST_H
#define _SKIP_LIST_H

#include <stdint.h>
#include <stdio.h>
struct skip_listnode {
  uint32_t key;
  void *ptr;
  int level;
  struct skip_listnode **next;
};
struct skip_list {
  int max_level;
  struct skip_listnode *head;
  struct skip_listnode *tail;
};
struct skip_list *skip_list_create(int max_level);
int skip_list_insert(struct skip_list *st,void *ptr,uint32_t key);
int skip_list_remove(struct skip_list *st,uint32_t key);
void *skip_list_find(struct skip_list *st,uint32_t key);
void skip_list_print(struct skip_list *st);
void skip_list_destroy(struct skip_list *st);
#endif
