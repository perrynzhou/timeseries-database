#ifndef _CONF_H_
#define _CONF_H_

#include "array.h"
#include "cstring.h"
#include "util.h"
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <yaml.h>

#define CONF_OK (void *)NULL
#define CONF_ERROR (void *)"has an invalid value"

#define CONF_ROOT_DEPTH 1
#define CONF_MAX_DEPTH (CONF_ROOT_DEPTH+1)

#define CONF_DEFAULT_ARGS 3
#define CONF_DEFAULT_POOL 8

#define CONF_UNSET_NUM -1
#define CONF_UNSET_PTR NULL

#define CONF_DEFAULT_TIMEOUT -1
#define CONF_DEFAULT_LISTEN_BACKLOG 512
#define CONF_DEFAULT_PORT 5432
#define CONF_DEFAULT_HTTP_VERSION 1
#define CONF_MAX_HTTP_VERSION 2
struct conf_pool
{
    struct string name; /* pool name (root node) */
    int port;
    struct string addr; /* listen: */
    int workers;        /*  threads worker for accept  client connection*/
    int timeout;        /* timeout: */
    int backlog;        /* backlog: */
    int max_slots;   /* memtable size in memory */
    struct string datadir;        /*http version */

};

struct conf
{
    char *fname;               /* file name (ref in argv[]) */
    FILE *fh;                  /* file handle */
    struct array arg;          /* string[] (parsed {key, value} pairs) */
    struct array pool;         /* conf_pool[] (parsed pools) */
    uint32_t depth;            /* parsed tree depth */
    yaml_parser_t parser;      /* yaml parser */
    yaml_event_t event;        /* yaml event */
    yaml_token_t token;        /* yaml token */
    unsigned seq : 1;          /* sequence? */
    unsigned valid_parser : 1; /* valid parser? */
    unsigned valid_event : 1;  /* valid event? */
    unsigned valid_token : 1;  /* valid token? */
    unsigned sound : 1;        /* sound? */
    unsigned parsed : 1;       /* parsed? */
    unsigned valid : 1;        /* valid? */
};

struct command
{
    struct string name;
    char *(*set)(struct conf *cf, struct command *cmd, void *data);
    int offset;
};

#define null_command         \
    {                        \
        null_string, NULL, 0 \
    }

char *conf_set_string(struct conf *cf, struct command *cmd, void *conf);
char *conf_set_num(struct conf *cf, struct command *cmd, void *conf);
char *conf_set_bool(struct conf *cf, struct command *cmd, void *conf);

struct conf *conf_create(char *filename,bool is_multilevel);
void conf_dump(struct conf *cf);
void conf_destroy(struct conf *cf);

#endif
