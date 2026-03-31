#include <set>
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
    static const inline std::set<std::pair<V, V>> VALID_VERSIONS{{0, 9}, {1, 0}, {1, 1}, {2, 0}, {3, 0}};

    Version *_prot;

public:
    HttpProtocol(Version *vp) : _prot(vp) {}

    ParseResult parse(strv s)
    {
        if (s.length() != FIXED_PROTOCOL_LENGTH)
            return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Protocol"));

        auto p = s.begin();

        // parse the first 5 chars 'http/'
        if (*p != 'H' || *(p + 1) != 'T' || *(p + 2) != 'T' || *(p + 3) != 'P' || *(p + 4) != '/')
            return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Protocol"));
        p += 5;

        // check the versions
        if (isdigit(*p) && *(p + 1) == '.' && isdigit(*(p + 2)))
        {
            V major = *p - '0', minor = *(p + 2) - '0';
            if (VALID_VERSIONS.count({major, minor}) != 0)
            {
                *_prot = Version(major, minor);
                return ParseResult();
            }
        }
        return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Protocol Version"));
    }
};

#endif