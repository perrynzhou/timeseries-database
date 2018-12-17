#include "conf.h"
#include "log.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
static struct command conf_commands[] = {
    {string("addr"), conf_set_string, offsetof(struct conf_pool, addr)},
    {string("port"), conf_set_num, offsetof(struct conf_pool, port)},

    {string("timeout"), conf_set_num, offsetof(struct conf_pool, timeout)},

    {string("workers"), conf_set_num, offsetof(struct conf_pool, workers)},
    {string("backlog"), conf_set_num, offsetof(struct conf_pool, backlog)},
    {string("datadir"), conf_set_string, offsetof(struct conf_pool, datadir)},
    {string("max_slots"), conf_set_num, offsetof(struct conf_pool, max_slots)},
    
    null_command};
static int conf_pool_init(struct conf_pool *cp, struct string *name)
{
  int status;

  string_init(&cp->name);
  cp->timeout = CONF_UNSET_NUM;
  cp->backlog = CONF_UNSET_NUM;
  cp->workers = CONF_UNSET_NUM;
  cp->port = CONF_UNSET_NUM;
  string_init(&cp->datadir);
  string_init(&cp->addr);
  status = string_duplicate(&cp->name, name);
  if (status != 0)
  {
    return status;
  }
  string_init(&cp->addr);

  log_debug(LOG_VVERB, "init conf pool %p, '%.*s'", cp, name->len, name->data);

  return 0;
}

static void conf_pool_deinit(struct conf_pool *cp)
{
  string_deinit(&cp->name);
  string_deinit(&cp->addr);
  string_deinit(&cp->datadir);
  log_debug(LOG_VVERB, "deinit conf pool %p", cp);
}

void conf_dump(struct conf *cf)
{
  uint32_t i, j, npool, nserver;
  struct conf_pool *cp;
  struct string *s;

  npool = array_n(&cf->pool);
  if (npool == 0)
  {
    return;
  }

  log_info(LOG_INFO, "%d  pools in configuration file '%s'", npool, cf->fname);

  for (i = 0; i < npool; i++)
  {
    cp = array_get(&cf->pool, i);

    log_info(LOG_INFO, "%.*s", cp->name.len, cp->name.data);
    log_info(LOG_INFO, "%.*s", cp->addr.len, cp->addr.data);
    log_info(LOG_INFO, "%.*s", cp->datadir.len, cp->datadir.data);
    log_info(LOG_INFO, "  port: %d", cp->port);
    log_info(LOG_INFO, "  timeout: %d", cp->timeout);
    log_info(LOG_INFO, "  backlog: %d", cp->backlog);
    log_info(LOG_INFO, "  workers: %d", cp->workers);
  }
}

static int conf_yaml_init(struct conf *cf)
{
  int rv;

  ASSERT(!cf->valid_parser);

  rv = fseek(cf->fh, 0L, SEEK_SET);
  if (rv < 0)
  {
    log_error("conf: failed to seek to the beginning of file '%s': %s",
              cf->fname, strerror(errno));
    return -1;
  }

  rv = yaml_parser_initialize(&cf->parser);
  if (!rv)
  {
    log_error("conf: failed (err %d) to initialize yaml parser",
              cf->parser.error);
    return -1;
  }

  yaml_parser_set_input_file(&cf->parser, cf->fh);
  cf->valid_parser = 1;

  return 0;
}

static void conf_yaml_deinit(struct conf *cf)
{
  if (cf->valid_parser)
  {
    yaml_parser_delete(&cf->parser);
    cf->valid_parser = 0;
  }
}

static int conf_token_next(struct conf *cf)
{
  int rv;

  ASSERT(cf->valid_parser && !cf->valid_token);

  rv = yaml_parser_scan(&cf->parser, &cf->token);
  if (!rv)
  {
    log_error("conf: failed (err %d) to scan next token", cf->parser.error);
    return -1;
  }
  cf->valid_token = 1;

  return 0;
}

static void conf_token_done(struct conf *cf)
{
  ASSERT(cf->valid_parser);

  if (cf->valid_token)
  {
    yaml_token_delete(&cf->token);
    cf->valid_token = 0;
  }
}

static int conf_event_next(struct conf *cf)
{
  int rv;

  ASSERT(cf->valid_parser && !cf->valid_event);

  rv = yaml_parser_parse(&cf->parser, &cf->event);
  if (!rv)
  {
    log_error("conf: failed (err %d) to get next event", cf->parser.error);
    return -1;
  }
  cf->valid_event = 1;

  return 0;
}

static void conf_event_done(struct conf *cf)
{
  if (cf->valid_event)
  {
    yaml_event_delete(&cf->event);
    cf->valid_event = 0;
  }
}

static int conf_push_scalar(struct conf *cf)
{
  int status;
  struct string *value;
  uint8_t *scalar;
  uint32_t scalar_len;

  scalar = cf->event.data.scalar.value;
  scalar_len = (uint32_t)cf->event.data.scalar.length;
  if (scalar_len == 0)
  {
    return -1;
  }

  log_debug(LOG_VVERB, "push '%.*s'", scalar_len, scalar);

  value = array_push(&cf->arg);
  if (value == NULL)
  {
    return -3;
  }
  string_init(value);

  status = string_copy(value, scalar, scalar_len);
  if (status != 0)
  {
    array_pop(&cf->arg);
    return status;
  }

  return 0;
}

static void conf_pop_scalar(struct conf *cf)
{
  struct string *value;

  value = array_pop(&cf->arg);
  log_debug(LOG_VVERB, "pop '%.*s'", value->len, value->data);
  string_deinit(value);
}

static int conf_handler(struct conf *cf, void *data)
{
  struct command *cmd;
  struct string *key, *value;
  uint32_t narg;

  if (array_n(&cf->arg) == 1)
  {
    value = array_top(&cf->arg);
    log_debug(LOG_VVERB, "conf handler on '%.*s'", value->len, value->data);
    return conf_pool_init(data, value);
  }

  narg = array_n(&cf->arg);
  value = array_get(&cf->arg, narg - 1);
  key = array_get(&cf->arg, narg - 2);

  log_debug(LOG_VVERB, "conf handler on %.*s: %.*s", key->len, key->data,
            value->len, value->data);

  for (cmd = conf_commands; cmd->name.len != 0; cmd++)
  {
    char *rv;

    if (string_compare(key, &cmd->name) != 0)
    {
      continue;
    }
    rv = cmd->set(cf, cmd, data);
    if (rv != CONF_OK)
    {
      log_error("conf: directive \"%.*s\" %s", key->len, key->data, rv);
      return -1;
    }

    return 0;
  }
  return 0;
}

static int conf_begin_parse(struct conf *cf)
{
  int status;
  bool done;

  ASSERT(cf->sound && !cf->parsed);
  ASSERT(cf->depth == 0);

  status = conf_yaml_init(cf);
  if (status != 0)
  {
    return status;
  }

  done = false;
  do
  {
    status = conf_event_next(cf);
    if (status != 0)
    {
      return status;
    }

    log_debug(LOG_VVERB, "next begin event %d", cf->event.type);

    switch (cf->event.type)
    {
    case YAML_STREAM_START_EVENT:
    case YAML_DOCUMENT_START_EVENT:
      break;

    case YAML_MAPPING_START_EVENT:
      ASSERT(cf->depth < CONF_MAX_DEPTH);
      cf->depth++;
      done = true;
      break;

    default:
      NOT_REACHED();
    }

    conf_event_done(cf);

  } while (!done);

  return 0;
}

static int conf_end_parse(struct conf *cf)
{
  int status;
  bool done;

  ASSERT(cf->sound && !cf->parsed);
  ASSERT(cf->depth == 0);

  done = false;
  do
  {
    status = conf_event_next(cf);
    if (status != 0)
    {
      return status;
    }

    log_debug(LOG_VVERB, "next end event %d", cf->event.type);

    switch (cf->event.type)
    {
    case YAML_STREAM_END_EVENT:
      done = true;
      break;

    case YAML_DOCUMENT_END_EVENT:
      break;

    default:
      NOT_REACHED();
    }

    conf_event_done(cf);
  } while (!done);

  conf_yaml_deinit(cf);

  return 0;
}

static int conf_parse_core(struct conf *cf, void *data)
{
  int status;
  bool done, leaf, new_pool;

  ASSERT(cf->sound);

  status = conf_event_next(cf);
  if (status != 0)
  {
    return status;
  }

  log_debug(LOG_INFO, "next event %d depth %" PRIu32 " seq %d", cf->event.type,
            cf->depth, cf->seq);

  done = false;
  leaf = false;
  new_pool = false;

  switch (cf->event.type)
  {
  case YAML_MAPPING_END_EVENT:
    cf->depth--;
    if (cf->depth == 1)
    {
      conf_pop_scalar(cf);
    }
    else if (cf->depth == 0)
    {
      done = true;
    }
    break;

  case YAML_MAPPING_START_EVENT:
    cf->depth++;
    break;

  case YAML_SEQUENCE_START_EVENT:
    cf->seq = 1;
    break;

  case YAML_SEQUENCE_END_EVENT:
    conf_pop_scalar(cf);
    cf->seq = 0;
    break;

  case YAML_SCALAR_EVENT:
    status = conf_push_scalar(cf);
    if (status != 0)
    {
      break;
    }

    /* take appropriate action */
    if (cf->seq)
    {
      /* for a sequence, leaf is at CONF_MAX_DEPTH */
      ASSERT(cf->depth == CONF_MAX_DEPTH);
      leaf = true;
    }
    else if (cf->depth == CONF_ROOT_DEPTH)
    {
      /* create new conf_pool */
      data = array_push(&cf->pool);
      if (data == NULL)
      {
        status = -3;
        break;
      }
      new_pool = true;
    }
    else if (array_n(&cf->arg) == cf->depth + 1)
    {
      /* for {key: value}, leaf is at CONF_MAX_DEPTH */
      ASSERT(cf->depth == CONF_MAX_DEPTH);
      leaf = true;
    }
    break;

  default:
    NOT_REACHED();
    break;
  }

  conf_event_done(cf);

  if (status != 0)
  {
    return status;
  }

  if (done)
  {
    /* terminating condition */
    return 0;
  }

  if (leaf || new_pool)
  {
    status = conf_handler(cf, data);

    if (leaf)
    {
      conf_pop_scalar(cf);
      if (!cf->seq)
      {
        conf_pop_scalar(cf);
      }
    }

    if (status != 0)
    {
      return status;
    }
  }
  return conf_parse_core(cf, data);
}

static int conf_parse(struct conf *cf)
{
  int status;

  ASSERT(cf->sound && !cf->parsed);
  ASSERT(array_n(&cf->arg) == 0);

  status = conf_begin_parse(cf);
  if (status != 0)
  {
    return status;
  }

  status = conf_parse_core(cf, NULL);
  if (status != 0)
  {
    return status;
  }

  status = conf_end_parse(cf);
  if (status != 0)
  {
    return status;
  }

  cf->parsed = 1;

  return 0;
}

static struct conf *conf_open(char *filename)
{
  int status;
  struct conf *cf;
  FILE *fh;

  fh = fopen(filename, "r");
  if (fh == NULL)
  {
    log_error("conf: failed to open configuration '%s': %s", filename,
              strerror(errno));
    return NULL;
  }

  cf = calloc(1, sizeof(*cf));
  if (cf == NULL)
  {
    fclose(fh);
    return NULL;
  }

  status = array_init(&cf->arg, CONF_DEFAULT_ARGS, sizeof(struct string));
  if (status != 0)
  {
    free(cf);
    fclose(fh);
    return NULL;
  }

  status = array_init(&cf->pool, CONF_DEFAULT_POOL, sizeof(struct conf_pool));
  if (status != 0)
  {
    array_deinit(&cf->arg);
    free(cf);
    fclose(fh);
    return NULL;
  }

  cf->fname = filename;
  cf->fh = fh;
  cf->depth = 0;
  /* parser, event, and token are initialized later */
  cf->seq = 0;
  cf->valid_parser = 0;
  cf->valid_event = 0;
  cf->valid_token = 0;
  cf->sound = 0;
  cf->parsed = 0;
  cf->valid = 0;

  log_debug(LOG_VVERB, "opened conf '%s'", filename);

  return cf;
}

static int conf_validate_document(struct conf *cf)
{
  int status;
  uint32_t count;
  bool done;

  status = conf_yaml_init(cf);
  if (status != 0)
  {
    return status;
  }

  count = 0;
  done = false;
  do
  {
    yaml_document_t document;
    yaml_node_t *node;
    int rv;

    rv = yaml_parser_load(&cf->parser, &document);
    if (!rv)
    {
      log_error("conf: failed (err %d) to get the next yaml document",
                cf->parser.error);
      conf_yaml_deinit(cf);
      return -1;
    }

    node = yaml_document_get_root_node(&document);
    if (node == NULL)
    {
      done = true;
    }
    else
    {
      count++;
    }

    yaml_document_delete(&document);
  } while (!done);

  conf_yaml_deinit(cf);

  if (count != 1)
  {
    log_error("conf: '%s' must contain only 1 document; found %d documents",
              cf->fname, count);
    return -1;
  }

  return 0;
}

static int conf_validate_tokens(struct conf *cf)
{
  int status;
  bool done, error;
  int type;

  status = conf_yaml_init(cf);
  if (status != 0)
  {
    return status;
  }

  done = false;
  error = false;
  do
  {
    status = conf_token_next(cf);
    if (status != 0)
    {
      return status;
    }
    type = cf->token.type;

    switch (type)
    {
    case YAML_NO_TOKEN:
      error = true;
      log_error("conf: no token (%d) is disallowed", type);
      break;

    case YAML_VERSION_DIRECTIVE_TOKEN:
      error = true;
      log_error("conf: version directive token (%d) is disallowed", type);
      break;

    case YAML_TAG_DIRECTIVE_TOKEN:
      error = true;
      log_error("conf: tag directive token (%d) is disallowed", type);
      break;

    case YAML_DOCUMENT_START_TOKEN:
      error = true;
      log_error("conf: document start token (%d) is disallowed", type);
      break;

    case YAML_DOCUMENT_END_TOKEN:
      error = true;
      log_error("conf: document end token (%d) is disallowed", type);
      break;

    case YAML_FLOW_SEQUENCE_START_TOKEN:
      error = true;
      log_error("conf: flow sequence start token (%d) is disallowed", type);
      break;

    case YAML_FLOW_SEQUENCE_END_TOKEN:
      error = true;
      log_error("conf: flow sequence end token (%d) is disallowed", type);
      break;

    case YAML_FLOW_MAPPING_START_TOKEN:
      error = true;
      log_error("conf: flow mapping start token (%d) is disallowed", type);
      break;

    case YAML_FLOW_MAPPING_END_TOKEN:
      error = true;
      log_error("conf: flow mapping end token (%d) is disallowed", type);
      break;

    case YAML_FLOW_ENTRY_TOKEN:
      error = true;
      log_error("conf: flow entry token (%d) is disallowed", type);
      break;

    case YAML_ALIAS_TOKEN:
      error = true;
      log_error("conf: alias token (%d) is disallowed", type);
      break;

    case YAML_ANCHOR_TOKEN:
      error = true;
      log_error("conf: anchor token (%d) is disallowed", type);
      break;

    case YAML_TAG_TOKEN:
      error = true;
      log_error("conf: tag token (%d) is disallowed", type);
      break;

    case YAML_BLOCK_SEQUENCE_START_TOKEN:
    case YAML_BLOCK_MAPPING_START_TOKEN:
    case YAML_BLOCK_END_TOKEN:
    case YAML_BLOCK_ENTRY_TOKEN:
      break;

    case YAML_KEY_TOKEN:
    case YAML_VALUE_TOKEN:
    case YAML_SCALAR_TOKEN:
      break;

    case YAML_STREAM_START_TOKEN:
      break;

    case YAML_STREAM_END_TOKEN:
      done = true;
      log_debug(LOG_VVERB, "conf '%s' has valid tokens", cf->fname);
      break;

    default:
      error = true;
      log_error("conf: unknown token (%d) is disallowed", type);
      break;
    }

    conf_token_done(cf);
  } while (!done && !error);

  conf_yaml_deinit(cf);

  return !error ? 0 : -1;
}

static int conf_validate_structure(struct conf *cf)
{
  int status;
  int type, depth;
  uint32_t i, count[CONF_MAX_DEPTH + 1];
  bool done, error, seq;

  status = conf_yaml_init(cf);
  if (status != 0)
  {
    return status;
  }

  done = false;
  error = false;
  seq = false;
  depth = 0;
  for (i = 0; i < CONF_MAX_DEPTH + 1; i++)
  {
    count[i] = 0;
  }

  /*
   * Validate that the configuration conforms roughly to the following
   * yaml tree structure:
   *
   * keyx:
   *   key1: value1
   *   key2: value2
   *   seq:
   *     - elem1
   *     - elem2
   *     - elem3
   *   key3: value3
   *
   * keyy:
   *   key1: value1
   *   key2: value2
   *   seq:
   *     - elem1
   *     - elem2
   *     - elem3
   *   key3: value3
   */
  do
  {
    status = conf_event_next(cf);
    if (status != 0)
    {
      return status;
    }

    type = cf->event.type;

    log_debug(LOG_VVERB, "next event %d depth %d seq %d", type, depth, seq);

    switch (type)
    {
    case YAML_STREAM_START_EVENT:
    case YAML_DOCUMENT_START_EVENT:
      break;

    case YAML_DOCUMENT_END_EVENT:
      break;

    case YAML_STREAM_END_EVENT:
      done = true;
      break;

    case YAML_MAPPING_START_EVENT:
      if (depth == CONF_ROOT_DEPTH && count[depth] != 1)
      {
        error = true;
        log_error("conf: '%s' has more than one \"key:value\" at depth"
                  " %d",
                  cf->fname, depth);
      }
      else if (depth >= CONF_MAX_DEPTH)
      {
        error = true;
        log_error("conf: '%s' has a depth greater than %d", cf->fname,
                  CONF_MAX_DEPTH);
      }
      depth++;
      break;

    case YAML_MAPPING_END_EVENT:
      if (depth == CONF_MAX_DEPTH)
      {
        if (seq)
        {
          seq = false;
        }
        else
        {
          error = true;
          log_error("conf: '%s' missing sequence directive at depth "
                    "%d",
                    cf->fname, depth);
        }
      }
      depth--;
      count[depth] = 0;
      break;

    case YAML_SEQUENCE_START_EVENT:
      if (seq)
      {
        error = true;
        log_error("conf: '%s' has more than one sequence directive", cf->fname);
      }
      else if (depth != CONF_MAX_DEPTH)
      {
        error = true;
        log_error("conf: '%s' has sequence at depth %d instead of %d",
                  cf->fname, depth, CONF_MAX_DEPTH);
      }
      else if (count[depth] != 1)
      {
        error = true;
        log_error("conf: '%s' has invalid \"key:value\" at depth %d", cf->fname,
                  depth);
      }
      seq = true;
      break;

    case YAML_SEQUENCE_END_EVENT:
      ASSERT(depth == CONF_MAX_DEPTH);
      count[depth] = 0;
      break;

    case YAML_SCALAR_EVENT:
      if (depth == 0)
      {
        error = true;
        log_error("conf: '%s' has invalid empty \"key:\" at depth %d",
                  cf->fname, depth);
      }
      else if (depth == CONF_ROOT_DEPTH && count[depth] != 0)
      {
        error = true;
        log_error("conf: '%s' has invalid mapping \"key:\" at depth %d",
                  cf->fname, depth);
      }
      else if (depth == CONF_MAX_DEPTH && count[depth] == 2)
      {
        /* found a "key: value", resetting! */
        count[depth] = 0;
      }
      count[depth]++;
      break;

    default:
      NOT_REACHED();
    }

    conf_event_done(cf);
  } while (!done && !error);

  conf_yaml_deinit(cf);

  return !error ? 0 : -1;
}

static int conf_pre_validate(struct conf *cf, bool is_multilevel)
{
  int status;

  status = conf_validate_document(cf);
  if (status != 0)
  {
    return status;
  }

  status = conf_validate_tokens(cf);
  if (status != 0)
  {
    return status;
  }

  /* invalid check yaml structure */

  if (is_multilevel)
  {
    status = conf_validate_structure(cf);
    if (status != 0)
    {
      return status;
    }
  }
  cf->sound = 1;

  return 0;
}

static int conf_pool_name_cmp(const void *t1, const void *t2)
{
  const struct conf_pool *p1 = t1, *p2 = t2;

  return string_compare(&p1->name, &p2->name);
}

static int conf_pool_addr_cmp(const void *t1, const void *t2)
{
  const struct conf_pool *p1 = t1, *p2 = t2;

  return string_compare(&p1->addr, &p2->addr);
}

static int conf_validate_pool(struct conf *cf, struct conf_pool *cp)
{
  int status;

  ASSERT(!string_empty(&cp->name));

  if (cp->timeout == CONF_UNSET_NUM)
  {
    cp->timeout = CONF_DEFAULT_TIMEOUT;
  }
  if (cp->backlog == CONF_UNSET_NUM)
  {
    cp->backlog = CONF_DEFAULT_LISTEN_BACKLOG;
  }
  if (cp->port == CONF_UNSET_NUM)
  {
    cp->port = CONF_DEFAULT_PORT;
  }
  return 0;
}

static int conf_post_validate(struct conf *cf)
{
  int status;
  uint32_t i, npool;
  bool valid;

  ASSERT(cf->sound && cf->parsed);
  ASSERT(!cf->valid);

  npool = array_n(&cf->pool);
  if (npool == 0)
  {
    log_error("conf: '%.*s' has no pools", cf->fname);
    return -1;
  }

  /* validate pool */
  for (i = 0; i < npool; i++)
  {
    struct conf_pool *cp = array_get(&cf->pool, i);

    status = conf_validate_pool(cf, cp);
    if (status != 0)
    {
      return status;
    }
  }

  /* disallow pools with duplicate listen: key values */
  array_sort(&cf->pool, conf_pool_addr_cmp);
  for (valid = true, i = 0; i < npool - 1; i++)
  {
    struct conf_pool *p1, *p2;

    p1 = array_get(&cf->pool, i);
    p2 = array_get(&cf->pool, i + 1);

    if (string_compare(&p1->addr, &p2->addr) == 0 && p1->port == p2->port)
    {
      log_error("conf: pools '%.*s' and '%.*s' have the same listen address '%s:%d'",
                p1->name.len, p1->name.data, p2->name.len, p2->name.data,
                p1->addr.data, p1->port);
      valid = false;
      break;
    }
  }
  if (!valid)
  {
    return -1;
  }

  /* disallow pools with duplicate names */
  array_sort(&cf->pool, conf_pool_name_cmp);
  for (valid = true, i = 0; i < npool - 1; i++)
  {
    struct conf_pool *p1, *p2;

    p1 = array_get(&cf->pool, i);
    p2 = array_get(&cf->pool, i + 1);

    if (string_compare(&p1->name, &p2->name) == 0)
    {
      log_error("conf: '%s' has pools with same name %.*s'", cf->fname,
                p1->name.len, p1->name.data);
      valid = false;
      break;
    }
  }
  if (!valid)
  {
    return -1;
  }

  return 0;
}

struct conf *conf_create(char *filename, bool is_multilevel)
{
  int status;
  struct conf *cf;

  cf = conf_open(filename);
  if (cf == NULL)
  {
    return NULL;
  }

  /* validate configuration file before parsing */
  status = conf_pre_validate(cf, is_multilevel);

  if (status != 0)
  {
    goto error;
  }

  /* parse the configuration file */
  status = conf_parse(cf);
  if (status != 0)
  {
    goto error;
  }

  /* validate parsed configuration */
  status = conf_post_validate(cf);
  if (status != 0)
  {
    goto error;
  }

  log_level_set(LOG_DEBUG);

  fclose(cf->fh);
  cf->fh = NULL;

  return cf;

error:
  log_stderr("proxy: configuration file '%s' syntax is invalid", filename);
  fclose(cf->fh);
  cf->fh = NULL;
  conf_destroy(cf);
  return NULL;
}

void conf_destroy(struct conf *cf)
{
  while (array_n(&cf->arg) != 0)
  {
    conf_pop_scalar(cf);
  }
  array_deinit(&cf->arg);

  while (array_n(&cf->pool) != 0)
  {
    conf_pool_deinit(array_pop(&cf->pool));
  }
  array_deinit(&cf->pool);

  free(cf);
}

char *conf_set_string(struct conf *cf, struct command *cmd, void *conf)
{
  int status;
  uint8_t *p;
  struct string *field, *value;

  p = conf;
  field = (struct string *)(p + cmd->offset);

  if (field->data != CONF_UNSET_PTR)
  {
    return "is a duplicate";
  }

  value = array_top(&cf->arg);

  status = string_duplicate(field, value);
  if (status != 0)
  {
    return CONF_ERROR;
  }

  return CONF_OK;
}

char *conf_set_num(struct conf *cf, struct command *cmd, void *conf)
{
  uint8_t *p;
  int num, *np;
  struct string *value;

  p = conf;
  np = (int *)(p + cmd->offset);

  if (*np != CONF_UNSET_NUM)
  {
    return "is a duplicate";
  }

  value = array_top(&cf->arg);

  num = str_atoi(value->data, value->len);
  if (num < 0)
  {
    return "is not a number";
  }

  *np = num;

  return CONF_OK;
}

char *conf_set_bool(struct conf *cf, struct command *cmd, void *conf)
{
  uint8_t *p;
  int *bp;
  struct string *value, true_str, false_str;

  p = conf;
  bp = (int *)(p + cmd->offset);

  if (*bp != CONF_UNSET_NUM)
  {
    return "is a duplicate";
  }

  value = array_top(&cf->arg);
  string_set_text(&true_str, "true");
  string_set_text(&false_str, "false");

  if (string_compare(value, &true_str) == 0)
  {
    *bp = 1;
  }
  else if (string_compare(value, &false_str) == 0)
  {
    *bp = 0;
  }
  else
  {
    return "is not \"true\" or \"false\"";
  }

  return CONF_OK;
}

char *conf_set_hash(struct conf *cf, struct command *cmd, void *conf)
{
  int status;
  uint8_t *p;
  struct string *field, *value;

  p = conf;
  field = (struct string *)(p + cmd->offset);

  if (field->data != CONF_UNSET_PTR)
  {
    return "is a duplicate";
  }

  value = array_top(&cf->arg);

  status = string_duplicate(field, value);
  if (status != 0)
  {
    return CONF_ERROR;
  }

  return CONF_OK;
}
char *conf_set_service_type(struct conf *cf, struct command *cmd, void *conf)
{
  int status;
  uint8_t *p;
  struct string *field, *value;

  p = conf;
  field = (struct string *)(p + cmd->offset);

  if (field->data != CONF_UNSET_PTR)
  {
    return "is a duplicate";
  }

  value = array_top(&cf->arg);

  status = string_duplicate(field, value);
  if (status != 0)
  {
    return CONF_ERROR;
  }

  return CONF_OK;
}
#ifdef CONF_TEST
int main(void)
{
  log_init(LOG_INFO, NULL);
  struct conf *f = conf_create("./test.yml", false);
  if (f != NULL)
  {
    conf_dump(f);
    conf_destroy(f);
  }
}
#endif