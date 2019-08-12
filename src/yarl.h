#ifndef __YARL_H__
#define __YARL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define YARL_VERSION "0.2.0"
#define YARL_VERSION_NUMERIC 020

#include "commands.h"

extern void RedisArray_dealloc(RedisArray_t *);

#ifdef __cplusplus
}
#endif

#endif // __YARL_H__
