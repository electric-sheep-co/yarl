#include "constants.h"
#include "connection.h"
#include "object.h"
#include "log.h"

#include <unistd.h>
#include <string.h>
#include <assert.h>

static const size_t READ_BUF_BLOCK_SIZE = 32;

RedisObject_t RedisObject_parseSimpleString(RedisConnection_t conn)
{
    RedisObject_t rObj = {
        .type = RedisObjectType_SimpleString,
        .obj = NULL,
        .objIsOwned = true };

    char *readBuf = NULL;
    size_t readOffset = 0;
    do
    {
        if (!(readOffset % READ_BUF_BLOCK_SIZE))
        {
            size_t newSize = READ_BUF_BLOCK_SIZE * ((readOffset / READ_BUF_BLOCK_SIZE) + 1);
            readBuf = realloc(readBuf, newSize);
        }

        assert(read(conn, readBuf + readOffset, sizeof(char)) == sizeof(char));

        // is this needed? consumes the newline, but doesn't getNextObject do that already?
        if (readBuf[readOffset] != '\n')
            ++readOffset;
    } while (readOffset == 0 || readBuf[readOffset - 1] != '\r');

    readBuf[readOffset - 1] = '\0';
    rObj.obj = realloc(readBuf, readOffset);
    RedisLog("RedisObject_parseSimpleString(%d): '%s' (%lu bytes)\n", conn, readBuf, readOffset);
    return rObj;
}

RedisObject_t RedisObject_parseBulkString(RedisConnection_t conn)
{
    RedisObject_t rObj = {
        .type = RedisObjectType_BulkString,
        .obj = NULL,
        .objIsOwned = true };

    RedisObject_t lenObj = RedisObject_parseInteger(conn);
    assert(lenObj.obj);

    int len = *(int *)lenObj.obj;
    RedisObject_dealloc(lenObj);

    // "Null Bulk String" -- https://redis.io/topics/protocol#resp-bulk-strings
    if (len != -1)
    {
        RedisObject_t strObj = RedisObject_parseSimpleString(conn);
        rObj.obj = strObj.obj;
    }

    return rObj;
}

RedisObject_t RedisObject_parseInteger(RedisConnection_t conn)
{
    RedisObject_t rObj = RedisObject_parseSimpleString(conn);
    int convNum = atoi((const char *)rObj.obj);
    RedisObject_dealloc(rObj);

    rObj.type = RedisObjectType_Integer;
    rObj.obj = (void *)malloc(sizeof(int));
    rObj.objIsOwned = true;
    memcpy(rObj.obj, &convNum, sizeof(int));

    return rObj;
}

RedisObject_t RedisObject_parseArray(RedisConnection_t conn)
{
    RedisObject_t rObj = {
        .type = RedisObjectType_Array,
        .obj = NULL,
        .objIsOwned = true };

    RedisObject_t lenObj = RedisObject_parseInteger(conn);
    assert(lenObj.obj);

    int len = *(int *)lenObj.obj;
    RedisObject_dealloc(lenObj);
    assert(len >= 0);

    RedisArray_t *retArr = RedisArray_init(len);

    for (int i = 0; i < len; i++)
        retArr->objects[i] = RedisConnection_getNextObject(conn);

    rObj.obj = retArr;
    return rObj;
}

RedisObject_t RedisObject_parseError(RedisConnection_t conn)
{
    RedisObject_t rObj = RedisObject_parseSimpleString(conn);
    rObj.type = RedisObjectType_Error;
    return rObj;
}

RedisArray_t *RedisArray_init(size_t count)
{
    assert(count >= 0);
    RedisArray_t *ret = (RedisArray_t *)malloc(sizeof(RedisArray_t));
    ret->count = count;
    ret->objects = (RedisObject_t *)malloc(sizeof(RedisObject_t) * (size_t)ret->count);
    return ret;
}

void RedisArray_dealloc(RedisArray_t *arr)
{
    for (int i = 0; i < arr->count; i++)
        RedisObject_dealloc(arr->objects[i]);
    free(arr->objects);
    free(arr);
}

void RedisObject_dealloc(RedisObject_t obj)
{
    if (!obj.obj)
        return;

    if (obj.type == RedisObjectType_Array)
        RedisArray_dealloc((RedisArray_t *)obj.obj);
    else if (obj.objIsOwned)
        free(obj.obj);
}
