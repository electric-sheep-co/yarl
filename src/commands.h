#ifndef __YARL_COMMANDS__H__
#define __YARL_COMMANDS__H__

#include "types.h"
#include "constants.h"

#include <stdbool.h>
#include <stdint.h>

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

// int Redis_LPUSH(RedisConnection_t conn, const char *key, uint32_t argCount, ...)
#define Redis_LPUSH(conn, key, argCount, ...) \
    _RedisCommandReturn_extractInt(_RedisCommand_issue((conn), "LPUSH", (argCount) + 1, 0x0, (key), __VA_ARGS__))

int Redis_LPUSHX(RedisConnection_t conn, const char *key, const char* value);

char* Redis_LPOP(RedisConnection_t conn, const char* key);

char* Redis_LINDEX(RedisConnection_t conn, const char* key, int index);

int Redis_LINSERT(RedisConnection_t conn, const char* key, RedisLINSERTPivot_t pivotPoint, const char* pivot, const char* value);

// use RedisConnection_getNextObject() to get messages
void Redis_SUBSCRIBE(RedisConnection_t conn, const char *channel);
void Redis_PSUBSCRIBE(RedisConnection_t conn, const char *pattern);

// for Redis_LPUSH as currently (macro) defined
extern RedisObject_t _RedisCommand_issue(RedisConnection_t, const char *, uint32_t, uint32_t, ...);
extern int _RedisCommandReturn_extractInt(RedisObject_t);

#endif // __YARL_COMMANDS__H__
