#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <vector>
#include <algorithm>
#include "../utils/strs.hpp"
#include "../utils/whitespaces.hpp"

#ifndef _C_HTTP_HEADERS_
#define _C_HTTP_HEADERS_ 1

#define BAD_CONTENT_LENGTH -1
#define NO_CONTENT_LENGTH -2

class HttpHeaders
{
public:
    typedef std::unordered_map<std::string, std::vector<std::string>> HeaderMap;

    static constexpr size_t MAX_HEADER_FIELD_KEY_LENGTH = 64;
    static constexpr size_t MAX_HEADER_FIELD_VALUE_LENGTH = 8192;
    static constexpr size_t MAX_HEADER_FIELDS_LENGTH = 8192;

private:
    static constexpr unsigned int FRONT_RESERVED_FIELD_KEY_SPACE = 16;
    static constexpr unsigned int FRONT_RESERVED_FIELD_VALUE_SPACE = 32;

    static inline const std::unordered_set<char> ALLOWED_CHARS_IN_FIELD_KEY{
        '!', '#', '$', '%', '&', '\'', '*', '+', '-', '.', '^', '_', '`', '|', '~'};

    HeaderMap headers;

    auto it(strv s)
    {
        return headers.find(tolower(s));
    }

public:
    /// @brief parse a header line and stores the parsed key-value pair in the map
    /// @param s header line to be parsed
    /// @return true if parsed correctly and no not-allowed characters are present
    bool parseAnother(strv s)
    {
        int l = 0; // length of key/field
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
        while (isWS(*ps)) // remove prefix and suffix whitespaces
            ps++;
        while (isWS(*pe))
            pe--;

        // get the key (lowered) if all allowed characters and less than max length
        l = 0;
        while (ps <= pe)
        {
            // allowed in header keys
            if (l < MAX_HEADER_FIELD_KEY_LENGTH && (isalnum(*ps) || ALLOWED_CHARS_IN_FIELD_KEY.count(*ps)))
                key += tolower(*ps++), l++;
            else
                return false;
        }

        // if no key found
        if (key.size() == 0)
            return false;

        // Get the value
        ps = colonPos + 1, pe = s.end() - 1;
        while (isWS(*ps)) // remove prefix and suffix whitespaces
            ps++;
        while (isWS(*pe))
            pe--;

        // get the value if allowed characters and less than max length
        l = 0;
        while (ps <= pe)
        {
            // allowed in header values
            if (l < MAX_HEADER_FIELD_VALUE_LENGTH && (*ps == '\t' || (*ps >= 32 && *ps <= 126) || *ps >= 128))
                value += *ps++, l++;
            else
                return false;
        }

        headers[key].push_back(value);
        return true;
    }

    // Checks if a key exists in the parsed headers
    bool exists(strv s)
    {
        return it(s) != headers.end();
    }

    /// @brief Gives content-length if given in parsed headers
    /// @return valid positive value if given and valid, else BAD_CONTENT_LENGTH if given and invalid, else NO_CONTENT_LENGTH
    int contentLength()
    {
        auto itx = it("content-length");
        if (itx != headers.end())
        {
            auto values = &((*itx).second);
            if ((*values).size() > 1 || (*values)[0].length() == 0)
                return BAD_CONTENT_LENGTH;

            // only digits are allowed
            for (auto i = (*values)[0].begin(); i < (*values)[0].end(); i++)
                if (*i < '0' || *i > '9')
                    return BAD_CONTENT_LENGTH;

            return stoi((*values)[0]);
        }

        return NO_CONTENT_LENGTH;
    }

    /// @brief Checks if all the "known" header key-value pairs are valid or not, e.g. content-length = number, or TE = known value
    /// @return true if "known" header values are acceptable, else false
    bool isValidByValue()
    {
        HeaderMap::iterator itx;
        std::vector<std::string> *values;

        // Check if content length is only one and only contains digits (>=0)
        if ((itx = it("content-length")) != headers.end())
        {
            values = &((*itx).second);
            if ((*values).size() > 1)
                return false;

            if ((*values)[0].length() == 0)
                return false;

            // only digits are allowed
            for (auto i = (*values)[0].begin(); i < (*values)[0].end(); i++)
                if (*i < '0' || *i > '9')
                    return false;
        }

        // Check if transfer encoding is present, then has only valid values
        // TODO: transfer encoding checking
        // if ((itx = it("transfer-encoding")) != headers.end())
        // {
        //     // values = &((*itx).second);
        //     // for (auto i = (*values).begin(); i < (*values).end(); i++)
        //     // {
        //     //     if ((*i).length() == 0)
        //     //         return false;
        //     // }
        //     // Not Implemented yet
        // }

        return true;
    }

    HeaderMap *headerMap()
    {
        return &headers;
    }
};

#endif