#include "resp.h"
#include "constants.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#define SS_LEN_STR_BUF_LEN 32
#define assert_RESP(obj, expectType) assert(obj.type == expectType && obj.obj != NULL)

char *_RedisObject_RESP__allocWithLen(RedisObject_t obj, size_t emitLen)
{
    // the null terminator (just in case the caller forgot, which does tend to happen)
    emitLen += 1;
    char *emitStr = (char *)malloc(emitLen);
    bzero(emitStr, emitLen);
    emitStr[0] = (char)obj.type;
    return emitStr;
}

char *_RedisObject_RESP__intAsStringWithLength(size_t num, size_t *outLength)
{
    char ssLenStr[SS_LEN_STR_BUF_LEN];
    bzero(ssLenStr, SS_LEN_STR_BUF_LEN);

    int ssLenStrLen = snprintf(ssLenStr, SS_LEN_STR_BUF_LEN, "%ld", num);
    assert(ssLenStrLen >= 0 && ssLenStrLen < SS_LEN_STR_BUF_LEN);

    size_t _sslslt = (size_t)ssLenStrLen;
    if (outLength)
        *outLength = _sslslt;

    // account for trailing null
    char *ret = (char *)malloc(++_sslslt);
    memcpy(ret, ssLenStr, _sslslt);
    return ret;
}

char *RedisObject_RESP_simpleString(RedisObject_t obj)
{
    assert_RESP(obj, RedisObjectType_SimpleString);
    // Simple strings cannot contain CRLF, as they must terminate with CRLF
    // https://redis.io/topics/protocol#resp-simple-strings
    assert(obj.obj && strstr((char *)obj.obj, CRLF) == NULL);
    char *ss = (char *)obj.obj;

    size_t emitStrLen = strlen(ss) + sizeof(obj.type) + strlen(CRLF);
    char *emitStr = _RedisObject_RESP__allocWithLen(obj, emitStrLen);
    memcpy(emitStr + 1, ss, strlen(ss));
    memcpy(emitStr + (emitStrLen - strlen(CRLF)), CRLF, strlen(CRLF));

    return emitStr;
}

#define SS_LEN_STR_BUF_LEN 32
char *RedisObject_RESP_bulkString(RedisObject_t obj)
{
    assert_RESP(obj, RedisObjectType_BulkString);
    char *ss = (char *)obj.obj;
    size_t ssLen = strlen(ss);

    size_t ssLenStrLen;
    char *ssLenStr = _RedisObject_RESP__intAsStringWithLength(ssLen, &ssLenStrLen);

    size_t crlfLen = strlen(CRLF);
    size_t emitStrLen = ssLen + sizeof(obj.type) + ssLenStrLen + (crlfLen * 2);

    char *emitStr = _RedisObject_RESP__allocWithLen(obj, emitStrLen);
    char *copyPtr = emitStr + 1;

    memcpy(copyPtr, ssLenStr, ssLenStrLen);
    copyPtr += ssLenStrLen;
    free(ssLenStr);
    memcpy(copyPtr, CRLF, crlfLen);
    copyPtr += crlfLen;
    memcpy(copyPtr, ss, ssLen);
    copyPtr += ssLen;
    memcpy(copyPtr, CRLF, crlfLen);

    return emitStr;
}

typedef struct
{
    char *RESP;
    size_t length;
} _RedisObject_RESP_array__coll_t;

char *RedisObject_RESP_array(RedisObject_t obj)
{
    assert_RESP(obj, RedisObjectType_Array);
    RedisArray_t *rArr = (RedisArray_t *)obj.obj;

    size_t constRESPsLen = sizeof(_RedisObject_RESP_array__coll_t) * rArr->count;
    _RedisObject_RESP_array__coll_t *constRESPs = (_RedisObject_RESP_array__coll_t *)malloc(constRESPsLen);
    bzero(constRESPs, constRESPsLen);

    size_t constRESPsTotalLen = 0;
    for (int i = 0; i < rArr->count; i++)
    {
        char *tRESP = RedisRESP_generate(rArr->objects[i]);
        _RedisObject_RESP_array__coll_t tCol = {
            .RESP = tRESP,
            .length = strlen(tRESP)};
        constRESPs[i] = tCol;
        constRESPsTotalLen += tCol.length;
    }

    size_t arrLenStrLen;
    char *arrLenStr = _RedisObject_RESP__intAsStringWithLength(rArr->count, &arrLenStrLen);

    size_t emitStrLen = sizeof(obj.type) + constRESPsTotalLen + arrLenStrLen + strlen(CRLF) + 1;
    char *emitStr = _RedisObject_RESP__allocWithLen(obj, emitStrLen);
    char *copyPtr = emitStr + 1;

    memcpy(copyPtr, arrLenStr, arrLenStrLen);
    copyPtr += arrLenStrLen;
    free(arrLenStr);

    memcpy(copyPtr, CRLF, strlen(CRLF));
    copyPtr += strlen(CRLF);

    for (int i = 0; i < rArr->count; i++)
    {
        memcpy(copyPtr, constRESPs[i].RESP, constRESPs[i].length);
        copyPtr += constRESPs[i].length;
    }

    return emitStr;
}

typedef struct RedisObjectRESPTypeMap_t
{
    RedisObjectType_t type;
    char *(*RESP)(RedisObject_t);
} RedisObjectRESPTypeMap_t;

static RedisObjectRESPTypeMap_t RedisObject_RESP_map[] = {
    {RedisObjectType_SimpleString, RedisObject_RESP_simpleString},
    {RedisObjectType_BulkString, RedisObject_RESP_bulkString},
    {RedisObjectType_Integer, RedisObject_RESP_simpleString},
    {RedisObjectType_Array, RedisObject_RESP_array},
    {RedisObjectType_Error, RedisObject_RESP_simpleString},
    {RedisObjectType_InternalError, NULL}};

char *RedisRESP_generate(RedisObject_t obj)
{
    RedisObjectRESPTypeMap_t *typeMapWalk = RedisObject_RESP_map;

    for (; typeMapWalk->type != RedisObjectType_InternalError && typeMapWalk->RESP != NULL; typeMapWalk++)
    {
        assert(typeMapWalk->RESP != NULL);
        if (typeMapWalk->type == obj.type)
            return typeMapWalk->RESP(obj);
    }

    return NULL;
}
