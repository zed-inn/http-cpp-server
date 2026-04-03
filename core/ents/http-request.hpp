#include "../interface/parser.hpp"
#include "../parsers/request-method.hpp"
#include "../parsers/request-target-uri.hpp"
#include "../parsers/request-protocol.hpp"
#include "../parsers/http-headers.hpp"

#ifndef _C_HTTP_REQUEST_PARSER_
#define _C_HTTP_REQUEST_PARSER_ 1

/*
Parsing Http Request work only when the whole request is in one memory location.
Submitting broken parts from that memory location works.
Broken parts must be provided in order for request to be parsed as valid.

Using this parser to parse a request broken in-memory in different location may lead
to unexpected results. It is therefore not recommended.
*/
class HttpRequest : Parser
{
public:
    static constexpr size_t MAX_REQUEST_SIZE = 16384;

private:
    typedef unsigned char Task;

    // Continous spaces allowed for not more than 32 characters
    static constexpr unsigned short CONTINOUS_OBSP_ALLOWED = 32;

    // Task mapped to numbers for easier checking
    static constexpr Task NO_TASK = 0;
    static constexpr Task PARSE_METHOD = 1;
    static constexpr Task PARSE_TARGET_URI = 2;
    static constexpr Task PARSE_PROTOCOL = 3;
    static constexpr Task PARSE_REQUEST_LINE_ENDING = 4;
    static constexpr Task PARSE_HEADER_LINE = 5;
    static constexpr Task PARSE_HEADER_LINE_ENDING = 6;
    static constexpr Task PARSE_PAYLOAD = 7;
    static constexpr Task PARSE_RWS = 8;
    static constexpr Task PARSE_OWS = 9;
    static constexpr Task PARSE_BWS = 10;

    // Methods currently supported
    static const inline std::unordered_set<HttpRequestMethod::Name> ALLOWED_METHODS{HttpRequestMethod::GET};

    // Current (parsing) task to perform
    Task current, prev = NO_TASK;

    // Returns a parse result with removing any task remaining
    ParseResult ErrorOccured(HttpStatusCode::IANAStatusCode sc, str reason)
    {
        current = NO_TASK;              // Set task to be NO_TASK, so to not do any parsing anymore
        return ParseResult(sc, reason); // return the parse result
    }

    // Previous pointer in the continous location
    const char *saved = NULL;

    // Method name
    HttpRequestMethod::Name _method;

    // Target Uri Context
    HttpTargetUri::Context _uricontext;

    // Protocol Version;
    HttpProtocol::Version _protVersion;

    // Headers Context
    HttpHeaders::Context _hcontext;

    // Validate headers
    ParseResult validateHeaders()
    {
        str lowercasedKey;
        HttpHeaderKey::Name name;
        HttpHeaderKey::Map::const_iterator it;

        // Loop over all headers
        for (auto line : _hcontext.mapped)
        {
            // lowercase the current key and match with named keys
            lowercasedKey = tolower(line.first);

            // Skip non-named keys
            it = HttpHeaderKey::NamedKeys.find(lowercasedKey);
            if (it == HttpHeaderKey::NamedKeys.end())
                continue;
            name = it->second; // Set name to current

            // Validate by name, payload unsupported
            if (name == HttpHeaderKey::TRANSFER_ENCODING || (name == HttpHeaderKey::CONTENT_LENGTH && stoi(str(line.second[0])) > 0))
                return ParseResult(HttpStatusCode::NOT_IMPLEMENTED, "Payload Unsupported");
        }

        return ParseResult();
    }

public:
    HttpRequest() : current(PARSE_METHOD) {}

    ParseResult parse(strv s)
    {
        if (current == NO_TASK)
            return ParseResult(HttpStatusCode::INTERNAL_SERVER_ERROR, "No Parsing Task Remain");

        // Join the saved pointer from before to new strv
        if (saved != nullptr)
            s = strv(saved, (s.begin() - saved) + s.size());

        // To store intermediate results
        strv temp;
        ParseResult res;

        while (current != NO_TASK)
        {
            // Parse method
            if (current == PARSE_METHOD)
            {
                // Find the http space
                auto sp = s.find_first_of(" \t");
                if (sp == strv::npos)
                {
                    if (s.size() > HttpRequestMethod::MAX_METHOD_LENGTH)
                        return ErrorOccured(HttpStatusCode::METHOD_NOT_ALLOWED, "Invalid Method");

                    // continue in next session
                    saved = s.begin(); // save the current beginning of the s
                    return ParseResult();
                }

                // Found a space in the given request part but at a higher index than allowed
                if (sp > HttpRequestMethod::MAX_METHOD_LENGTH)
                    return ErrorOccured(HttpStatusCode::METHOD_NOT_ALLOWED, "Invalid Method");

                // Get the method name
                temp = s.substr(0, sp);
                HttpRequestMethod m = HttpRequestMethod(&_method);
                res = m.parse(temp);

                if (!res.success || _method == HttpRequestMethod::INVALID)
                    return ErrorOccured(HttpStatusCode::METHOD_NOT_ALLOWED, "Invalid Method");

                if (!ALLOWED_METHODS.count(_method))
                    return ErrorOccured(HttpStatusCode::METHOD_NOT_ALLOWED, "Method Not Supported");

                // Point the s to the new position and change task to parse RWS
                s.remove_prefix(sp), saved = nullptr;
                current = PARSE_RWS, prev = PARSE_METHOD;
            }

            // Parse Http spaces
            else if (current == PARSE_RWS || current == PARSE_OWS || current == PARSE_BWS)
            {
                // Find the non-space character
                auto nsp = s.find_first_not_of(" \t");
                if (nsp == strv::npos)
                {
                    // If spaces reached limit
                    if (s.size() > CONTINOUS_OBSP_ALLOWED)
                        return ErrorOccured(HttpStatusCode::BAD_REQUEST, "Continous Spaces Limit Reached");

                    saved = s.begin();
                    return ParseResult();
                }

                if (nsp > CONTINOUS_OBSP_ALLOWED)
                    return ErrorOccured(HttpStatusCode::BAD_REQUEST, "Continous Spaces Limit Reached");

                // RWS requires at least 1 space
                if (nsp < 1 && current == PARSE_RWS)
                    return ErrorOccured(HttpStatusCode::BAD_REQUEST, "Invalid Request");

                // Point to next section and set next task
                s.remove_prefix(nsp), saved = nullptr;
                // Prev not set for preserving previous main tasks
                current = prev == PARSE_METHOD ? PARSE_TARGET_URI : PARSE_PROTOCOL;
            }

            // Parse Http Target Uri
            else if (current == PARSE_TARGET_URI)
            {
                // Find the space character
                auto sp = s.find_first_of(" \t");
                if (sp == strv::npos)
                {
                    if (s.size() > HttpTargetUri::MAX_TARGET_URI_LENGTH)
                        return ErrorOccured(HttpStatusCode::URI_TOO_LONG, "URI Too Long");

                    saved = s.begin();
                    return ParseResult();
                }

                // If crossing max length
                if (sp > HttpTargetUri::MAX_TARGET_URI_LENGTH)
                    return ErrorOccured(HttpStatusCode::URI_TOO_LONG, "URI Too Long");

                temp = s.substr(0, sp);
                HttpTargetUri h = HttpTargetUri(&_uricontext);
                res = h.parse(temp);

                if (!res.success)
                {
                    current = NO_TASK;
                    return res;
                }

                // Point to next part and change task
                s.remove_prefix(sp), saved = nullptr;
                current = PARSE_RWS, prev = PARSE_TARGET_URI;
            }

            // Parse protocol
            else if (current == PARSE_PROTOCOL)
            {
                // Find the line ending
                auto le = s.find_first_of("\r\n");
                if (le == strv::npos)
                {
                    if (s.size() > HttpProtocol::FIXED_PROTOCOL_LENGTH)
                        return ErrorOccured(HttpStatusCode::BAD_REQUEST, "Invalid Protocol");

                    saved = s.begin();
                    return ParseResult();
                }

                if (le != HttpProtocol::FIXED_PROTOCOL_LENGTH)
                    return ErrorOccured(HttpStatusCode::BAD_REQUEST, "Invalid Protocol");

                temp = s.substr(0, le);
                HttpProtocol p = HttpProtocol(&_protVersion);
                res = p.parse(temp);

                if (!res.success)
                {
                    current = NO_TASK;
                    return res;
                }

                s.remove_prefix(le), saved = nullptr;
                current = PARSE_REQUEST_LINE_ENDING, prev = PARSE_PROTOCOL;
            }

            // Parse line endings
            else if (current == PARSE_REQUEST_LINE_ENDING || current == PARSE_HEADER_LINE_ENDING)
            {
                // if size less than 2
                if (s.size() < 2)
                {
                    saved = s.begin();
                    return ParseResult();
                }

                // Check if first two characters are \r\n
                if (s[0] != '\r' || s[1] != '\n')
                    return ErrorOccured(HttpStatusCode::BAD_REQUEST, "Invalid Request");

                // Move to next part
                s.remove_prefix(2), saved = nullptr;
                // Always, after getting line ending, will be header line parsing
                current = _hcontext.completed ? NO_TASK : PARSE_HEADER_LINE;
            }

            // Parse header line
            else if (current == PARSE_HEADER_LINE)
            {
                auto le = s.find("\r\n");
                if (le == strv::npos)
                {
                    if (s.size() + _hcontext.sizeInBytes > HttpHeaders::MAX_HEADERS_LENGTH)
                        return ErrorOccured(HttpStatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE, "Header Fields Too Large");

                    saved = s.begin();
                    return ParseResult();
                }

                if (le + _hcontext.sizeInBytes > HttpHeaders::MAX_HEADERS_LENGTH)
                    return ErrorOccured(HttpStatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE, "Header Fields Too Large");

                temp = s.substr(0, le);
                HttpHeaders h = HttpHeaders(&_hcontext);
                res = h.parse(temp);

                if (!res.success)
                {
                    current = NO_TASK;
                    return res;
                }

                // Check if headers are completed, then check header values to validate
                if (_hcontext.completed && !(res = validateHeaders()).success)
                {
                    current = NO_TASK;
                    return res;
                }

                // Point to next part
                s.remove_prefix(le), saved = nullptr;
                current = PARSE_HEADER_LINE_ENDING, prev = PARSE_HEADER_LINE;
            }

            // Invalid parsing task
            else
                return ErrorOccured(HttpStatusCode::INTERNAL_SERVER_ERROR, "Invalid Parsing Task");
        }

        return ParseResult();
    }

    // If the request in-memory changes location, update the location by giving the new s.begin()
    // This action can increase in complexity as more request is parsed, updating all the pointers
    // recreating the query and header map may increase processing time
    // It is adviced that a higher memory be used to contain the request so as this function is used
    // less frequently as possible
    void propagateMemoryChange(const char *newb, const char *prev)
    {
        ConditionalStr key, val;
        strv hkey, hval;

        // Propagate to saved string
        if (saved != nullptr)
            saved = HttpUtils::changeBeginning(saved, newb, prev).begin();

        // Propagate to path
        if (auto p = std::get_if<strv>(&_uricontext.path))
            (*p) = HttpUtils::changeBeginning(*p, newb, prev);

        // Propagate to query
        HttpTargetUri::Query newQuery;
        for (auto q : _uricontext.query)
        {
            // update key
            if (auto p = std::get_if<strv>(&q.first))
                key = HttpUtils::changeBeginning(*p, newb, prev);
            else
                key = q.first;

            // update val
            if (auto p = std::get_if<strv>(&q.second))
                val = HttpUtils::changeBeginning(*p, newb, prev);
            else
                val = q.second;

            // add in new query map
            newQuery[key] = val;
        }
        _uricontext.query = newQuery;

        // Propagate to headers
        HttpHeaders::Map newHeaderMap;
        for (auto h : _hcontext.mapped)
        {
            // update key
            hkey = HttpUtils::changeBeginning(h.first, newb, prev);

            // update val and push in new vector
            for (auto x : h.second)
            {
                hval = HttpUtils::changeBeginning(x, newb, prev);
                newHeaderMap[hkey].push_back(hval);
            }
        }
        _hcontext.mapped = newHeaderMap;
    }

    bool completed()
    {
        return current == NO_TASK;
    }

    auto method()
    {
        return _method;
    }

    auto path()
    {
        return _uricontext.path;
    }

    auto protocol()
    {
        str prot = "HTTP/";
        prot += (_protVersion.major + '0');
        prot += ".";
        prot += (_protVersion.minor + '0');
        return prot;
    }

    auto *headers()
    {
        return &_hcontext.mapped;
    }

    auto headers(strv s)
    {
        return _hcontext.mapped[s];
    }

    auto *query()
    {
        return &_uricontext.query;
    }

    auto query(strv s)
    {
        return _uricontext.query[s];
    }
};

#endif