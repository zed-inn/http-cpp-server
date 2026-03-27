#ifndef _C_HTTP_UTILS_WHITESPACE_CHARACTERS_
#define _C_HTTP_UTILS_WHITESPACE_CHARACTERS_ 1

// Whitespace
#define isWS(x) (x == ' ' || x == '\t')

// Carriage return
#define isCR(x) (x == '\r')

// Line feed
#define isLF(x) (x == '\n')

// Carriage return and line feed
#define isCRLF(x, y) (isCR(x) && isLF(y))

// CRLF twice
#define is2xCRLF(w, x, y, z) (isCRLF(w, x) && isCRLF(y, z))

#endif