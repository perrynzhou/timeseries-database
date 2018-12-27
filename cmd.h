/*************************************************************************
  > File Name: cmd.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 四 12/27 17:35:50 2018
 ************************************************************************/

#ifndef _COMMAND_H
#define _COMMAND_H
#include "cstring.h"
struct cmd
{
  struct string name;
  int (*handle)(void *, void *, void *);
};
const struct cmd cmds[] = {
    {string("put"), NULL},
    {string("get"), NULL},
    {string("update"), NULL},
     {string("delete"), NULL},
    {string("range"), NULL}};
#endif
