#ifndef __YARL_CONSTANTS__H__
#define __YARL_CONSTANTS__H__

#include "types.h"

#define CRLF "\r\n"

#define RedisObjectType_NoType (RedisObjectType_t)'\0'
#define RedisObjectType_SimpleString (RedisObjectType_t)'+'
#define RedisObjectType_Error (RedisObjectType_t)'-'
#define RedisObjectType_Integer (RedisObjectType_t)':'
#define RedisObjectType_BulkString (RedisObjectType_t)'$'
#define RedisObjectType_Array (RedisObjectType_t)'*'
#define RedisObjectType_InternalError (RedisObjectType_t)'!'

#define RedisObject_TypeToString(type) \
    ((type) == RedisObjectType_NoType ? "NoType" : \
    ((type) == RedisObjectType_SimpleString ? "SimpleString" : \
    ((type) == RedisObjectType_Error ? "Error" : \
    ((type) == RedisObjectType_Integer ? "Integer" : \
    ((type) == RedisObjectType_BulkString ? "BulkString" : \
    ((type) == RedisObjectType_Array ? "Array" : \
    ((type) == RedisObjectType_InternalError ? "LibInternalError" : "UnknownRedisObjectType")))))))

#endif // __YARL_CONSTANTS__H__