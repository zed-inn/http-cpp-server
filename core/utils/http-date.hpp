#include "./strs.hpp"
#include <ctime>

#ifndef _C_HTTP_DATE_
#define _C_HTTP_DATE_ 1

str httpDateNow()
{
    static const char *wkday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    std::time_t t = std::time(nullptr);
    std::tm gmt;
    gmtime_r(&t, &gmt);

    char buf[31];
    std::snprintf(buf, sizeof(buf), "%s, %02d %s %04d %02d:%02d:%02d GMT",
                  wkday[gmt.tm_wday],
                  gmt.tm_mday,
                  month[gmt.tm_mon],
                  1900 + gmt.tm_year,
                  gmt.tm_hour,
                  gmt.tm_min,
                  gmt.tm_sec);
    return str(buf);
}

#endif