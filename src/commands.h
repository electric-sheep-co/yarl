#ifndef __YARL_COMMANDS__H__
#define __YARL_COMMANDS__H__

#include "types.h"
#include "constants.h"

RedisReturnValue Redis_AUTH(RedisConnection_t conn, const char *password);

char *Redis_GET(RedisConnection_t conn, const char *key);

bool Redis_SET(RedisConnection_t conn, const char *key, const char *value);

#endif // __YARL_COMMANDS__H__