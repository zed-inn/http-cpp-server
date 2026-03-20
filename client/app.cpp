#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <memory.h>
#include <errno.h>
#include "../constants/env.h"
#include "../utils/addr_reuse.h"
#include "../utils/send_recv.h"

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

    // get working p attach to socket file descriptor
    {
        for (p = servinfo; p != nullptr; p = p->ai_next)
        {
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
                printf("%s\n", strerror(errno));
            else
                break;
        }
        if (p == nullptr)
        {
            printf("Err: No valid address to connect.");
            exit(1);
        }
    }

    // connect the socket fd to address
    {
        status = connect(sockfd, p->ai_addr, p->ai_addrlen);
        if (status == -1)
        {
            fprintf(stderr, "Err: %s\n", strerror(errno));
            exit(1);
        }
        printf("Port connected.\n");
    }

    // Send and recv
    {
        recvstr(sockfd);
        recvstr(sockfd);
        sendstr(sockfd, "sahi hu");
    }

    close(sockfd);
    freeaddrinfo(servinfo);

    return 0;
}