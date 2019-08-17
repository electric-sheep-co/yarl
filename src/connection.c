#include "connection.h"
#include "constants.h"
#include "object.h"
#include "resp.h"
#include "log.h"

#include <assert.h>
#include <unistd.h>
#include <string.h>

RedisObject_t RedisConnection_sendCommand(RedisConnection_t conn, RedisObject_t cmdArrayObj)
{
    assert(conn > 0 && cmdArrayObj.type == RedisObjectType_Array && cmdArrayObj.obj);
    RedisObject_t rObj = {.type = RedisObjectType_InternalError};

    char *cmdRESP = RedisRESP_generate(cmdArrayObj);
    RedisLog("RedisCommand_issue(%d):\n-----\n%s\n-----\n", conn, cmdRESP);

    if (cmdRESP)
    {
        size_t expectedBytes = strlen(cmdRESP);
        ssize_t wroteBytes = write(conn, (const void *)cmdRESP, expectedBytes);

        if (expectedBytes == wroteBytes)
            rObj = RedisConnection_getNextObject(conn);
    }

    free(cmdRESP);
    return rObj;
}

typedef struct RedisObjectTypeMap_t
{
    RedisObjectType_t type;
    RedisObject_t (*create)(RedisConnection_t);
} RedisObjectTypeMap_t;

static RedisObjectTypeMap_t RedisConnection_getNextObject__parseMap[] = {
    {RedisObjectType_SimpleString, RedisObject_parseSimpleString},
    {RedisObjectType_BulkString, RedisObject_parseBulkString},
    {RedisObjectType_Integer, RedisObject_parseInteger},
    {RedisObjectType_Array, RedisObject_parseArray},
    {RedisObjectType_Error, RedisObject_parseError},
    {RedisObjectType_InternalError, NULL}
};

RedisObject_t RedisConnection_getNextObject(RedisConnection_t conn)
{
    RedisObject_t rObj = {.type = RedisObjectType_InternalError};
    RedisObjectType_t typeChar = RedisObjectType_NoType;

    ssize_t readBytes;
    do
    {
        readBytes = read(conn, &typeChar, sizeof(typeChar));
    } while (readBytes == sizeof(typeChar) && (typeChar == -1 || typeChar == '\r' || typeChar == '\n'));

    assert(readBytes == sizeof(typeChar));

    RedisObjectTypeMap_t *typeMapWalk = RedisConnection_getNextObject__parseMap;
    for (; typeMapWalk->type != RedisObjectType_InternalError && typeMapWalk->create != NULL; typeMapWalk++)
    {
        assert(typeMapWalk->create != NULL);
        if (typeMapWalk->type == typeChar)
        {
            rObj = typeMapWalk->create(conn);
            break;
        }
    }

    return rObj;
}
