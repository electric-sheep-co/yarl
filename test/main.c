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

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s host port password\n\n", argv[0]);
        exit(-1);
    }

    const char* host = argv[1];
    const char* port = argv[2];
    const char* pass = argc > 3 ? argv[3] : NULL;

    RedisConnection_t rConn = RedisConnect(host, port);

    if (rConn < 1)
    {
        fprintf(stderr, "RedisConnect failed: %d\n", rConn);
        exit(rConn);
    }

    if (pass)
    {
        RedisReturnValue authRet = Redis_AUTH(rConn, pass);

        if (authRet != RedisSuccess)
        {
            fprintf(stderr, "AUTH failed: %d\n", authRet);
            exit(authRet);
        }
    }
    
	if (!Redis_SET(rConn, "FromTheSphere", "Hello, World!"))
	{
		fprintf(stderr, "SET failed\n");
		exit(-2);
	}

	char* getVal = Redis_GET(rConn, "FromTheSphere");
	printf("Hello, World? %s\n", getVal);
	free(getVal);
}
