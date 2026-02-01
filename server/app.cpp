#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <memory.h>
#include <errno.h>
#include "../constants/env.h"
#include "../utils/addr_reuse.h"

using namespace std;

int main()
{
    int status, sockfd;
    struct addrinfo hints, *p = nullptr, *servinfo = nullptr;

    // fill hints
    {
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
    }

    // get addr info
    {
        if ((status = getaddrinfo(APP_HOST, APP_PORT, &hints, &servinfo)) != 0)
        {
            fprintf(stderr, "Err: %s\n", gai_strerror(status));
            exit(1);
        }
    }

    // get working p attached to socket file descriptor
    {
        for (p = servinfo; p != nullptr; p = p->ai_next)
        {
            sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (sockfd != -1)
                break;
        }
        if (p == nullptr)
        {
            printf("Err: No valid address to connect.");
            exit(1);
        }
    }

    // reuse the address if already in use error, but not connected as we know
    reuse_addr(&sockfd);

    // bind the socket fd to address
    {
        status = bind(sockfd, p->ai_addr, p->ai_addrlen);
        int err_bind = errno;
        if (status == -1)
        {
            fprintf(stderr, "Err: %s\n", strerror(err_bind));
            exit(1);
        }
    }

    // listen on the socket after bind
    {
        status = listen(sockfd, APP_BACKLOG);
        int err_listen = errno;
        if (status == -1)
        {
            fprintf(stderr, "Err: %s\n", strerror(err_listen));
            exit(1);
        }
    }

    

    freeaddrinfo(servinfo); // free at last the struct

    return 0;
}