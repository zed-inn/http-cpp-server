#include <iostream>
#include <sys/socket.h>
#include <string.h>

#define BUFFER_SIZE 1024

int sendstr(int sockfd, const char *msg)
{
    int bytessent;
    int msglen = strlen(msg) + 1;
    bytessent = send(sockfd, msg, msglen, 0);
    if (bytessent == -1)
    {
        fprintf(stderr, "Err: %s\n", strerror(errno));
        return -1;
    }
    printf("Bytes sent: %d/%d\n", bytessent, msglen);
    return bytessent;
}

int recvstr(int sockfd)
{
    int bytesrecv;
    char buffer[BUFFER_SIZE];

    bytesrecv = recv(sockfd, buffer, sizeof(buffer), 0);
    if (bytesrecv == 0)
    {
        fprintf(stderr, "Err: %s\n", "Connection closed, no data recieved.");
        return -1;
    }
    printf("Data recieved [%d/%d]: %s\n", bytesrecv, sizeof(buffer), buffer);
    return bytesrecv;
}