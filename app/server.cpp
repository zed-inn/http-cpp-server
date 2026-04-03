#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <memory.h>
#include <errno.h>
#include <thread>

// Core Http Classes
#include "../core/ents/http-request.hpp"
#include "../core/ents/http-response.hpp"

using namespace std;

int getListeningSocket(int *sockfd)
{
    int status;
    struct addrinfo hints, *p = nullptr, *servinfo = nullptr;

    // Set hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // to connect to localhost

    // Fill the available addresses
    if ((status = getaddrinfo("localhost", "8080", &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "Err: %s\n", gai_strerror(status));
        return -1;
    }

    // Find a working address
    for (p = servinfo; p != nullptr; p = p->ai_next)
    {
        if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            fprintf(stderr, "Err: %s\n", strerror(errno));
            continue;
        }

        // Reuse addr
        int yes = 1;
        setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

        // Bind it to the port
        if ((status = bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1))
        {
            fprintf(stderr, "Err: %s\n", strerror(errno));
            continue;
        }

        if ((status = listen(*sockfd, 10)) == -1)
        {
            fprintf(stderr, "Err: %s\n", strerror(errno));
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == nullptr)
        return -1; // if no working sockfd found

    return 0;
}

void sendstr(int sockfd, strv s)
{
    size_t totalSent = 0, currSent;
    while (totalSent < s.size())
    {
        if ((currSent = send(sockfd, s.begin(), s.size(), 0)) == (long unsigned)-1)
        {
            fprintf(stderr, "Err: %s\n", strerror(errno));
            return;
        }
        totalSent += currSent;
    }
}

// Currently will only process one request and then disconnect
void processRequest(int sockfd)
{
    static const size_t START_SIZE = 64;
    static const unsigned short INCREMENT_MULTIPLIER = 2;

    str s;
    s.resize(START_SIZE);
    size_t bytesRecvd, bytesFilled = 0;

    HttpRequest req = HttpRequest();
    HttpResponse res = HttpResponse();
    ParseResult pres;

    while (!req.completed())
    {
        bytesRecvd = recv(sockfd, s.data() + bytesFilled, s.size() - bytesFilled, 0);
        if (bytesRecvd <= 0)
        {
            printf("Connection closed.\n");
            close(sockfd);
            return;
        }
        bytesFilled += bytesRecvd;

        // Increase size and propagate data point changes
        if (bytesFilled >= s.size())
        {
            auto prev = s.data();
            s.resize(s.size() * INCREMENT_MULTIPLIER);
            req.propagateMemoryChange(s.data(), prev);
        }

        pres = req.parse(s);
    }

    if (!pres.success)
    {
        res.setStatusCode(pres.error.code);
        res.setReponseLineMesssage(pres.error.reason);
        res.addBody("Some error occured.");
        sendstr(sockfd, res.createResponse());
        close(sockfd);
        return;
    }

    // Sending same message currently
    res.setStatusCode(HttpStatusCode::OK);
    res.addBody("Hello there!");
    sendstr(sockfd, res.createResponse());
    close(sockfd);
}

int main()
{
    int listener;
    if (getListeningSocket(&listener) == -1)
        return 1;

    // Wait and Accept connection for any new socket
    while (true)
    {
        int newfd;
        struct sockaddr_storage incoming_addr;
        socklen_t addr_size = sizeof(incoming_addr);

        newfd = accept(listener, (struct sockaddr *)&incoming_addr, &addr_size);

        // Process the new connection in thread
        thread t(processRequest, newfd);
        t.detach();
    }

    close(listener);

    return 0;
}