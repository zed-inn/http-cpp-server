#include "./strs.hpp"

#ifndef _C_HTTP_UTILS_
#define _C_HTTP_UTILS_ 1

#define isHexDigit(x) ((x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F'))

class HttpUtils
{
public:
    static constexpr strv httpSpaces{" \t"};

    static constexpr strv trimHttpSpaces(strv s)
    {
        auto left = s.find_first_not_of(httpSpaces);

        // all spaces
        if (left == strv::npos)
            return s.substr(0, 0);

        auto right = s.find_last_not_of(httpSpaces);

        return s.substr(left, right - left + 1);
    }

    static constexpr strv changeBeginning(strv s, const char *newb, const char *prev)
    {
        return strv(newb - prev + s.begin(), s.size());
    }
};

#endif