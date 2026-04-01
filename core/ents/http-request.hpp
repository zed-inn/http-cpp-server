#include <queue>
#include "../interface/parser.hpp"
#include "../parsers/request-method.hpp"
#include "../parsers/request-target-uri.hpp"
#include "../parsers/request-protocol.hpp"
#include "../parsers/http-headers.hpp"

#ifndef _C_HTTP_REQUEST_PARSER_
#define _C_HTTP_REQUEST_PARSER_ 1

class HttpRequest : Parser
{
private:
    typedef unsigned char Task;
    typedef std::unordered_map<strv, HttpHeaders::Value> Headers;

    static constexpr unsigned int FRONT_RESERVED_CONTAINER_SPACE = 256;

    static constexpr unsigned short CONTINOUS_OBSP_ALLOWED = 32;

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

    static const inline std::unordered_set<HttpRequestMethod::Name> ALLOWED_METHODS{HttpRequestMethod::GET};

    struct TaskProgress
    {
    private:
        std::queue<Task> pending;

    public:
        void add(Task t) { pending.push(t); };
        Task current() { return pending.front(); }
        void doneCurrent() { pending.pop(); }
        bool over() { return pending.empty(); }

        void done()
        {
            while (!pending.empty())
                pending.pop();
        }
    } tasks;

    str container;

    // request validity
    bool valid = false;

    // Method name
    HttpRequestMethod::Name _method;

    // Target Path
    str _path;

    // Query Parameters
    HttpTargetUri::Query _query;

    // Protocol Version
    HttpProtocol::Version _protVersion;

    // Headers Context
    HttpHeaders::Context _hcontext;

    // Headers with strv-strv
    Headers _headers;

public:
    HttpRequest()
    {
        container.reserve(FRONT_RESERVED_CONTAINER_SPACE);

        // add tasks to do in order
        tasks.add(PARSE_METHOD);
        tasks.add(PARSE_RWS);
        tasks.add(PARSE_TARGET_URI);
        tasks.add(PARSE_RWS);
        tasks.add(PARSE_PROTOCOL);
        tasks.add(PARSE_REQUEST_LINE_ENDING);
        tasks.add(PARSE_HEADER_LINE);
        tasks.add(PARSE_HEADER_LINE_ENDING);
    }

    ParseResult parse(strv s)
    {
        if (tasks.over())
            return ParseResult(DomainError(HttpStatusCode::INTERNAL_SERVER_ERROR, "No Parsing Task Remain"));

        ParseResult res;
        auto p = s.begin();
        while (p < s.end() && !tasks.over())
        {
            switch (tasks.current())
            {
            case PARSE_METHOD:
                while (p < s.end() && container.size() < HttpRequestMethod::MAX_METHOD_LENGTH && !isHttpSpace(*p))
                    container += *p++;

                // if container reached limit or space encountered
                if (container.size() >= HttpRequestMethod::MAX_METHOD_LENGTH || isHttpSpace(*p))
                {
                    HttpRequestMethod m = HttpRequestMethod(&_method);
                    res = m.parse(container);
                    if (!res.success || _method == HttpRequestMethod::INVALID)
                    {
                        tasks.done();
                        return res;
                    }

                    // not in allowed methods
                    if (!ALLOWED_METHODS.count(_method))
                    {
                        tasks.done();
                        return ParseResult(DomainError(HttpStatusCode::METHOD_NOT_ALLOWED, "Method Not Allowed"));
                    }

                    container.clear();
                    tasks.doneCurrent();
                }
                // if p ended
                else if (p >= s.end())
                    return ParseResult();
                else
                {
                    tasks.done();
                    return ParseResult(DomainError(HttpStatusCode::INTERNAL_SERVER_ERROR, "Unexpected Error In Method Parsing"));
                }

                break;

            case PARSE_TARGET_URI:
                while (p < s.end() && container.size() < HttpTargetUri::MAX_TARGET_URI_LENGTH && !isHttpSpace(*p))
                    container += *p++;

                // container reached limit or space encountered
                if (container.size() >= HttpTargetUri::MAX_TARGET_URI_LENGTH || isHttpSpace(*p))
                {
                    bool absform;
                    HttpTargetUri t = HttpTargetUri(&_path, &_query, &absform);
                    res = t.parse(container);
                    if (!res.success)
                    {
                        tasks.done();
                        return res;
                    }

                    container.clear();
                    tasks.doneCurrent();
                }
                // if p ended
                else if (p >= s.end())
                    return ParseResult();
                else
                {
                    tasks.done();
                    return ParseResult(DomainError(HttpStatusCode::INTERNAL_SERVER_ERROR, "Unexpected Error In Target Uri Parsing"));
                }

                break;

            case PARSE_PROTOCOL:
                while (p < s.end() && container.size() != HttpProtocol::FIXED_PROTOCOL_LENGTH)
                    container += *p++;

                // container reached the protocol length
                if (container.size() == HttpProtocol::FIXED_PROTOCOL_LENGTH)
                {
                    HttpProtocol p = HttpProtocol(&_protVersion);
                    res = p.parse(container);
                    if (!res.success)
                    {
                        tasks.done();
                        return res;
                    }

                    // only 1.1 is allowed
                    if (_protVersion.major != 1 || _protVersion.minor != 1)
                    {
                        tasks.done();
                        return ParseResult(DomainError(HttpStatusCode::HTTP_VERSION_NOT_SUPPORTED, "Http Version Not Supported"));
                    }

                    container.clear();
                    tasks.doneCurrent();
                }
                // if p ended
                else if (p >= s.end())
                    return ParseResult();
                else
                {
                    tasks.done();
                    return ParseResult(DomainError(HttpStatusCode::INTERNAL_SERVER_ERROR, "Unexpected Error In Protocol Parsing"));
                }

                break;

            case PARSE_HEADER_LINE:
                while (p < s.end() && container.size() < HttpHeaders::MAX_HEADERS_LENGTH && !isCR(*p))
                    container += *p++;

                // container reached limit or \r encountered
                if (container.size() >= HttpHeaders::MAX_HEADERS_LENGTH || isCR(*p))
                {
                    HttpHeaders h = HttpHeaders(&_hcontext);
                    res = h.parse(container);
                    if (!res.success)
                    {
                        tasks.done();
                        return res;
                    }

                    container.clear();
                    tasks.doneCurrent();

                    // headers not done, then add another task to parse next header line
                    if (!_hcontext.completed)
                    {
                        tasks.add(PARSE_HEADER_LINE);
                        tasks.add(PARSE_HEADER_LINE_ENDING);
                    }
                    // if completed, check content-length/transfer-encoding to see if payload has to be parsed
                    else
                    {
                        // convert context for headers of strv-strv
                        for (auto x : _hcontext.mapped)
                        {
                            // check with named keys
                            strv rawKey = HttpHeaderKey::RawValueByNamedKey(x.first);
                            if (rawKey.size() == 0)
                            {
                                // check with unknown keys

                                auto it = _hcontext.unknownKeysRev.find(x.first);
                                if (it == _hcontext.unknownKeysRev.end())
                                {
                                    tasks.done();
                                    return ParseResult(DomainError(HttpStatusCode::INTERNAL_SERVER_ERROR, "Header Parsing Invalid"));
                                }

                                rawKey = strv(it->second->first);
                            }
                            _headers[rawKey] = x.second;
                        }

                        // TODO: validate named headers if remaining

                        // currently not supporting any payload
                        // if content length and greater than 0 or transfer encoding given
                        auto it = _hcontext.mapped.find(HttpHeaderKey::CONTENT_LENGTH);
                        if (it != _hcontext.mapped.end())
                            if (auto p = std::get_if<unsigned int>(&it->second))
                                if (*p > 0)
                                    return ParseResult(DomainError(HttpStatusCode::NOT_IMPLEMENTED, "Payload Unsupported"));

                        it = _hcontext.mapped.find(HttpHeaderKey::TRANSFER_ENCODING);
                        if (it != _hcontext.mapped.end())
                            return ParseResult(DomainError(HttpStatusCode::NOT_IMPLEMENTED, "Payload Unsupported"));
                    }
                }
                // if p ended
                else if (p >= s.end())
                    return ParseResult();
                else
                {
                    tasks.done();
                    return ParseResult(DomainError(HttpStatusCode::INTERNAL_SERVER_ERROR, "Unexpected Error In Header Line Parsing"));
                }

                break;

            case PARSE_REQUEST_LINE_ENDING:
            case PARSE_HEADER_LINE_ENDING:
                while (p < s.end() && container.size() != 2)
                    container += *p++;

                // container limit reached
                if (container.size() == 2)
                {
                    if (!isCRLF(container[0], container[1]))
                    {
                        tasks.done();
                        return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Request"));
                    }

                    container.clear();
                    tasks.doneCurrent();
                }
                else if (container.size() > 2)
                {
                    tasks.done();
                    return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Request"));
                }
                // if p ended
                else if (p >= s.end())
                    return ParseResult();
                else
                {
                    tasks.done();
                    return ParseResult(DomainError(HttpStatusCode::INTERNAL_SERVER_ERROR, "Unexpected Error In Request-line/Header-line Ending Parsing"));
                }

                break;

            case PARSE_OWS:
            case PARSE_BWS:
            case PARSE_RWS:
                while (p < s.end() && container.size() < CONTINOUS_OBSP_ALLOWED && isHttpSpace(*p))
                    container += *p++;
                ;

                // if p ended
                if (p >= s.end())
                    return ParseResult();

                // container reached max limit
                if (container.size() <= CONTINOUS_OBSP_ALLOWED)
                {
                    // if no space
                    if (tasks.current() == PARSE_RWS && container.size() < 1)
                    {
                        tasks.done();
                        return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Request"));
                    }

                    // if limit reached and still the next char is a space
                    else if (isHttpSpace(*p))
                    {
                        tasks.done();
                        return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Continous Spaces Limit Reached"));
                    }

                    container.clear();
                    tasks.doneCurrent();
                }
                else if (container.size() > CONTINOUS_OBSP_ALLOWED)
                {
                    tasks.done();
                    return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Continous Spaces Limit Reached"));
                }
                else
                {
                    tasks.done();
                    return ParseResult(DomainError(HttpStatusCode::INTERNAL_SERVER_ERROR, "Unexpected Error In Request-line/Header-line Ending Parsing"));
                }

                break;

            default:
                tasks.done();
                return ParseResult(DomainError(HttpStatusCode::INTERNAL_SERVER_ERROR, "Invalid Task"));
            }
        }

        if (tasks.over())
            valid = true;

        return ParseResult();
    }

    bool completed()
    {
        return tasks.over();
    }

    HttpRequestMethod::Name method()
    {
        if (!valid)
            return HttpRequestMethod::INVALID;
        return _method;
    }

    str path()
    {
        if (!valid)
            return "";
        return _path;
    }

    str protocol()
    {
        if (!valid)
            return "";
        str prot = "HTTP/";
        prot += (_protVersion.major + '0');
        prot += ".";
        prot += (_protVersion.minor + '0');
        return prot;
    }

    Headers *headers()
    {
        if (!valid)
            return nullptr;

        return &_headers;
    }

    str query(strv key)
    {
        if (!valid)
            return "";

        return _query[str(key)];
    }

    HttpTargetUri::Query *query()
    {
        if (!valid)
            return nullptr;

        return &_query;
    }
};

#endif