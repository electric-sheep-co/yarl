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

#include <yarl.h>

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

int main(int argc, char **argv)
{
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

    bool exists = Redis_EXISTS(rConn, TEST_KEY_NAME);
    printf("Exists already? %d\n", exists);

    if (exists && !Redis_DEL(rConn, TEST_KEY_NAME))
    {
        fprintf(stderr, "DEL failed\n");
        exit(-2);
    }

    if (!Redis_SET(rConn, TEST_KEY_NAME, "Hello, World!"))
    {
        fprintf(stderr, "SET failed\n");
        exit(-2);
    }

    char *getVal = Redis_GET(rConn, TEST_KEY_NAME);
    printf("Hello, World? %s\n", getVal);
    free(getVal);

    int appended = Redis_APPEND(rConn, TEST_KEY_NAME, " It's a beautiful day!");
    getVal = Redis_GET(rConn, TEST_KEY_NAME);
    printf("Appended %d bytes, now have '%s'\n", appended, getVal);
    free(getVal);

    assert(Redis_DEL(rConn, TEST_KEY_NAME));

    printf("Publish result: %d\n", Redis_PUBLISH(rConn, TEST_KEY_NAME, "Pub/sub is the bee's knees!"));

    RedisArray_t *allKeys = Redis_KEYS(rConn, "*");

    if (allKeys)
    {
        printf("Found %lu keys\n", allKeys->count);
        RedisArray_dealloc(allKeys);
    }
    else
    {
        fprintf(stderr, "KEYS failed\n");
        exit(-1);
    }

    assert(Redis_SET(rConn, TEST_KEY_NAME "ButWillExpireInSixty", "60...") &&
        Redis_EXPIRE(rConn, TEST_KEY_NAME "ButWillExpireInSixty", 60));

    char* lName = TEST_KEY_NAME "Listicle";
    assert(Redis_LPUSH(rConn, lName, 3, "foo", "bar", "baz") == 3);
    assert(Redis_LPUSHX(rConn, lName, "LPUSHX") == 4);

    char* popped = Redis_LPOP(rConn, lName);
    assert(!strncmp(popped, "LPUSHX", strlen("LPUSHX")));
    free(popped);

    assert(!Redis_LPUSHX(rConn, TEST_KEY_NAME "DNE", "LPUSHX"));

    char* lidx = Redis_LINDEX(rConn, lName, 2);
    assert(!strncmp(lidx, "foo", strlen("foo")));

    assert(Redis_LINSERT(rConn, lName, PivotBefore, "bar", "before_bar") == 4 &&
        Redis_LINSERT(rConn, lName, PivotAfter, "bar", "after_bar") == 5);

    Redis_DEL(rConn, lName);
    printf("List tests successful.\n");
}
