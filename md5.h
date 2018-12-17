/*************************************************************************
  > File Name: md5.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 二 11/20 13:07:19 2018
 ************************************************************************/

#ifndef _MD5_H
#define _MD5_H
#include <stdint.h>
#include <stdio.h>
typedef struct {
    unsigned int lo, hi;
    unsigned int a, b, c, d;
    unsigned char buffer[64];
    unsigned int block[16];
} md5_context;
void
md5_init(md5_context *ctx);

void
md5_update(md5_context *ctx, void *data, unsigned long size);

void
md5_final(unsigned char *result, md5_context *ctx);
void
md5_signature(unsigned char *key, unsigned long length, unsigned char *result);
uint32_t
hash_md5(const char *key, size_t key_length);

#endif
