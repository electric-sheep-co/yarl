#ifndef __YARL_H__
#define __YARL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define YARL_VERSION "0.4.0"

#include "commands.h"
#include "constants.h"

// object.h
extern void RedisObject_dealloc(RedisObject_t);
extern void RedisArray_dealloc(RedisArray_t *);

// connection.h
extern RedisObject_t RedisConnection_getNextObject(RedisConnection_t conn);

#ifdef __cplusplus
}
#endif

#endif // __YARL_H__
