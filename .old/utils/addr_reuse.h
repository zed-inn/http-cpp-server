#include <sys/socket.h>

void reuse_addr(int *sockfd)
{
    int yes = 1;
    setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
}

#pragma once