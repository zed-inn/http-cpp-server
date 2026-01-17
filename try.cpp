#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <memory.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#define PRINT(x) cout << x << endl

using namespace std;

int main(int argc, char *argv[])
{
    int status;
    struct addrinfo hints, *p = nullptr;
    struct addrinfo *servinfo = nullptr;
    const char *host, *port;

    if (argc < 2)
    {
        PRINT("usage: host");
        exit(1);
    }

    // Assign host and port
    {
        host = argv[1];
        port = argc >= 3 ? argv[2] : nullptr;
        if (port == nullptr)
            port = "http";
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host, port, &hints, &servinfo)) != 0)
        fprintf(stderr, "gai_error: %s\n", gai_strerror(status));

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        void *addr;
        const char *ipver;
        struct sockaddr_in *ipv4;
        struct sockaddr_in6 *ipv6;
        char ipstr[INET6_ADDRSTRLEN];

        if (p->ai_family == AF_INET)
        {
            ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        else
        {
            ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        PRINT(ipver << ": " << ipstr);
    }

    freeaddrinfo(servinfo);

    return 0;
}