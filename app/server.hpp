#include <vector>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <memory.h>
#include <errno.h>
#include <poll.h>
#include <string>
#include <unistd.h>

// Core Http Classes
#include "../core/ents/http-request.hpp"
#include "../core/ents/http-response.hpp"

using namespace std;

#ifndef _C_HTTP_OVER_TCP_SERVER_
#define _C_HTTP_OVER_TCP_SERVER_ 1

/*
Depends on Pollfd struct. Creates and builds a dynamic array.
When polling the usual thing is to get lost on how to add and
remove fds that have been closed, this solves it

Use it like a dynamic array, where you won't have to worry about
pollfd struct and vector maths, just use the appropriate fn when
needed
*/
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
        descs.erase(
            std::remove_if(descs.begin(),
                           descs.end(),
                           [](const pollfd &p)
                           { return p.fd < 0; }),
            descs.end());
    }

    // Get the size of the current descriptors
    nfds_t size()
    {
        return descs.size();
    }
};

class HttpOverTcpServer
{
public:
    typedef void (*Router)(HttpRequest *req, HttpResponse *res);

    // If errcode is 0, then no error
    struct ServerError
    {
        int code; // would be copied from the global 'errno'
        string reason;

    } typedef Error;

    struct ServerInformation
    {
        string host; // locatlhost
        string port; // 8080

        // Defaults are localhost and port
        ServerInformation() : host("localhost"), port("8080") {}
        ServerInformation(string host, string port) : host(host), port(port) {}

    } typedef Servinfo;

private:
    typedef unordered_map<string, Router> RouteMap;
    typedef int Socket; // socket as is given by socket()

    static constexpr unsigned short BACKLOG = 10; // 10 backlog is fine

    static constexpr Socket INVALID_SOCKET = -1;

    // Get listening socket
    static inline Socket getListener(Servinfo *sv, Error *e)
    {
        int status;                            // error collection
        int fd;                                // listener socket file descriptor
        struct addrinfo hints, *p, *saddrinfo; // addrinfo structs
        saddrinfo = p = nullptr;               // dangling pointers

        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family = AF_UNSPEC; // could be v4 or v6
        hints.ai_flags = AI_PASSIVE;

        // Fill up the structs
        if ((status = getaddrinfo(sv->host.data(), sv->port.data(), &hints, &saddrinfo)) != 0)
        {
            e->code = errno;
            e->reason = gai_strerror(status);
            return INVALID_SOCKET;
        }

        // Loop over the structs and find a working socket
        for (p = saddrinfo; p != nullptr; p = p->ai_next)
        {
            // Get the socket
            fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (fd == -1)
            {
                e->code = errno;
                e->reason = strerror(errno);
                continue;
            }

            // SO_REUSEADDR for binding
            int yes = 1;
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

            // Bind the socket to the port
            status = bind(fd, p->ai_addr, p->ai_addrlen);
            if (status == -1)
            {
                e->code = errno;
                e->reason = strerror(errno);
                continue;
            }

            // Listen on the socket
            status = listen(fd, BACKLOG);
            if (status == -1)
            {
                e->code = errno;
                e->reason = strerror(errno);
                continue;
            }

            break;
        }

        freeaddrinfo(saddrinfo);

        if (p == nullptr)
        {
            // No working address found
            e->code = -1;
            e->reason = "No working address found.";
            return INVALID_SOCKET;
        }

        return fd;
    }

    static inline void sendStr(Socket fd, strv s)
    {
        size_t totalSent = 0, currSent;
        while (totalSent < s.size())
        {
            if ((currSent = send(fd, s.begin(), s.size(), 0)) == (long unsigned)-1)
            {
                fprintf(stderr, "Err: %s\n", strerror(errno));
                return;
            }
            totalSent += currSent;
        }
    }

    Socket listener;
    Servinfo svri;
    RouteMap rm;

    void processRequest(Socket fd)
    {
        if (fd < 0)
            return;

        string s;
        s.resize(HttpRequest::MAX_REQUEST_SIZE);
        int recvd = 0;

        HttpRequest req = HttpRequest();
        HttpResponse res = HttpResponse();
        ParseResult pr;

        while (!req.completed())
        {
            int n = recv(fd, s.data() + recvd, s.size() - recvd, 0);
            if (n <= 0)
            {
                close(fd);
                return;
            }
            recvd += n;
            pr = req.parse(s);
        }

        if (!pr.success)
        {
            res.setStatusCode(pr.error.code);
            if (pr.error.code == HttpStatusCode::INTERNAL_SERVER_ERROR)
                res.setReponseLineMesssage("Internval Server Error");
            else
                res.setReponseLineMesssage(pr.error.reason);
            sendStr(fd, res.createResponse());
            close(fd);
            return;
        }

        // Search for paths
        RouteMap::iterator it;
        auto path = req.path();
        if (auto p = get_if<string>(&path))
            it = rm.find(*p);
        else if (auto p = get_if<string_view>(&path))
            it = rm.find(str(*p));

        // No route like that
        if (it == rm.end())
        {
            res.setStatusCode(HttpStatusCode::NOT_FOUND);
            res.setReponseLineMesssage("Not Found");
            res.addBody("No path found like that");
        }
        // Run the function to update response
        else
            it->second(&req, &res);

        sendStr(fd, res.createResponse());
        close(fd);
    }

public:
    HttpOverTcpServer() : svri(Servinfo()) {}
    HttpOverTcpServer(str host, str port) : svri(Servinfo(host, port)) {}

    Error launch()
    {
        Error e;
        listener = getListener(&svri, &e);
        if (listener == INVALID_SOCKET)
            return e;

        // list of descriptors
        Descriptors ds;

        // Push the main listener fd
        ds.add(listener, POLLIN); // only for listening

        int numEvents;
        int newFd;                                        // Client's fd
        struct sockaddr_storage incomingAddr;             // Incoming addr
        socklen_t incomingAddrLen = sizeof(incomingAddr); // Size of incoming addr
        while (true)
        {
            numEvents = poll(ds.array(), ds.size(), -1);

            // if no events
            if (numEvents == 0)
                continue;

            for (size_t i = 0; i < ds.size(); i++)
            {
                // Skip any with no revents
                if (!ds[i].revents)
                    continue;

                // Skip any with not POLLIN
                if (!(ds[i].revents & POLLIN))
                    continue;

                // Listener with POLLIN
                if (ds[i].fd == listener)
                {
                    // Accept new connections
                    newFd = accept(listener, (struct sockaddr *)&incomingAddr, &incomingAddrLen);

                    // Return error if not accepted
                    if (newFd == -1)
                    {
                        e.code = errno;
                        e.reason = strerror(errno);
                        continue;
                    }

                    ds.add(newFd, POLLIN); // Only checking if ready to read
                    continue;
                }

                // Client sockets

                // Handles only one request CURRENTLY
                // Process request ->  Create response -> Send Response -> Close socket
                processRequest(ds[i].fd);

                ds.setForRemoval(i);
            }

            ds.removeDead();
        }

        close(listener);
    }

    // Only absolute paths are supported currently
    void addRoute(str path, Router router)
    {
        rm[path] = router;
    }

} typedef Server;

#endif
