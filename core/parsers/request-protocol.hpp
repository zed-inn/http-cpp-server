#include <array>
#include "../interface/parser.hpp"

#ifndef _C_HTTP_PROTOCOL_PARSER_
#define _C_HTTP_PROTOCOL_PARSER_ 1

class HttpProtocol : Parser
{
public:
    typedef unsigned char V;

    struct ProtocolVersion
    {
        V major, minor;

        ProtocolVersion() : major(0), minor(0) {};
        ProtocolVersion(V major, V minor) : major(major), minor(minor) {}

    } typedef Version;

    static constexpr size_t FIXED_PROTOCOL_LENGTH = 8;

private:
    static constexpr bool isValidVersion(V major, V minor)
    {
        if (major == 0 && minor == 9)
            return true;
        else if (major == 1 && (minor == 0 || minor == 1))
            return true;
        else if (minor == 0 && (major == 2 || major == 3))
            return true;
        return false;
    }

    Version *_prot;

public:
    HttpProtocol(Version *vp) : _prot(vp) {}

    ParseResult parse(strv s)
    {
        if (s.size() != FIXED_PROTOCOL_LENGTH)
            return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Protocol");

        // Should start with "HTTP/"
        auto front = s.find("HTTP/");
        if (front != 0)
            return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Protocol");

        // Check the versions
        auto p = s.begin() + 5;
        if (isdigit(*p) && *(p + 1) == '.' && isdigit(*(p + 2)))
        {
            V major = *p - '0', minor = *(p + 2) - '0';
            if (isValidVersion(major, minor))
            {
                _prot->major = major, _prot->minor = minor;
                return ParseResult();
            }
        }

        // Invalid Version Format or not a digit in them
        return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Protocol Version");
    }
};

#endif