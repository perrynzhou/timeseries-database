/*************************************************************************
  > File Name: skip_list.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 五 12/14 16:38:18 2018
 ************************************************************************/

#ifndef _skiplist_H
#define _skiplist_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
struct skiplist_node {
  uint32_t key;
  void *data;
  int level;
  struct skiplist_node **next;
};
struct skiplist {
  int max_level;
  int *level_nodesize;
  struct skiplist_node *head;
  struct skiplist_node *tail;
  void   (*handle_data_cb)(void *ptr);
};
struct skiplist *skiplist_create(int max_level,void (*handle_data_cb)(void *));
void skiplist_init(struct skiplist *st,int max_level);
int skiplist_insert(struct skiplist *st,void *ptr,uint32_t key);
int skiplist_remove(struct skiplist *st,uint32_t key);
void *skiplist_search(struct skiplist *st,uint32_t key);
void skiplist_print(struct skiplist *st,bool is_print_data);
void skiplist_deinit(struct skiplist *st);
void skiplist_destroy(struct skiplist *st);
#endif
