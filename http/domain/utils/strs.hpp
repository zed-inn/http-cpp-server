#include <string>

#ifndef _C_HTTP_STRING_TYPEDEFS_
#define _C_HTTP_STRING_TYPEDEFS_ 1

typedef std::string str;
typedef std::string_view strv;

/// @brief Return the lowercased version of s
/// @param s string view s
/// @return a lowercased string version of s
str tolower(strv s)
{
    str lowercased(s);
    transform(lowercased.begin(), lowercased.end(), lowercased.begin(), [](unsigned char c)
              { return tolower(c); });
    return lowercased;
}

#endif