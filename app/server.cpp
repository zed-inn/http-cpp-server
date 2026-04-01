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
    int totalSent = 0, currSent;
    while (totalSent < s.size())
    {
        if ((currSent = send(sockfd, s.begin(), s.size(), 0)) == -1)
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
    int bytesRecieved;
    char bytes[8192];

    HttpRequest req = HttpRequest();
    ParseResult res;

    while (!req.completed())
    {
        bytesRecieved = recv(sockfd, bytes, sizeof(bytes), 0);
        if (bytesRecieved <= 0)
        {
            printf("Connection closed.\n");
            close(sockfd);
            return;
        }
        res = req.parse(bytes);
    }

    if (!res.success)
    {
        if (res.error.code == HttpStatusCode::INTERNAL_SERVER_ERROR)
        {
            HttpResponse resp = HttpResponse(HttpStatusCode::INTERNAL_SERVER_ERROR, "Internal Server Error");
            resp.addBody("Server Issue");
            sendstr(sockfd, resp.createResponse());
            close(sockfd);
            return;
        }
        else
        {
            HttpResponse resp = HttpResponse(res.error.code, res.error.reason);
            resp.addBody("Server Issue");
            sendstr(sockfd, resp.createResponse());
            close(sockfd);
            return;
        }
    }

    if (req.method() == HttpRequestMethod::GET && req.path() == "/hello")
    {
        HttpResponse resp = HttpResponse(HttpStatusCode::OK);
        resp.addBody(R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>Hello to you too</title>
</head>
<body>
  <h1>Hi there!</h1>
  <p>Welcome to Localhost:8080</p>
</body>
</html>
)");
        resp.addHeader("Content-Type", "text/html");

        sendstr(sockfd, resp.createResponse());
        close(sockfd);
        return;
    }
    else
    {
        HttpResponse resp = HttpResponse(HttpStatusCode::NOT_FOUND, "Not Found");
        sendstr(sockfd, resp.createResponse());
        close(sockfd);
        return;
    }

    close(sockfd);
}

int main()
{
    int listener;
    if (getListeningSocket(&listener) == -1)
    {
        printf("Getting listening socket failed\n");
        return 1;
    }

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