#ifndef __YARL_LOG__H__
#define __YARL_LOG__H__

#if DEBUG
#include <stdio.h>
#define RedisLog(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); fflush(stderr); } while(0)
#else
#define RedisLog(fmt, ...)
#endif

#endif // __YARL_LOG__H__