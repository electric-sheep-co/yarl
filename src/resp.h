#ifndef __YARL_RESP__H__
#define __YARL_RESP__H__

#include "types.h"

char *RedisRESP_generate(RedisObject_t obj);

char *_RedisObject_RESP__intAsStringWithLength(size_t num, size_t *outLength);

#endif // __YARL_RESP__H__