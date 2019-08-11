#include "commands.h"
#include "connection.h"
#include "object.h"

#include <string.h>

RedisReturnValue Redis_AUTH(RedisConnection_t conn, const char *password)
{
    if (conn > 0)
    {
        size_t passwordLength = strlen(password);
        if (passwordLength > 0)
        {
            RedisArray_t *command = RedisArray_init(2);
            RedisObject_t cmdObj = {
                .type = RedisObjectType_Array,
                .obj = (void *)command,
                .objIsOwned = true};

            RedisObject_t authObj = {
                .type = RedisObjectType_BulkString,
                .obj = (void *)"AUTH",
                .objIsOwned = false};

            RedisObject_t passwordObj = {
                .type = RedisObjectType_BulkString,
                .obj = (void *)password,
                .objIsOwned = false};

            command->objects[0] = authObj;
            command->objects[1] = passwordObj;

            RedisObject_t cmdRet = RedisConnection_sendCommand(conn, cmdObj);
            RedisObject_dealloc(cmdObj);

            return cmdRet.type == RedisObjectType_SimpleString && cmdRet.obj &&
                           strcmp((const char *)cmdRet.obj, "OK") == 0
                       ? RedisSuccess
                       : RedisAuthFailure;
        }

        return RedisSuccess;
    }

    return RedisNotConnectedFailure;
}

char *Redis_GET(RedisConnection_t conn, const char *key)
{
    RedisArray_t *command = RedisArray_init(2);

    RedisObject_t cmdObj = {
        .type = RedisObjectType_Array,
        .obj = (void *)command,
        .objIsOwned = true};

    RedisObject_t getObj = {
        .type = RedisObjectType_BulkString,
        .obj = (void *)"GET",
        .objIsOwned = false};

    RedisObject_t keyObj = {
        .type = RedisObjectType_BulkString,
        .obj = (void *)key,
        .objIsOwned = false};

    command->objects[0] = getObj;
    command->objects[1] = keyObj;

    RedisObject_t cmdRet = RedisConnection_sendCommand(conn, cmdObj);
    RedisObject_dealloc(cmdObj);

    if (cmdRet.type == RedisObjectType_InternalError || !cmdRet.obj)
        return NULL;

    return (char *)cmdRet.obj;
}

bool Redis_SET(RedisConnection_t conn, const char *key, const char *value)
{
    RedisArray_t *command = RedisArray_init(3);
    RedisObject_t cmdObj = {
        .type = RedisObjectType_Array,
        .obj = (void *)command,
        .objIsOwned = true};

    RedisObject_t setObj = {
        .type = RedisObjectType_BulkString,
        .obj = (void *)"SET",
        .objIsOwned = false};

    RedisObject_t keyObj = {
        .type = RedisObjectType_BulkString,
        .obj = (void *)key,
        .objIsOwned = false};

    RedisObject_t valObj = {
        .type = RedisObjectType_BulkString,
        .obj = (void *)value,
        .objIsOwned = false};

    command->objects[0] = setObj;
    command->objects[1] = keyObj;
    command->objects[2] = valObj;

    RedisObject_t cmdRet = RedisConnection_sendCommand(conn, cmdObj);
    RedisObject_dealloc(cmdObj);

    if (cmdRet.type == RedisObjectType_InternalError || !cmdRet.obj)
        return NULL;

    return strncmp((const char *)cmdRet.obj, "OK", strlen("OK")) == 0;
}