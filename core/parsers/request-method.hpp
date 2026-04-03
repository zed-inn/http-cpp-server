#include "../interface/parser.hpp"
#include "../utils/strs.hpp"

#ifndef _C_HTTP_REQUEST_METHOD_PARSER_
#define _C_HTTP_REQUEST_METHOD_PARSER_ 1

class HttpRequestMethod : Parser
{
public:
    typedef unsigned char Name;
    static constexpr size_t MAX_METHOD_LENGTH = 7;

    static constexpr Name INVALID = 0; // Invalid Method
    static constexpr Name GET = 1;     // GET Method
    static constexpr Name HEAD = 2;    // HEAD Method
    static constexpr Name POST = 3;    // POST Method
    static constexpr Name PUT = 4;     // PUT Method
    static constexpr Name DELETE = 5;  // DELETE Method
    static constexpr Name CONNECT = 6; // CONNECT Method
    static constexpr Name OPTIONS = 7; // OPTIONS Method
    static constexpr Name TRACE = 8;   // TRACE Method
    static constexpr Name PATCH = 9;   // PATCH Method

private:
    Name *_name;

public:
    HttpRequestMethod(Name *np) : _name(np) {}

    ParseResult parse(strv s) override
    {
        // If size is greater than allowed
        if (s.size() > MAX_METHOD_LENGTH)
            return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid HTTP Method");

        // Check if s completely matches method names
        if (s.compare("GET") == 0)
            *_name = GET;
        else if (s.compare("POST") == 0)
            *_name = POST;
        else if (s.compare("HEAD") == 0)
            *_name = HEAD;
        else if (s.compare("DELETE") == 0)
            *_name = DELETE;
        else if (s.compare("PUT") == 0)
            *_name = PUT;
        else if (s.compare("OPTIONS") == 0)
            *_name = OPTIONS;
        else if (s.compare("TRACE") == 0)
            *_name = TRACE;
        else if (s.compare("PATCH") == 0)
            *_name = PATCH;
        else if (s.compare("CONNECT") == 0)
            *_name = CONNECT;
        else // if nothing matched
        {
            *_name = INVALID;
            return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid HTTP Method");
        }

        return ParseResult();
    }
};

#endif