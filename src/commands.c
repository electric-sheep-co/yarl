#include "commands.h"
#include "connection.h"
#include "object.h"

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

// optional arguments must be of type void*
RedisObject_t _RedisCommand_issue(RedisConnection_t conn, const char* cmd, uint32_t argCount, uint32_t ownershipBitMask, ...)
{
    assert(cmd);

    argCount += 1;
    RedisArray_t* cmdArr = RedisArray_init(argCount);
    RedisObject_t cmdObj = {
        .type = RedisObjectType_Array,
        .obj = (void *)cmdArr,
        .objIsOwned = true };

    RedisObject_t cmdNameObj = {
        .type = RedisObjectType_BulkString,
        .obj = (void *)cmd,
        .objIsOwned = false };

    cmdArr->objects[0] = cmdNameObj;

    if (argCount > 0)
    {
        va_list args;
        va_start(args, ownershipBitMask);
        for (int i = 1; i < (argCount); i++)
        {
            RedisObject_t argObj = {
                .type = RedisObjectType_BulkString,
                .obj = (void*)va_arg(args, void*),
                .objIsOwned = (bool)(ownershipBitMask & (uint32_t)(1 << i))
            };

            cmdArr->objects[i] = argObj;
        }
        va_end(args);
    }

    RedisObject_t cmdResult = RedisConnection_sendCommand(conn, cmdObj);
    RedisObject_dealloc(cmdObj);
    return cmdResult;
}

#define REDIS_CMD__GENERIC__PREDEALLOC(conn, cmd, count, bm, endCond, failReturn, preDealloc, deallocBeforeReturn, ...) \
	RedisObject_t cmdRet = _RedisCommand_issue(conn, cmd, count, bm, ##__VA_ARGS__); \
	if (cmdRet.type == RedisObjectType_InternalError || !cmdRet.obj) return failReturn; \
	preDealloc; \
	if (deallocBeforeReturn) RedisObject_dealloc(cmdRet); \
	return endCond;

#define REDIS_CMD__GENERIC(conn, cmd, count, bm, endCond, failReturn, deallocBeforeReturn, ...) \
	RedisObject_t cmdRet = _RedisCommand_issue(conn, cmd, count, bm, ##__VA_ARGS__); \
	if (cmdRet.type == RedisObjectType_InternalError || !cmdRet.obj) return failReturn; \
	if (deallocBeforeReturn) RedisObject_dealloc(cmdRet); \
	return endCond;

#define REDIS_CMD__EXPECT_OK(conn, cmd, count, bm, ...) \
	REDIS_CMD__GENERIC__PREDEALLOC(conn, cmd, count, bm, boolVal, false, \
		bool boolVal = cmdRet.type == RedisObjectType_SimpleString && cmdRet.obj && \
			strncmp((const char *)cmdRet.obj, "OK", strlen("OK")) == 0, true, ##__VA_ARGS__)

#define REDIS_CMD__EXPECT_BOOL(conn, cmd, count, bm, ...) \
	REDIS_CMD__GENERIC__PREDEALLOC(conn, cmd, count, bm, boolVal, false, \
		bool boolVal = cmdRet.type == RedisObjectType_Integer && (bool)*(int*)cmdRet.obj, true, ##__VA_ARGS__)

#define REDIS_CMD__EXPECT_INT(conn, cmd, count, bm, ...) \
	REDIS_CMD__GENERIC__PREDEALLOC(conn, cmd, count, bm, intVal, -1, \
		int intVal = *(int*)cmdRet.obj, true, ##__VA_ARGS__)

bool Redis_AUTH(RedisConnection_t conn, const char *password)
{
    REDIS_CMD__EXPECT_OK(conn, "AUTH", 1, 0, password);
}

char *Redis_GET(RedisConnection_t conn, const char *key)
{
    REDIS_CMD__GENERIC(conn, "GET", 1, 0, (char*)cmdRet.obj, NULL, false, key);
}

bool Redis_SET(RedisConnection_t conn, const char *key, const char *value)
{
    REDIS_CMD__EXPECT_OK(conn, "SET", 2, 0, key, value);
}

bool Redis_DEL(RedisConnection_t conn, const char *key)
{
    REDIS_CMD__EXPECT_BOOL(conn, "DEL", 1, 0, key);
}

bool Redis_EXISTS(RedisConnection_t conn, const char *key)
{
    REDIS_CMD__EXPECT_BOOL(conn, "EXISTS", 1, 0, key);
}

int Redis_APPEND(RedisConnection_t conn, const char *key, const char *value)
{
    REDIS_CMD__EXPECT_INT(conn, "APPEND", 2, 0, key, value);
}
