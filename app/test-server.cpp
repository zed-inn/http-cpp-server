#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <memory.h>
#include <errno.h>
#include <poll.h>

// Core Http Classes
#include "../core/ents/http-request.hpp"
#include "../core/ents/http-response.hpp"

#define INFNTIME -1

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
    str s;
    s.resize(16384); // fixed size
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

class Descriptors
{
private:
    vector<pollfd> descs;

    bool isFdAtIndValid(size_t i)
    {
        if (i > descs.size() - 1)
            return false;

        return descs[i].fd >= 0;
    }

public:
    // To numerate over indices
    pollfd operator[](size_t i)
    {
        if (i > descs.size() - 1)
            return {fd : -1, events : 0, revents : 0};

        return descs[i];
    }

    // Get the array pointer
    pollfd *array()
    {
        return descs.data();
    }

    // Ignore the negative ones
    void add(int fd, short int events)
    {
        descs.push_back(pollfd{fd : fd, events : events, revents : 0});
    }

    // set a fd for removal
    // necessary to do `removeDead` afterwards
    void setForRemoval(size_t i)
    {
        if (i > descs.size() - 1)
            return; // oob

        // Change the current's fd to be negative
        descs[i].fd = ~descs[i].fd;
    }

    // Remove all the descriptor that are not going to be listened to
    void removeDead()
    {
        // Gather all the dead ones at end
        // Swap any invalid fd with the right most valid one
        size_t l = 0, r = descs.size() - 1;

        while (l < r)
        {
            // Find the leftmost invalid
            while (l < r && isFdAtIndValid(l))
                l++;

            // Find the rightmost valid
            while (r > l && !isFdAtIndValid(r))
                r--;

            // Swap and move forward
            auto temp = descs[l];
            descs[l] = descs[r], descs[r] = temp;
            l++, r--;
        }

        // Get the leftmost valid
        l = 0;
        while (isFdAtIndValid(l))
            l++;
        if (l > descs.size())
            l = descs.size() - 1;
        else if (!isFdAtIndValid(l))
            l--;

        // Erase all afterwards
        descs.erase(descs.begin() + l + 1, descs.end());
    }

    // Get the size of the current descriptors
    nfds_t size()
    {
        return descs.size();
    }
};

int main()
{
    int listener;
    if (getListeningSocket(&listener) == -1)
        return 1;

    // Create Descriptors
    Descriptors ds = Descriptors();

    // Push the listener in the descriptors
    ds.add(listener, POLLIN); // Only listening to read

    int newFd;
    struct sockaddr_storage incomingAddr;
    socklen_t addrSize = sizeof(incomingAddr);

    while (true)
    {
        int numEvents = poll(ds.array(), ds.size(), INFNTIME);

        // If no event happened
        if (numEvents == 0)
            continue;

        // Events happened

        pollfd tf;
        for (size_t i = 0; i < ds.size(); i++)
        {
            tf = ds[i];

            // If no revents
            if (!tf.revents)
                continue;

            // No events related to pollin or pollout happened
            if (!(tf.revents & POLLIN || tf.events & POLLOUT))
                continue;

            // POLLIN happened on listener
            if (tf.fd == listener)
            {
                // Accept Connections
                newFd = accept(listener, (struct sockaddr *)&incomingAddr, &addrSize);

                if (newFd == -1)
                {
                    perror("Err: newFd cannot be created\n");
                    continue;
                }

                // Add in fds
                ds.add(newFd, POLLIN | POLLOUT); // Worry about reading AND writing to it!

                continue;
            }

            // Process Request -> Send Response -> Close socket
            processRequest(tf.fd); // WARN: send can block if buffer is full

            // Remove from the fds array
            ds.setForRemoval(i);
        }

        ds.removeDead(); // Remove all those fds put for removal
    }

    close(listener);

    return 0;
}