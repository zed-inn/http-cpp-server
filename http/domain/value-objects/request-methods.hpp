#include <cctype>
#include "../utils/strs.hpp"

#ifndef _C_HTTP_REQUEST_METHOD_
#define _C_HTTP_REQUEST_METHOD_ 1

class HttpRequestMethod
{
public:
    typedef unsigned short Name; // Http Request Method Name
    static constexpr int MAX_METHOD_NAME_LENGTH = 7;

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
    static constexpr char _GET[] = "GET";
    static constexpr char _POST[] = "POST";
    static constexpr char _HEAD[] = "HEAD";
    static constexpr char _DELETE[] = "DELETE";
    static constexpr char _PUT[] = "PUT";
    static constexpr char _OPTIONS[] = "OPTIONS";
    static constexpr char _TRACE[] = "TRACE";
    static constexpr char _PATCH[] = "PATCH";
    static constexpr char _CONNECT[] = "CONNECT";

    /// @brief Maps a string to the value of the method
    /// @param methodName string representing method
    /// @return Name(int) value of the http method or 'INVALID'
    static Name
    mapped(strv methodName)
    {
        if (methodName.compare(_GET) == 0)
            return GET;
        else if (methodName.compare(_POST) == 0)
            return POST;
        else if (methodName.compare(_HEAD) == 0)
            return HEAD;
        else if (methodName.compare(_DELETE) == 0)
            return DELETE;
        else if (methodName.compare(_PUT) == 0)
            return PUT;
        else if (methodName.compare(_OPTIONS) == 0)
            return OPTIONS;
        else if (methodName.compare(_TRACE) == 0)
            return TRACE;
        else if (methodName.compare(_PATCH) == 0)
            return PATCH;
        else if (methodName.compare(_CONNECT) == 0)
            return CONNECT;
        else
            return INVALID;
    }

public:
    Name name;

    HttpRequestMethod(strv s) : name(mapToMethod(s)) {}

    /// @brief Maps a string to its representing http method. Strict checking to ensure proper meaning. Case insensitive.
    /// @param s String containing http method
    /// @return Name(int) value of the http method or 'INVALID'
    static Name
    mapToMethod(strv s)
    {
        if (s.length() > MAX_METHOD_NAME_LENGTH)
            return INVALID;

        str name;
        name.reserve(MAX_METHOD_NAME_LENGTH);
        for (auto i = s.begin(); i < s.end(); i++)
            name += toupper(*i);

        return mapped(name);
    }
};

#endif