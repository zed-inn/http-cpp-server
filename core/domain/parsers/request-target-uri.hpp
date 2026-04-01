#include <vector>
#include <unordered_map>
#include "../interface/parser.hpp"
#include "../utils/macros.hpp"

#ifndef _C_HTTP_TARGET_URI_PARSER_
#define _C_HTTP_TARGET_URI_PARSER_ 1

class SchemeValidation
{
public:
    static const inline bool isHttp(const char *p, const char *e)
    {
        return (p + 6 < e && *p == 'h' && *(p + 1) == 't' && *(p + 2) == 't' && *(p + 3) == 'p' && *(p + 4) == ':' && *(p + 5) == '/' && *(p + 6) == '/');
    }

    static const inline bool isHttps(const char *p, const char *e)
    {
        return (p + 7 < e && *p == 'h' && *(p + 1) == 't' && *(p + 2) == 't' && *(p + 3) == 'p' && *(p + 4) == 's' && *(p + 5) == ':' && *(p + 6) == '/' && *(p + 7) == '/');
    }
} typedef Scheme;

class AuthorityValidation
{
private:
    static const inline bool isIPv6PrefixOfConvIPv4(const char *p, const char *e)
    {
        return (p + 7 < e && *p == ':' && *(p + 1) == ':' && *(p + 2) == 'f' && *(p + 3) == 'f' && *(p + 4) == ':' && *(p + 5) == '0' && *(p + 6) == '0' && *(p + 7) == ':');
    }

public:
    static const inline bool isValidIPv6(const char *p, const char *e, const char **next)
    {
        // Check if converted from ipv4
        if (isIPv6PrefixOfConvIPv4(p, e))
            return isValidIPv4(p + 8, e, next);

        const char *validTill = p;
        unsigned totalLen = 0;
        unsigned short total2xColons = 0, totalColons = 0, totalHexDigits = 0;

        // till it contains <=32 hexdigits and <=7 colons and <=1 2x-colon
        while (validTill < e && (isHexDigit(*validTill) || *validTill == ':'))
        {
            if (*validTill == ':')
            {
                totalColons++;
                if (validTill + 1 < e && *(validTill + 1) == ':')
                    total2xColons++;
            }
            else
                totalHexDigits++;

            if (total2xColons > 1 || totalColons > 7 || totalHexDigits > 32)
                return false;

            validTill++;
        }

        totalLen = validTill - p;
        if (totalLen > 39)
            return false;

        // atleast two colons are necessary
        if (totalLen < 2)
            return false;
        else if (totalLen == 2)
            return *p == ':' && *(p + 1) == ':';
        else if (*p == ':' && *(validTill - 1) == ':')
            return false;

        // check if each colon contains <=4 digits
        int dgs, fields = 0;
        const char *r;
        for (auto q = p; q < validTill;)
        {
            dgs = 0;
            for (r = q; r < validTill && *r != ':'; r++)
                dgs++;
            fields++; // increase fields

            if (dgs > 4)
                return false;

            r += *r == ':';
            r += r < validTill && *r == ':';
            q = r;
        }

        // if colon, then fields could be less
        if (total2xColons)
        {
            *next = validTill; // set here too
            return fields <= 8;
        }

        if (fields == 8)
        {
            *next = validTill; // point to next one
            return true;
        }

        return false;
    }

    static const inline bool isValidIPv4(const char *p, const char *e, const char **next)
    {

        auto validTill = p;
        int totalNums = 0, totalDots = 0, totalLen = 0;

        // check till valid chars
        while (validTill < e && ((*validTill >= '0' && *validTill <= '9') || *validTill == '.'))
        {

            if (*validTill == '.')
            {
                if (validTill + 1 < e && *(validTill + 1) == '.')
                    return false;
                totalDots++;
            }
            else
                totalNums++;

            if (totalDots > 3 || totalNums > 12)
                return false;

            validTill++;
        }

        totalLen = validTill - p;
        if (totalLen < 6 || totalLen > 15)
            return false;

        if (*p == '.' || *(validTill - 1) == '.')
            return false;

        // check if numbers in 0-255
        str s;
        s.reserve(3);
        const char *r;
        int temp, fields = 0;
        for (auto q = p; q < validTill;)
        {
            s.clear();
            r = q;
            while (r < validTill && *r != '.')
                s += *r++;
            fields++;

            temp = stoi(s);
            if (temp < 0 || temp > 255)
                return false;

            r += *r == '.';
            q = r;
        }

        // must have 4 fields
        if (fields == 4)
        {
            *next = validTill; // point to next one
            return true;
        }

        return false;
    }

    static const inline bool isValidDomainName(const char *p, const char *e, const char **next)
    {
        auto validTill = p;

        while (validTill < e && (isalnum(*validTill) || *validTill == '.'))
            validTill++;

        // should have at least one length
        if (validTill - p > 0)
        {
            *next = validTill;
            return true;
        }

        return false;
    }

    static const inline bool isValidPortNumber(const char *p, const char *e, const char **next)
    {
        int temp = 0, len = 0;

        while (p < e && (*p >= '0' && *p <= '9') && len <= 5)
        {
            temp = temp * 10 + (*p - '0');
            len++, p++;
        }

        if (temp >= 0 && temp <= 65535)
        {
            *next = p;
            return true;
        }

        return false;
    }
};

class HttpTargetUri : Parser
{
public:
    typedef std::unordered_map<str, str> Query;
    static constexpr size_t MAX_TARGET_URI_LENGTH = 2048;

private:
    static constexpr size_t MAX_QUERY_KEY_LENGTH = 256;
    static constexpr size_t MAX_QUERY_VALUE_LENGTH = 2048;

    static constexpr size_t FRONT_RESERVED_PATH_SPACE = 64;
    static constexpr size_t FRONT_RESERVED_QUERY_KEY_SPACE = 32;
    static constexpr size_t FRONT_RESERVED_QUERY_VALUE_SPACE = 64;

    ParseResult parsePath(const char **_p, const char *e)
    {
        auto p = *_p;
        int charNum;

        while (p < e && *p != '?')
        {
            // %Hexhex uri decoding
            if (*p == '%' && p + 2 < e && isHexDigit(*(p + 1)) && isHexDigit(*(p + 2)))
            {
                charNum = stoi(str({*(p + 1), *(p + 2)}), nullptr, 16);

                // if special characters that can change meaning of the path, skip
                if (charNum == '?' || charNum == '/' || charNum == '#')
                    *_path += *p++;
                else
                    *_path += charNum, p += 3;
            }

            // invalid use of %
            else if (*p == '%')
                return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid URI"));

            else
                *_path += *p++;
        }

        *_p = p;
        return ParseResult();
    }

    ParseResult parseQuery(const char **_p, const char *e)
    {
        auto p = *_p;

        str queryKey, queryVal;
        queryKey.reserve(FRONT_RESERVED_QUERY_KEY_SPACE);
        queryVal.reserve(FRONT_RESERVED_QUERY_VALUE_SPACE);

        // till fragment or string over
        while (p < e && *p != '#')
        {
            // clear both the container strings
            queryKey.clear(), queryVal.clear();

            // collect key
            while (p < e && *p != '=' && *p != '&' && *p != '#')
            {
                // %Hexhex uri decoding
                if (*p == '%' && p + 2 < e && isHexDigit(*(p + 1)) && isHexDigit(*(p + 2)))
                    queryKey += stoi(str({*(p + 1), *(p + 2)}), nullptr, 16), p += 3;

                // invalid use of %
                else if (*p == '%')
                    return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid URI"));

                // any other character
                else
                    queryKey += *p == '+' ? ' ' : *p, p++;
            }

            if (p < e && *p == '=')
            {
                p++;

                // collect value
                while (p < e && *p != '#' && *p != '&')
                {
                    // %Hexhex uri decoding
                    if (*p == '%' && p + 2 < e && isHexDigit(*(p + 1)) && isHexDigit(*(p + 2)))
                        queryVal += stoi(str({*(p + 1), *(p + 2)}), nullptr, 16), p += 3;

                    // invalid use of %
                    else if (*p == '%')
                        return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid URI"));

                    // any other character
                    else
                        queryVal += *p == '+' ? ' ' : *p, p++;
                }
            }

            // move to next query
            if (p < e && *p == '&')
                p++;

            // if no key and no value
            if (!queryKey.length() && !queryVal.length())
                continue;

            _query->insert({queryKey, queryVal});
        }

        *_p = p;
        return ParseResult();
    }

    str *_path;
    Query *_query;
    bool *_absform;

public:
    HttpTargetUri(str *pathp, Query *qp, bool *absform) : _path(pathp), _query(qp), _absform(absform)
    {
        _path->reserve(FRONT_RESERVED_PATH_SPACE);
    }

    ParseResult parse(strv s) override
    {

        if (s.length() > MAX_TARGET_URI_LENGTH)
            return ParseResult(DomainError(HttpStatusCode::URI_TOO_LONG, "URI Too Long"));

        if (s.length() <= 0)
            return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "No URI"));

        auto p = s.begin();
        ParseResult res = ParseResult();

        // If does not start with *, /, or alphabets
        if (*p != '*' && *p != '/' && !isalpha(*p))
            return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid URI Characters"));

        // It should not have any http spaces
        {
            auto pn = p;
            while (pn < s.end())
            {
                if (isspace(*pn))
                    return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid URI"));
                pn++;
            }
        }

        // If starts with *
        if (*p == '*')
        {
            if (s.length() != 1)
                return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid URI"));
            else
            {
                *_path += "/";
                return ParseResult();
            }
        }

        // If starts with /, will count as path starting
        if (*p == '/')
            res = parsePath(&p, s.end());

        // If starts with some alphabet, either scheme or authority
        else
        {
            bool schemeFound = true;
            *_absform = true;

            // if scheme, then http/s
            if (Scheme::isHttp(p, s.end()))
                p += 7;
            else if (Scheme::isHttps(p, s.end()))
                p += 8;
            else
            {
                schemeFound = false, *_absform = false;

                // Check if :// appears
                auto x = p;
                while (x < s.end() && *x != ':')
                    x++;
                if (x < s.end() && *x == ':' && x + 2 < s.end() && *(x + 1) == '/' && *(x + 2) == '/')
                    return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Scheme"));
            }

            // if authority given is ipv6
            auto next = p;
            if (*p == '[')
            {
                p++;

                if (!AuthorityValidation::isValidIPv6(p, s.end(), &next))
                    return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Authority"));
                p = next;

                if (*p == ']')
                    p++;
                else
                    return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Authority"));
            }

            // if authority given is ipv4
            else if (AuthorityValidation::isValidIPv4(p, s.end(), &next))
                p = next;

            // if domain name
            else if (AuthorityValidation::isValidDomainName(p, s.end(), &next))
                p = next;

            else
                return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Authority"));

            // must be colon or /
            if (p >= s.end() || (*p != ':' && *p != '/' && *p != '?' && *p != '#'))
                return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Authority"));

            // if port given
            if (*p == ':')
            {
                p++;

                next = p;
                if (!AuthorityValidation::isValidPortNumber(p, s.end(), &next))
                    return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Authority"));
                p = next;

                // authority form with something else
                if (!schemeFound && p < s.end())
                    return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Target URI"));

                // path with absolute form
                if (*p == '/' && schemeFound)
                    res = parsePath(&p, s.end());
                else
                    return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Authority"));
            }

            // path starts here
            else if (*p == '/')
            {
                // with absolute form
                if (schemeFound)
                    res = parsePath(&p, s.end());
                else
                    return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Authority"));
            }
        }

        // If error in parsing path
        if (!res.success)
            return res;

        // If path but nothing get means implicit root
        if (_path->length() == 0)
            *_path += "/";

        // Parse query
        if (p < s.end() && *p == '?')
        {
            p++;
            res = parseQuery(&p, s.end());
        }

        // If error in parsing query
        if (!res.success)
            return res;

        // dont' have to parse fragment

        return ParseResult();
    }
};

#endif