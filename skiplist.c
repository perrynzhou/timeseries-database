/*************************************************************************
  > File Name: skiplist.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: 五 12/14 16:38:23 2018
 ************************************************************************/

#include "skiplist.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#define SKIPLIST_MAX_LEVEL (16)
static int skiplist_random_level(int max_level)
{
  int level = 0;
  while ((rand() % 2) && level < (max_level - 1))
  {
    level++;
  }
  return level;
}
static struct skiplist_node *skiplist_node_init(int level, void *data,
                                                uint32_t key)
{
  struct skiplist_node *slt = calloc(1, sizeof(struct skiplist_node));
  if (slt != NULL)
  {
    slt->key = key;
    slt->data = data;
    slt->level = level;
    slt->next =
        (struct skiplist_node **)malloc(sizeof(struct skiplist_node *) * level);
    if (slt->next == NULL)
    {
      free(slt);
      slt = NULL;
    }
  }
  return slt;
}
inline static void skip_listnode_destroy(struct skiplist_node *slt)
{
  if (slt != NULL)
  {
    free(slt->next);
    free(slt);
    slt = NULL;
  }
}
void skiplist_init(struct skiplist *st, int max_level)
{
  if (max_level < 0)
  {
    max_level = 1;
  }

  max_level = (max_level > SKIPLIST_MAX_LEVEL) ? SKIPLIST_MAX_LEVEL : max_level;
  assert(st != NULL);
  st->head = (struct skiplist_node *)malloc(sizeof(struct skiplist_node));
  st->tail = (struct skiplist_node *)malloc(sizeof(struct skiplist_node));
  st->head->next = (struct skiplist_node **)malloc(
      sizeof(struct skiplist_node *) * max_level);
  st->level_nodesize = (int *)calloc(max_level, sizeof(int));
  assert(st->level_nodesize != NULL);
  for (int i = 0; i < max_level; i++)
  {
    st->head->next[i] = st->tail;
  }
  st->max_level = max_level;
}
struct skiplist *skiplist_create(int max_level,
                                 void (*handle_data_cb)(void *))
{

  struct skiplist *st = (struct skiplist *)calloc(1, sizeof(struct skiplist));
  assert(st != NULL);
  skiplist_init(st, max_level);
  st->handle_data_cb = handle_data_cb;
  return st;
}
int skiplist_insert(struct skiplist *st, void *data, uint32_t key)
{
  struct skiplist_node *cur = st->head;
  struct skiplist_node *update[st->max_level];
  for (int i = (st->max_level - 1); i >= 0; i--)
  {
    if (cur->next[i] == st->tail || cur->next[i]->key > key)
    {
      update[i] = cur;
    }
    else
    {
      while (cur->next[i] != st->tail && cur->next[i]->key < key)
      {
        cur = cur->next[i];
      }
      if (cur->next[i]->key == key)
      {
        fprintf(stdout, " ...the same key:%d\n", key);
        return -1;
      }
      update[i] = cur;
    }
  }
  int level = skiplist_random_level(st->max_level) + 1;
  struct skiplist_node *slt = skiplist_node_init(level, data, key);
  for (int i = 0; i < level; i++)
  {
    slt->next[i] = update[i]->next[i];
    update[i]->next[i] = slt;
    __sync_fetch_and_add(&st->level_nodesize[i], 1);
  }
  return 0;
}
int skiplist_remove(struct skiplist *st, uint32_t key)
{
  struct skiplist_node *update[st->max_level];
  struct skiplist_node *cur = st->head;
  for (int i = st->max_level - 1; i >= 0; --i)
  {
    if (cur->next[i] == st->tail || cur->next[i]->key > key)
    {
      update[i] = NULL;
    }
    else
    {
      while (cur->next[i] != st->tail && cur->next[i]->key < key)
      {
        cur = cur->next[i];
      }
      if (cur->next[i] != st->tail && cur->next[i]->key == key)
      {
        update[i] = cur;
      }
      else
      {
        update[i] = NULL;
      }
    }
  }
  struct skiplist_node *temp = NULL;
  for (int i = 0; i < st->max_level; i++)
  {
    if (update[i] != NULL)
    {
      __sync_fetch_and_sub(&st->level_nodesize[i], 1);
      temp = update[i]->next[i];
      update[i]->next[i] = temp->next[i];
    }
  }
  if (temp != NULL)
  {
    skip_listnode_destroy(temp);
    temp = NULL;
  }
  return 0;
}
void *skiplist_search(struct skiplist *st, uint32_t key)
{
  struct skiplist_node *cur = st->head;
  for (int i = (st->max_level - 1); i >= 0; i--)
  {
    if (cur->next[i] == st->tail || cur->next[i]->key > key)
    {
      continue;
    }
    while (cur->next[i] != st->tail && cur->next[i]->key < key)
    {
      cur = cur->next[i];
    }
    if (cur->next[i] != st->tail && cur->next[i]->key == key)
    {
      return cur->next[i]->data;
    }
  }
  return NULL;
}
void skiplist_destroy(struct skiplist *st)
{

  skiplist_deinit(st);
  free(st);
  st = NULL;
}
void skiplist_deinit(struct skiplist *st)
{
  struct skiplist_node *cur = st->head->next[0];

  while (cur != NULL && cur != st->tail)
  {
    struct skiplist_node *next = cur->next[0];
    st->handle_data_cb(cur->data);
    skip_listnode_destroy(cur);
    cur = next;
  }
  skip_listnode_destroy(st->head);
  skip_listnode_destroy(st->tail);
  free(st->level_nodesize);
}
void skiplist_print(struct skiplist *st, bool is_print_data)
{
  for (int i = (st->max_level - 1); i >= 0; i--)
  {

    struct skiplist_node *cur = st->head->next[i];
    int nodesize = st->level_nodesize[i];
    if (nodesize == 0)
    {
      fprintf(stdout, " level%d[size=%d] -| \n", i, st->level_nodesize[i]);
      continue;
    }
    fprintf(stdout, " level%d[size=%d] -| ", i, st->level_nodesize[i]);

    if (is_print_data)
    {
      while (cur != st->tail)
      {
        struct skiplist_node *next = cur->next[i];
        fprintf(stdout, "%d,", cur->key);
        cur = next;
      }
      fprintf(stdout, "\n");
    }
  }
}
#ifdef TEST
int main(int argc, char *argv[])
{
  int n = atoi(argv[1]);
  int *values = (int *)calloc(n, sizeof(int));
  int fn = n / 6;
  struct skiplist *st = skiplist_create(3);
  // fprintf(stdout, "------insert skip_list-----------\n");
  for (int i = 0; i < n; i++)
  {
    values[i] = rand() % n + i;
    if (values[i] == 0)
    {
      // fprintf(stdout, "...this is 0\n");
    }
    int ret = skiplist_insert(st, &values[i], values[i]);
    // fprintf(stdout, "insert %d to skiplist,ret=%d\n", values[i], ret);
  }
  // fprintf(stdout, "----------print skip_list------------\n");
  skiplist_print(st, true);
  // fprintf(stdout, "-------find skip_lsit ------------\n");
  for (int i = 1; i < fn; i++)
  {
    int key = values[i];
    int *ptr = skiplist_search(st, key);
    if (ptr != NULL)
    {
      // fprintf(stdout, " search key=%d,*ptr=%d\n", key, *ptr);
    }
  }
  // fprintf(stdout, "------delete skip_list-----------\n");
  for (int i = 1; i < fn; i++)
  {
    int key = values[i];
    int ret = skiplist_remove(st, key);
    if (ret == 0)
    {
      // fprintf(stdout, "delete key=%d,ret=%d\n", key, ret);
    }
  }
  // fprintf(stdout, "----------print skip_list------------\n");
  skiplist_print(st, true);
  skiplist_destroy(st);
}
#endif