#ifndef _C_HTTP_HELPFUL_MACROS_
#define _C_HTTP_HELPFUL_MACROS_

#define isHttpSpace(x) (x == ' ' || x == '\t')

#define isHexDigit(x) ((x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F'))

#endif