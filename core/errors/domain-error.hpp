#include "../utils/strs.hpp"
#include "../utils/http-status-codes.hpp"

#ifndef _C_HTTP_DOMAIN_ERROR_
#define _C_HTTP_DOMAIN_ERROR_ 1

struct DomainError
{
    HttpStatusCode::IANAStatusCode code;
    str reason;

    DomainError() : code(HttpStatusCode::OK) {}
    DomainError(HttpStatusCode::IANAStatusCode code, str message) : code(code), reason(message) {}
};

#endif