#include <cctype>
#include "../utils/strs.hpp"

#ifndef _C_HTTP_VERSION_
#define _C_HTTP_VERSION_ 1

class HttpProtocol
{
public:
    static constexpr size_t MAX_PROTOCOL_LENGTH = 8;

private:
    typedef std::pair<int, int> Version;
    static constexpr char HTTP_LITERAL[] = "HTTP";

    Version version{-1, -1};
    bool valid = true;

public:
    HttpProtocol() {}

    /// @brief Parses a string containg http protocol with versions
    /// @param s String containing protocol and version
    HttpProtocol(strv s)
    {
        auto p = s.begin();
        auto h = HTTP_LITERAL;

        // Check if first word in s is 'http'
        while (*h != '\0' && *p != '\0' && p < s.end() && *h == toupper(*p))
            h++, p++;
        if (*h != '\0')
        {
            valid = false;
            return;
        }

        // Check if the next sequence is '/DIGIT.DIGIT'
        if (p < s.end() && p + 3 < s.end() && p + 4 == s.end() && *p == '/' && isdigit(*(p + 1)) && *(p + 2) == '.' && isdigit(*(p + 3)))
        {
            version = {*(p + 1) - '0', *(p + 3) - '0'};
            return;
        }

        valid = false;
        return;
    }

    /// @brief Check if the parsed result is invalid
    /// @return true if invalid
    bool isInvalid() const
    {
        return !valid;
    }

    /// @brief Gets the major version parsed from the protocol
    /// @return 0, 1, 2, 3
    int majorVersion() const
    {
        return version.first;
    }

    /// @brief Gets the minor version parsed from the protocol
    /// @return 0, 1, 9
    int minorVersion() const
    {
        return version.second;
    }
};

#endif