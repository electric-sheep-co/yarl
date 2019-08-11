#ifndef __YARL_TYPES__H__
#define __YARL_TYPES__H__

#include <stdbool.h>
#include <stdlib.h>

typedef int RedisConnection_t;
typedef char RedisObjectType_t;

typedef enum RedisReturnValue
{
    RedisSuccess = 0,
    RedisNotConnectedFailure = 1,
    RedisAuthFailure = 2,
} RedisReturnValue;

typedef struct RedisObject_t
{
    RedisObjectType_t type;
    void *obj;
    bool objIsOwned;
} RedisObject_t;

typedef struct RedisArray_t
{
    size_t count;
    RedisObject_t *objects;
} RedisArray_t;

#endif // __YARL_TYPES__H__