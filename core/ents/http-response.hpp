#include "../utils/strs.hpp"
#include "../utils/http-date.hpp"

#ifndef _C_HTTP_RESPONSE_
#define _C_HTTP_RESPONSE_ 1

// Minimal headers currently
// Date and Content-Length
// Content Type would be text/plain

// protocol getting used will be HTTP/1.1

// Would not use Transfer-Encoding

class HttpResponse
{
private:
    typedef unsigned short ResponseStatusCode;

    static constexpr size_t FRONT_RESERVED_PAYLOAD_SPACE = 256;
    static constexpr size_t FRONT_RESERVED_RESPONSE_SPACE = 128;

    static const inline str PROTOCOL_LITERAL = "HTTP/1.1";

    ResponseStatusCode statusCode; // response status code
    str message;                   // small message
    size_t contentLength = 0;      // as in payload size
    str payload;                   // payload to add to response

public:
    HttpResponse(ResponseStatusCode sc, str message = "") : statusCode(sc), message(message)
    {
        payload.reserve(FRONT_RESERVED_PAYLOAD_SPACE);
    }

    void addBody(strv body)
    {
        payload = str(body);
        contentLength = body.size();
    }

    str createResponse()
    {
        str response;
        response.reserve(FRONT_RESERVED_RESPONSE_SPACE);

        // status line
        response += PROTOCOL_LITERAL;
        response += " " + fromInt(statusCode);
        if (message.size() != 0)
            response += " " + message;
        response += "\r\n";

        // headers
        response += "Date: " + httpDateNow() + "\r\n";
        if (contentLength > 0)
            response += "Content-Length: " + fromInt(contentLength) + "\r\n";
        response += "\r\n"; // end headers

        // add paylaod if any
        if (contentLength > 0)
            response += payload;

        return response;
    }
};

#endif