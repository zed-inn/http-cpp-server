#include "../utils/strs.hpp"
#include "../errors/domain-error.hpp"

#ifndef _C_HTTP_DOMAIN_INTERFACE_PARSER_
#define _C_HTTP_DOMAIN_INTERFACE_PARSER_ 1

struct ParseResult
{
    bool success;
    DomainError error;

    ParseResult() : success(true) {}
    ParseResult(DomainError error) : success(false), error(error) {}
};

class Parser
{
public:
    virtual ParseResult parse(strv s)
    {
        return ParseResult();
    };
};

#endif