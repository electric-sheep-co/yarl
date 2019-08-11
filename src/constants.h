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

#endif // __YARL_CONSTANTS__H__