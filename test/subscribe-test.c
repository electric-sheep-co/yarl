#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include <yarl.h>

static volatile sig_atomic_t running = true;

// adapted from http://beej.us/guide/bgnet/html/multi/clientserver.html#simpleclient

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

RedisConnection_t RedisConnect(const char *host, const char *port)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return -2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    freeaddrinfo(servinfo); // all done with this structure
    return (RedisConnection_t)sockfd;
}

#define TEST_KEY_NAME "YarlSaysHi"

void printRedisObject(RedisObject_t rObj)
{
    printf("RedisObject_t<.obj=%p> of type '%s':\n", rObj.obj, 
        RedisObject_TypeToString(rObj.type));

    if (!rObj.obj) 
        return;
        
    switch (rObj.type)
    {
        case RedisObjectType_SimpleString:
        case RedisObjectType_BulkString:
        case RedisObjectType_Error:
            printf("\t%s\n", (char*)rObj.obj);
            break;
        case RedisObjectType_Integer:
            printf("\t%d\n", *(int*)rObj.obj);
            break;
        case RedisObjectType_Array:
        {
            RedisArray_t* rArr = (RedisArray_t*)rObj.obj;
            for (int i = 0; i < rArr->count; i++) {
                printRedisObject(rArr->objects[i]);
            }
            break;
        }
        default: break;
    }
}

void* subThreadFunc(void* arg)
{
    assert(arg);
    RedisConnection_t rConn = (RedisConnection_t)arg;
    
    while (running)
    {
        RedisObject_t nextObj = RedisConnection_getNextObject(rConn);
        printRedisObject(nextObj);
        RedisObject_dealloc(nextObj);
    }

    return NULL;
}

void sighand(int signal)
{
    fprintf(stderr, "SIGNALED %d\n", signal);
    running = false;
}

int main(int argc, char **argv)
{
    signal(SIGINT, sighand);
    signal(SIGTERM, sighand);

    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s host port password\n\n", argv[0]);
        exit(-1);
    }

    const char *host = argv[1];
    const char *port = argv[2];
    const char *pass = argc > 3 ? argv[3] : NULL;

    RedisConnection_t rConn = RedisConnect(host, port);

    if (rConn < 1)
    {
        fprintf(stderr, "RedisConnect failed: %d\n", rConn);
        exit(rConn);
    }

    if (pass && !Redis_AUTH(rConn, pass))
    {
        fprintf(stderr, "AUTH failed\n");
        exit(-1);
    }

    pthread_t th;
    int pc = pthread_create(&th, NULL, subThreadFunc, (void*)(long)rConn);
    if (pc)
    {
        fprintf(stderr, "pthread_create: %d\n", pc);
        exit(-1);
    }

    Redis_PSUBSCRIBE(rConn, "*");
    pthread_join(th, NULL);
    printf("Done!\n");
}
