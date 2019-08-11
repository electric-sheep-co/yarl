#ifndef __YARL_OBJECT__H__
#define __YARL_OBJECT__H__

#include "types.h"

RedisObject_t RedisObject_parseSimpleString(RedisConnection_t);
RedisObject_t RedisObject_parseBulkString(RedisConnection_t);
RedisObject_t RedisObject_parseInteger(RedisConnection_t);
RedisObject_t RedisObject_parseArray(RedisConnection_t);
RedisObject_t RedisObject_parseError(RedisConnection_t);

void RedisObject_dealloc(RedisObject_t);

RedisArray_t *RedisArray_init(size_t);
void RedisArray_dealloc(RedisArray_t *);

#endif // __YARL_OBJECT__H__