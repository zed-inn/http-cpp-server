#include <cctype>
#include <vector>
#include "../utils/strs.hpp"
#include <unordered_map>

#ifndef _C_HTTP_TARGET_URI_
#define _C_HTTP_TARGET_URI_ 1

class HttpTargetUri
{
public:
    typedef std::vector<std::pair<std::string, std::string>> QueryParameters;
    static constexpr size_t MAX_TARGET_URI_LENGTH = 2048;

private:
    /// @brief Checks if a character is a valid character of a hex digit
    /// @param c Arbitrary character
    /// @return true if valid hex digit else false
    static constexpr bool isValidHexChar(char c)
    {
        return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
    }

    /// @brief Decode Uri Query String from Hex '%' formats and '+' characters
    /// @param s String pointing to the uri encoded query string
    static const void
    decodeUriQuery(str *s)
    {
        str temp;
        temp.reserve(2);
        int charNum;
        for (int i = 0; i < s->length(); i++)
        {
            if (s->at(i) == '+')
                s->replace(i, 1, " ");
            else if (s->at(i) == '%' && i + 2 < s->length() && isValidHexChar(s->at(i + 1)) && isValidHexChar(s->at(i + 2)))
            {
                temp.clear();
                charNum = stoi(s->substr(i + 1, 2), nullptr, 16);
                temp += charNum;
                s->replace(i, 3, temp);
            }
        }
    }

    /// @brief Decode Uri Path String from Hex '%' formats. Skips special characters
    /// @param s String pointing to the uri encoded path string
    static const void
    decodeUriPath(str *s)
    {
        str temp;
        int charNum;
        for (int i = 0; i < s->length(); i++)
            if (s->at(i) == '%' && i + 2 < s->length() && isValidHexChar(s->at(i + 1)) && isValidHexChar(s->at(i + 2)))
            {
                temp.clear();
                charNum = stoi(s->substr(i + 1, 2), nullptr, 16);
                if (charNum == '?' || charNum == '/' || charNum == '#') // Special characters that change the meaning to path
                    continue;

                temp = charNum;
                s->replace(i, 3, temp);
            }
    }

private:
    static constexpr size_t FRONT_RESERVED_URI_SPACE = 64;
    static constexpr size_t FRONT_RESERVED_QUERY_PARAMETER_SPACE = 64;
    static constexpr size_t FRONT_RESERVED_QUERY_VALUE_SPACE = 64;
    static constexpr size_t FRONT_RESERVED_QUERY_PAIRS_TOTAL_SPACE = 8;

    str _path;                       // String containing Uri Path
    QueryParameters queryParameters; // Query parameters map

    /// @brief Parse query parameters and stores them in the instance query parameters map
    /// @param s String containing the query string of a uri after '?'
    /// @param i Position the uri has been parsed till
    void parseQueryParameters(strv s, int i)
    {
        queryParameters.reserve(FRONT_RESERVED_QUERY_PAIRS_TOTAL_SPACE);

        str queryParam, queryVal;
        queryParam.reserve(FRONT_RESERVED_QUERY_PARAMETER_SPACE);
        queryParam.reserve(FRONT_RESERVED_QUERY_VALUE_SPACE);

        for (auto p = s.begin(); p < s.end() && *p != '#' && !isspace(*p) && i < MAX_TARGET_URI_LENGTH;)
        {
            // Clear param and val strings
            queryParam.clear(), queryVal.clear();

            // Write param
            while (p < s.end() && *p != '=' && *p != '&' && *p != '#' && !isspace(*p) && i < MAX_TARGET_URI_LENGTH)
                queryParam += *p++, i++;

            // Write value
            if (p < s.end() && *p == '=')
            {
                p++, i++;
                while (p < s.end() && *p != '&' && *p != '#' && !isspace(*p) && i < MAX_TARGET_URI_LENGTH)
                    queryVal += *p++, i++;
            }

            // Increase to point to next parameter
            if (p < s.end() && *p == '&')
                p++, i++;

            if (queryParam.length() == 0 && queryVal.length() == 0)
                continue;

            // Decode and append to the map
            decodeUriQuery(&queryParam);
            decodeUriQuery(&queryVal);
            queryParameters.push_back({queryParam, queryVal});
        }
    }

public:
    HttpTargetUri() {}

    /// @brief Parses a string containing http target uri to its path and query parameters
    /// @param s String containing target uri
    HttpTargetUri(strv s)
    {
        int i = 0;
        _path.reserve(FRONT_RESERVED_URI_SPACE);

        if (s.length() <= 0)
        {
            _path += '/';
            return;
        }

        auto p = s.begin();

        // If target uri starts with '/'
        if (s[0] == '/')
        {
            for (; p < s.end() && *p != '?' && i < MAX_TARGET_URI_LENGTH; p++, i++)
                _path += *p;

            if (p < s.end() && *p == '?')
                s.remove_prefix(i + 1), parseQueryParameters(s, i);
        }

        // Else starts with a character other than '/'
        else
        {
            for (; p < s.end() && *p != '/' && i < MAX_TARGET_URI_LENGTH; p++, i++)
                ;

            // If '/' was part of scheme
            if (*p == '/' && p - 1 >= s.begin() && *(p - 1) == ':' && p + 1 < s.end() && *(p + 1) == '/')
            {
                p += 2;
                for (; p < s.end() && *p != '/' && i < MAX_TARGET_URI_LENGTH; p++, i++)
                    ;

                // Found another '/' and after it starts path
                for (; p < s.end() && *p != '?' && i < MAX_TARGET_URI_LENGTH; p++, i++)
                    _path += *p;

                if (p < s.end() && *p == '?')
                    s.remove_prefix(i + 1), parseQueryParameters(s, i);
            }

            // Else '/' was part of authority and port
            else
            {
                for (; p < s.end() && *p != '?' && i < MAX_TARGET_URI_LENGTH; p++, i++)
                    _path += *p;

                if (p < s.end() && *p == '?')
                    s.remove_prefix(i + 1), parseQueryParameters(s, i);
            }
        }

        if (_path.length() == 0 || i == 0)
            _path += '/';

        decodeUriPath(&_path);
    }

    /// @brief Get pointer to the query parameters map
    /// @return pointer to query parameter map
    QueryParameters *getQueryParams()
    {
        return &queryParameters;
    }

    /// @brief Get path of the target uri
    /// @return String view of path
    strv path() const
    {
        return _path;
    }
};

#endif
