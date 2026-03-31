#include <string>
#include <algorithm>

#ifndef _C_HTTP_STRING_TYPEDEFS_
#define _C_HTTP_STRING_TYPEDEFS_ 1

typedef std::string str;
typedef std::string_view strv;

// Lowercase the string
str tolower(strv s)
{
    str lowercased(s);
    transform(lowercased.begin(), lowercased.end(), lowercased.begin(), [](unsigned char c)
              { return tolower(c); });
    return lowercased;
}

#endif