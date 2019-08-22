#include "commands.h"
#include "connection.h"
#include "object.h"
#include "resp.h"
#include "log.h"

#include <string.h>
#include <stdarg.h>
#include <assert.h>

// optional arguments must all be of type void* (well, really just any pointer type will do)
// ownershipBitMask allows marking which of the optional arguments is owned: the index for which the argument is passed is
// checked against that bit in ownershipBitMask, and if set is marked as "owned" by us
RedisObject_t _RedisCommand_issue(RedisConnection_t, const char *, uint32_t, uint32_t, ...);
RedisObject_t _RedisCommand_issue(RedisConnection_t conn, const char *cmd, uint32_t argCount, uint32_t ownershipBitMask, ...)
{
    assert(cmd);

    // 'argCount + 1' to account for the cmd object that must always be included
    RedisArray_t *cmdArr = RedisArray_init(argCount + 1);
    RedisObject_t cmdObj = {
        .type = RedisObjectType_Array,
        .obj = (void *)cmdArr,
        .objIsOwned = true};

    RedisObject_t cmdNameObj = {
        .type = RedisObjectType_BulkString,
        .obj = (void *)cmd,
        .objIsOwned = false};

    cmdArr->objects[0] = cmdNameObj;

    if (argCount)
    {
        va_list args;
        va_start(args, ownershipBitMask);
        for (int i = 0; i < argCount; i++)
        {
            RedisObject_t argObj = {
                .type = RedisObjectType_BulkString,
                .obj = (void *)va_arg(args, void *),
                .objIsOwned = (bool)(ownershipBitMask & (uint32_t)(1 << i))};

            cmdArr->objects[i + 1] = argObj;
        }
        va_end(args);
    }

    RedisObject_t cmdResult = RedisConnection_sendCommand(conn, cmdObj);
    RedisObject_dealloc(cmdObj);
    return cmdResult;
}
bool _RedisCommandReturn__typeCheck(RedisObject_t, RedisObjectType_t);
bool _RedisCommandReturn__typeCheck(RedisObject_t cmdRet, RedisObjectType_t expectType)
{
    return cmdRet.type != RedisObjectType_InternalError && cmdRet.type == expectType && cmdRet.obj;
}

#define REDIS_CMD__GENERIC(expectType, conn, cmd, count, bm, successReturn, failReturn, dealloc, ...) \
    RedisObject_t cmdRet = _RedisCommand_issue(conn, cmd, count, bm, ##__VA_ARGS__);                  \
                                                                                                      \
    if (dealloc)                                                                                      \
        RedisObject_dealloc(cmdRet);                                                                  \
                                                                                                      \
    return _RedisCommandReturn__typeCheck(cmdRet, expectType) ? successReturn : failReturn;

#define _RedisCommandReturn_(name, retType, exType, failReturn, dealloc, extractSt) \
    retType _RedisCommandReturn_##name(RedisObject_t);                              \
    retType _RedisCommandReturn_##name(RedisObject_t cmdRet)                        \
    {                                                                               \
        if (!_RedisCommandReturn__typeCheck(cmdRet, exType))                        \
        {                                                                           \
            RedisLog("_RedisCommandReturn__typeCheck failed: %d\n", exType);        \
            return failReturn;                                                      \
        }                                                                           \
                                                                                    \
        retType retVal = extractSt;                                                 \
                                                                                    \
        if (dealloc)                                                                \
            RedisObject_dealloc(cmdRet);                                            \
                                                                                    \
        return retVal;                                                              \
    }

_RedisCommandReturn_(extractBool, bool, RedisObjectType_Integer, false, true, (bool)*(int *)cmdRet.obj);

_RedisCommandReturn_(extractInt, int, RedisObjectType_Integer, -1, true, *(int *)cmdRet.obj);

_RedisCommandReturn_(isOKString, bool, RedisObjectType_SimpleString, false, true,
                     strncmp((const char *)cmdRet.obj, "OK", strlen("OK")) == 0);

bool Redis_AUTH(RedisConnection_t conn, const char *password)
{
    return _RedisCommandReturn_isOKString(_RedisCommand_issue(conn, "AUTH", 1, 0, password));
}

char *Redis_GET(RedisConnection_t conn, const char *key)
{
    REDIS_CMD__GENERIC(RedisObjectType_BulkString, conn, "GET", 1, 0, (char *)cmdRet.obj, NULL, false, key);
}

bool Redis_SET(RedisConnection_t conn, const char *key, const char *value)
{
    return _RedisCommandReturn_isOKString(_RedisCommand_issue(conn, "SET", 2, 0, key, value));
}

bool Redis_DEL(RedisConnection_t conn, const char *key)
{
    return _RedisCommandReturn_extractBool(_RedisCommand_issue(conn, "DEL", 1, 0, key));
}

bool Redis_EXISTS(RedisConnection_t conn, const char *key)
{
    return _RedisCommandReturn_extractBool(_RedisCommand_issue(conn, "EXISTS", 1, 0, key));
}

int Redis_APPEND(RedisConnection_t conn, const char *key, const char *value)
{
    return _RedisCommandReturn_extractInt(_RedisCommand_issue(conn, "APPEND", 2, 0, key, value));
}

int Redis_PUBLISH(RedisConnection_t conn, const char *channel, const char *message)
{
    return _RedisCommandReturn_extractInt(_RedisCommand_issue(conn, "PUBLISH", 2, 0, channel, message));
}

RedisArray_t *Redis_KEYS(RedisConnection_t conn, const char *pattern)
{
    REDIS_CMD__GENERIC(RedisObjectType_Array, conn, "KEYS", 1, 0, (RedisArray_t *)cmdRet.obj, NULL, false, pattern);
}

bool _Redis_EXPIRE_generic(RedisConnection_t conn, const char *cmd, const char *key, int num)
{
    char *numStr = _RedisObject_RESP__intAsStringWithLength(num, NULL);
    bool retVal = _RedisCommandReturn_extractBool(_RedisCommand_issue(conn, cmd, 2, 0, key, numStr));
    free(numStr);
    return retVal;
}

bool Redis_EXPIRE(RedisConnection_t conn, const char *key, int seconds)
{
    return _Redis_EXPIRE_generic(conn, "EXPIRE", key, seconds);
}

bool Redis_EXPIREAT(RedisConnection_t conn, const char *key, int timestamp)
{
    return _Redis_EXPIRE_generic(conn, "EXPIREAT", key, timestamp);
}

void Redis_SUBSCRIBE(RedisConnection_t conn, const char *channel)
{
    _RedisCommand_issue(conn, "SUBSCRIBE", 1, 0, channel);
}

void Redis_PSUBSCRIBE(RedisConnection_t conn, const char *pattern)
{
    _RedisCommand_issue(conn, "PSUBSCRIBE", 1, 0, pattern);
}

int Redis_LPUSHX(RedisConnection_t conn, const char *key, const char* value)
{
    return _RedisCommandReturn_extractInt(_RedisCommand_issue(conn, "LPUSHX", 2, 0, key, value));
}

char* Redis_LPOP(RedisConnection_t conn, const char* key)
{
    REDIS_CMD__GENERIC(RedisObjectType_BulkString, conn, "LPOP", 1, 0, (char *)cmdRet.obj, NULL, false, key);
}

char *Redis_LINDEX(RedisConnection_t conn, const char* key, int index)
{
    REDIS_CMD__GENERIC(RedisObjectType_BulkString, conn, "LINDEX", 2, 0x2, 
        (char *)cmdRet.obj, NULL, false, key, _RedisObject_RESP__intAsStringWithLength(index, NULL));
}

int Redis_LINSERT(RedisConnection_t conn, const char* key, RedisLINSERTPivot_t pivotPoint, const char* pivot, const char* value)
{
    assert(pivotPoint > PivotInvalid && pivotPoint < PivotLastSentinel);
    const char* pivotStr = pivotPoint == PivotBefore ? "BEFORE" : "AFTER";
    return _RedisCommandReturn_extractInt(_RedisCommand_issue(conn, "LINSERT", 4, 0, key, pivotStr, pivot, value));
}