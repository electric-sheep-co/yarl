#ifndef __YARL_CONNECTION__H__
#define __YARL_CONNECTION__H__

#include "types.h"

RedisConnection_t RedisConnection_open(const char *host, const char *port);

RedisObject_t RedisConnection_sendCommand(RedisConnection_t conn, RedisObject_t cmdArrayObj);

RedisObject_t RedisConnection_getNextObject(RedisConnection_t conn);

#endif // __YARL_CONNECTION__H__