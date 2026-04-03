#include "../utils/strs.hpp"
#include "../utils/http-date.hpp"

#ifndef _C_HTTP_RESPONSE_
#define _C_HTTP_RESPONSE_ 1

/*
Minimal Http Response generator
You can add other headers except for Date/Server/Content-Length which will be set automatically
Content-Type by default would be set to text/plain

Protocol using is HTTP/1.1
Transfer encoding is not supported
*/
class HttpResponse
{
private:
    static constexpr size_t FRONT_RESERVED_RESPONSE_SPACE = 128;

    static constexpr const char *PROTOCOL_LITERAL = "HTTP/1.1";

    HttpStatusCode::IANAStatusCode statusCode; // response status code
    str message;                               // small message to be put in response line
    str payload;                               // payload to add to response
    std::unordered_map<str, str> headers;      // headers to be sent in response

public:
    void setStatusCode(HttpStatusCode::IANAStatusCode sc)
    {
        statusCode = sc;
    }

    void setReponseLineMesssage(str message)
    {
        message = message;
    }

    void addBody(str body)
    {
        // Can't add more body once added
        if (payload.size() > 0)
            return;

        payload = body;
    }

    void addHeader(str key, str val)
    {
        // Don't set headers to be set automatically for content-length, server and date
        str loweredName = tolower(key);
        if (loweredName.compare("content-length") == 0 || loweredName.compare("server") == 0 || loweredName.compare("date"))
            return;

        headers[key] = val;
    }

    str createResponse()
    {
        str response;
        response.reserve(FRONT_RESERVED_RESPONSE_SPACE);

        // Response status line
        response += PROTOCOL_LITERAL;
        response += " " + std::to_string(statusCode);
        if (message.size() != 0)
            response += " " + message;
        response += "\r\n";

        // Set headers
        for (auto x : headers)
            response += x.first + ": " + x.second + "\r\n";
        response += "Date: " + httpDateNow() + "\r\n";
        response += "Server: Localhost:8080\r\n"; // Currently hard-coded, TODO: Create environment variables
        if (payload.size() > 0)
        {
            // Put content length and type only when payload set
            response += "Content-Length: " + std::to_string(payload.size()) + "\r\n";
            response += "Content-Type: " + (headers.find("Content-Type") != headers.end() ? headers["Content-Type"] : "text/plain") + "\r\n";
        }
        response += "\r\n"; // end headers

        // Add payload if any
        if (payload.size() > 0)
            response += payload;

        return response;
    }
};

#endif