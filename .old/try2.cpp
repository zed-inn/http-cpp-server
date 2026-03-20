#include <sys/socket.h>
#include <netdb.h>
#include <memory.h>
#include <iostream>
#include <arpa/inet.h>
#include <errno.h>

using namespace std;

void reset_bind(int *sockfd)
{
    int yes = 1;
    setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
}

int main(int argc, char *argv[])
{
    struct sockaddr_storage thier_addr;
    int status, sockfd, new_fd;
    struct addrinfo hints, *p = nullptr, *servinfo = nullptr;
    const char *port = "4000";

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "Err: %s\n", gai_strerror(status));
        exit(1);
    }

    for (p = servinfo; p != nullptr; p = p->ai_next)
    {
        void *addr = nullptr;
        struct sockaddr_in *ip4;
        struct sockaddr_in6 *ip6;
        char ipstr[INET6_ADDRSTRLEN];
        const char *ipver = nullptr;

        if (p->ai_family == AF_INET)
        {
            ip4 = (sockaddr_in *)p->ai_addr;
            addr = &(ip4->sin_addr);
            ipver = "IPv4";
        }
        else if (p->ai_family == AF_INET6)
        {
            ip6 = (sockaddr_in6 *)p->ai_addr;
            addr = &(ip6->sin6_addr);
            ipver = "IPv6";
        }

        if (addr != nullptr)
        {
            inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
            cout << "Using " << ipver << " address: " << ipstr << endl;
            break;
        }
    }

    if (p == nullptr)
    {
        cout << "Err: No valid address to use." << endl;
        exit(1);
    }

    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1)
    {
        cout << "Err: Socket file descriptor could not be set." << endl;
        exit(1);
    }

    reset_bind(&sockfd);

    status = bind(sockfd, p->ai_addr, p->ai_addrlen);
    if (status == -1)
    {
        cout << "Err: Socket could not be binded." << endl;
        exit(1);
    }

    cout << "Binded on port: " << port << endl;

    // status = connect(sockfd, p->ai_addr, p->ai_addrlen);
    // if (status == -1)
    // {
    //     cout << "Err: Could not be connected." << endl;
    //     exit(1);
    // }

    // cout << "Connected." << endl;

    status = listen(sockfd, 5);
    if (status == -1)
    {
        cout << "Err: Not listening." << endl;
        exit(1);
    }

    cout << "Listening." << endl;

    // socklen_t addr_size = sizeof thier_addr;
    // new_fd = accept(sockfd, (struct sockaddr *)&thier_addr, &addr_size);
    // if (new_fd == -1)
    // {
    //     cout << "Err: Not accepted." << endl;
    //     exit(1);
    // }

    // cout << "Accepted." << endl;

    freeaddrinfo(servinfo);

    return 0;
}