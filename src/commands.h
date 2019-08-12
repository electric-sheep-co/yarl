#ifndef __YARL_COMMANDS__H__
#define __YARL_COMMANDS__H__

#include "types.h"
#include "constants.h"

#include <stdbool.h>

// any non-scalar return values are OWNED BY THE CALLER and must be free()ed

bool Redis_AUTH(RedisConnection_t conn, const char *password);

char *Redis_GET(RedisConnection_t conn, const char *key);

bool Redis_SET(RedisConnection_t conn, const char *key, const char *value);

bool Redis_DEL(RedisConnection_t conn, const char *key);

bool Redis_EXISTS(RedisConnection_t conn, const char *key);

int Redis_APPEND(RedisConnection_t conn, const char *key, const char *value);

int Redis_PUBLISH(RedisConnection_t conn, const char *channel, const char *message);

RedisArray_t *Redis_KEYS(RedisConnection_t conn, const char *pattern);

bool Redis_EXPIRE(RedisConnection_t conn, const char *key, int seconds);

bool Redis_EXPIREAT(RedisConnection_t conn, const char *key, int timestamp);

#endif // __YARL_COMMANDS__H__
