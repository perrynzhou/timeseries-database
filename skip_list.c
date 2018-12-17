/*************************************************************************
  > File Name: skip_list.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: 五 12/14 16:38:23 2018
 ************************************************************************/

#include "skip_list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#define SKIP_LIST_MAX_LEVEL (8)
static int skip_list_random_level(int max_level)
{
  int level = 0;
  while ((rand() % 2) && level < (max_level - 1))
  {
    level++;
  }
  return level;
}
static struct skip_listnode *skip_listnode_init(int level, void *ptr,
                                                uint32_t key)
{
  struct skip_listnode *slt = calloc(1, sizeof(struct skip_listnode));
  if (slt != NULL)
  {
    slt->key = key;
    slt->ptr = ptr;
    slt->level = level;
    slt->next = (struct skip_listnode **)malloc(sizeof(struct skip_listnode *) *
                                                (level + 1));
    if (slt->next == NULL)
    {
      free(slt);
      slt = NULL;
    }
  }
  return slt;
}
inline static void skip_listnode_destroy(struct skip_listnode *slt)
{
  if (slt != NULL)
  {
    free(slt->next);
    free(slt);
    slt = NULL;
  }
}
struct skip_list *skip_list_create(int max_level)
{

  if (max_level < 0)
  {
    max_level = 1;
  }

  max_level =
      (max_level > SKIP_LIST_MAX_LEVEL) ? SKIP_LIST_MAX_LEVEL : max_level;
  struct skip_list *st =
      (struct skip_list *)calloc(1, sizeof(struct skip_list));
  assert(st != NULL);
  st->head = (struct skip_listnode *)malloc(sizeof(struct skip_listnode));
  st->tail = (struct skip_listnode *)malloc(sizeof(struct skip_listnode));
  st->head->next = (struct skip_listnode **)malloc(
      sizeof(struct skip_listnode *) * max_level);
  for (int i = 0; i < max_level; i++)
  {
    st->head->next[i] = st->tail;
  }
  st->max_level = max_level;
  return st;
}
int skip_list_insert(struct skip_list *st, void *ptr, uint32_t key)
{

  struct skip_listnode *cur = st->head;
  struct skip_listnode *update[st->max_level];
  int level = skip_list_random_level(st->max_level);
  struct skip_listnode *slt = skip_listnode_init(level, ptr, key);
  if (slt == NULL)
  {
    return -1;
  }
  for (int i = (st->max_level - 1); i >= 0; i--)
  {
    if (cur == st->tail || cur->next[i]->key < key)
    {
      update[i] = cur;
    }
    else
    {
      while (cur != st->tail && cur->next[i]->key > key)
      {
        cur = cur->next[i];
      }
      update[i] = cur;
    }
  }
  for (int i = 0; i <= level; i++)
  {
    slt->next[i] = update[i]->next[i];
    update[i]->next[i] = slt;
  }
  return 0;
}
int skip_list_remove(struct skip_list *st, uint32_t key)
{
  struct skip_listnode *update[st->max_level];
  struct skip_listnode *cur = st->head;
  for (int i = st->max_level - 1; i >= 0; i--)
  {
    if (cur->next[i] == st->tail)
    {
      update[i] = NULL;
      continue;
    }
    while (cur->next[i] != st->tail && cur->next[i]->key > key)
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
  struct skip_listnode *temp = NULL;
  for (int i = 0; i < st->max_level; i++)
  {
    if (update[i] != NULL)
    {
      temp = update[i]->next[i];
      update[i]->next[i] = temp->next[i];
    }
  }
  if (temp == NULL)
  {
    return -1;
  }
  skip_listnode_destroy(temp);
  return 0;
}
void *skip_list_find(struct skip_list *st, uint32_t key)
{
  struct skip_listnode *cur = st->head;
  for (int i = (st->max_level - 1); i >= 0; i--)
  {
    if (cur->next[i] == st->tail)
    {
      continue;
    }
    else
    {
      while (cur->next[i] != st->tail && cur->next[i]->key > key)
      {
        cur = cur->next[i];
      }
      if (cur->next[i] != st->tail && cur->next[i]->key == key)
      {
        return cur->next[i]->ptr;
      }
    }
  }
  return NULL;
}
void skip_list_destroy(struct skip_list *st)
{

  struct skip_listnode *cur = NULL;
  while (st->head->next[0] != st->tail)
  {
    cur = st->head->next[0];
    st->head->next[0] = cur->next[0];
    skip_listnode_destroy(cur);
  }
  skip_listnode_destroy(st->head);
  skip_listnode_destroy(st->tail);
  free(st);
  st = NULL;
}
void skip_list_print(struct skip_list *st)
{
  for (int i = st->max_level - 1; i >= 0; i--)
  {
    struct skip_listnode *cur = st->head->next[i];
    if (cur != st->tail)
    {
      fprintf(stdout, "level :%d  ", i);
      while (cur != st->tail)
      {
        struct skip_listnode *next = cur->next[i];
        if (next == st->tail)
        {
          fprintf(stdout, "%d[%d]\n", cur->key, i);
        }
        else
        {
          fprintf(stdout, "%d[%d],", cur->key, i);
        }
        cur = next;
      }
    }
  }
}
#ifdef TEST
int main(void)
{
  int values[32];
  struct skip_list *st = skip_list_create(8);
  fprintf(stdout, "------insert skip_list-----------\n");
  for (int i = 0; i < 32; i++)
  {
    values[i] = rand() % 1024 + i;
    fprintf(stdout, "insert key=%d,ret = %d\n", values[i],
            skip_list_insert(st, &values[i], values[i]));
  }
  fprintf(stdout, "----------print skip_list------------\n");
  skip_list_print(st);
  fprintf(stdout, "-------find skip_lsit ------------\n");
  for (int i = 1; i < 8; i++)
  {
    int key = values[rand() % 32];
    int *ptr = skip_list_find(st, key);
    if (ptr != NULL)
    {
      fprintf(stdout, " find key=%d,*ptr=%d\n", key, *ptr);
    }
  }
  fprintf(stdout, "------delete skip_list-----------\n");
  for (int i = 1; i < 4; i++)
  {
    int key = values[rand() % 16];
    fprintf(stdout, "key=%d,ret=%d\n", key, skip_list_remove(st, key));
  }
  fprintf(stdout, "----------print skip_list------------\n");

  skip_list_print(st);

  skip_list_destroy(st);
}
#endif