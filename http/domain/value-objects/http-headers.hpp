#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <vector>
#include "../utils/strs.hpp"

#ifndef _C_HTTP_HEADERS_
#define _C_HTTP_HEADERS_ 1

class HttpHeaders
{
public:
    typedef std::unordered_map<std::string, std::vector<std::string>> HeaderMap;

private:
    static constexpr unsigned int MAX_HEADER_FIELD_KEY_LENGTH = 64;
    static constexpr unsigned int MAX_HEADER_FIELD_VALUE_LENGTH = 8192;
    static constexpr unsigned int MAX_HEADER_FIELDS_LENGTH = 8192;

    static constexpr unsigned int FRONT_RESERVED_FIELD_KEY_SPACE = 16;
    static constexpr unsigned int FRONT_RESERVED_FIELD_VALUE_SPACE = 32;

    static inline const std::unordered_set<char> ALLOWED_CHARS_IN_FIELD_KEY{
        '!', '#', '$', '%', '&', '\'', '*', '+', '-', '.', '^', '_', '`', '|', '~'};

    HeaderMap headers;

public:
    /// @brief parse a header line and stores the parsed key-value pair in the map
    /// @param s header line to be parsed
    /// @return true if parsed correctly and no not-allowed characters are present
    bool parseAnother(strv s)
    {
        str key, value;
        key.reserve(FRONT_RESERVED_FIELD_KEY_SPACE);
        value.reserve(FRONT_RESERVED_FIELD_VALUE_SPACE);

        // Find the first colon
        auto colonPos = s.begin();
        while (colonPos < s.end() && *colonPos != ':')
            colonPos++;
        if (colonPos >= s.end())
            return false;

        // Get the key
        int i;
        auto ps = s.begin(), pe = colonPos - 1;
        while (*ps == ' ') // remove prefix and suffix whitespaces
            ps++;
        while (*pe == ' ')
            pe--;

        // get the key if all allowed characters
        while (ps <= pe)
        {
            // allowed in header keys
            if (isalnum(*ps) || ALLOWED_CHARS_IN_FIELD_KEY.count(*ps))
                key += tolower(*ps++);
            else
                return false;
        }

        // if no key found
        if (key.size() == 0)
            return false;

        // Get the value
        ps = colonPos + 1, pe = s.end() - 1;
        while (*ps == ' ') // Remove prefix and suffix whitespaces
            ps++;
        while (*pe == ' ')
            pe--;

        // get the value if allowed characters
        while (ps <= pe)
        {
            // allowed in header values
            if (*ps == '\t' || (*ps >= 32 && *ps <= 126) || *ps >= 128)
                value += *ps++;
            else
                return false;
        }

        headers[key].push_back(value);
        return true;
    }

    HeaderMap *headerMap()
    {
        return &headers;
    }
};

#endif