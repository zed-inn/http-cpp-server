#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <variant>
#include "../interface/parser.hpp"
#include "../utils/http-header-keys.hpp"
#include "../utils/macros.hpp"

#ifndef _C_HTTP_HEADERS_PARSER_
#define _C_HTTP_HEADERS_PARSER_ 1

class HttpHeaders : Parser
{
public:
    // values
    typedef std::variant<std::vector<str>, str, bool, int, double, unsigned int> Value;

    // headers map
    typedef std::unordered_map<Key::Name, Value> HeaderMap;

    struct HeaderContext // headers context
    {
        size_t sizeInBytes;   // how much header is done
        HeaderMap mapped;     // key value pairs
        Key::Map unknownKeys; // keys that are not present in NamedKeys
        bool completed;

        HeaderContext() : sizeInBytes(0), completed(false) {}

    } typedef Context;

    static constexpr size_t MAX_HEADERS_LENGTH = 8192;

private:
    static constexpr size_t MAX_HEADER_KEY_LENGTH = 64;
    static constexpr size_t MAX_HEADER_VALUE_LENGTH = 8192;

    static constexpr size_t FRONT_RESERVED_KEY_SPACE = 16;
    static constexpr size_t FRONT_RESERVED_VALUE_SPACE = 32;

    static inline const std::unordered_set<unsigned char> ALLOWED_CHARS_IN_FIELD_KEY{
        '!', '#', '$', '%', '&', '\'', '*', '+', '-', '.', '^', '_', '`', '|', '~'};

    static const inline bool isValidKeyChar(unsigned int c)
    {
        return isalnum(c) || ALLOWED_CHARS_IN_FIELD_KEY.count(c);
    }

    static const inline bool isValidValueChar(unsigned int c)
    {
        return c == '\t' || (c >= 32 && c <= 126) || c > 128;
    }

    static const inline std::unordered_set<Key::Name> HEADER_CONTAINING_MULTIPLE_VALS_AND_Q_PARAMETER{Key::ACCEPT, Key::ACCEPT_ENCODING, Key::ACCEPT_LANGUAGE, Key::TE};

    static const inline std::unordered_set<Key::Name> HEADER_CONTAINING_MULTIPLE_VALS_VIA_COMMA{Key::ACCEPT, Key::ACCEPT_CH, Key::ACCEPT_ENCODING, Key::ACCEPT_LANGUAGE, Key::ACCEPT_PATCH, Key::ACCEPT_POST, Key::ACCEPT_RANGES, Key::ACCESS_CONTROL_ALLOW_HEADERS, Key::ACCESS_CONTROL_ALLOW_METHODS, Key::ACCESS_CONTROL_EXPOSE_HEADERS, Key::ACCESS_CONTROL_REQUEST_HEADERS, Key::AGE, Key::ALLOW, Key::ALT_SVC, Key::CACHE_CONTROL, Key::CONNECTION, Key::CONTENT_DIGEST, Key::CONTENT_ENCODING, Key::CONTENT_LANGUAGE, Key::CONTENT_SECURITY_POLICY, Key::CONTENT_SECURITY_POLICY_REPORT_ONLY, Key::EXPECT, Key::FORWARDED, Key::IF_MATCH, Key::IF_NONE_MATCH, Key::KEEP_ALIVE, Key::LINK, Key::PROXY_AUTHENTICATE, Key::RANGE, Key::REFERRER_POLICY, Key::REPORTING_ENDPOINTS, Key::REPR_DIGEST, Key::SEC_WEBSOCKET_EXTENSIONS, Key::SEC_WEBSOCKET_PROTOCOL, Key::SEC_WEBSOCKET_VERSION, Key::SERVER_TIMING, Key::SET_COOKIE, Key::SOURCEMAP, Key::TE, Key::TIMING_ALLOW_ORIGIN, Key::TRAILER, Key::TRANSFER_ENCODING, Key::UPGRADE, Key::VARY, Key::VIA, Key::WANT_CONTENT_DIGEST, Key::WANT_REPR_DIGEST, Key::WWW_AUTHENTICATE};

    static const inline std::unordered_set<Key::Name> HEADER_CONTAINING_SINGLE_VALUE{Key::ACCESS_CONTROL_ALLOW_CREDENTIALS, Key::ACCESS_CONTROL_ALLOW_ORIGIN, Key::ACCESS_CONTROL_MAX_AGE, Key::ACCESS_CONTROL_REQUEST_METHOD, Key::ACTIVATE_STORAGE_ACCESS, Key::ALT_USED, Key::AUTHORIZATION, Key::CLEAR_SITE_DATA, Key::CONTENT_DISPOSITION, Key::CONTENT_LENGTH, Key::CONTENT_LOCATION, Key::CONTENT_RANGE, Key::CONTENT_TYPE, Key::COOKIE, Key::CROSS_ORIGIN_EMBEDDER_POLICY, Key::CROSS_ORIGIN_EMBEDDER_POLICY_REPORT_ONLY, Key::CROSS_ORIGIN_OPENER_POLICY, Key::CROSS_ORIGIN_RESOURCE_POLICY, Key::DATE, Key::ETAG, Key::EXPIRES, Key::FROM, Key::HOST, Key::IF_MODIFIED_SINCE, Key::IF_RANGE, Key::IF_UNMODIFIED_SINCE, Key::INTEGRITY_POLICY, Key::INTEGRITY_POLICY_REPORT_ONLY, Key::LAST_MODIFIED, Key::LOCATION, Key::MAX_FORWARDS, Key::ORIGIN, Key::ORIGIN_AGENT_CLUSTER, Key::PREFER, Key::PREFERENCE_APPLIED, Key::PRIORITY, Key::PROXY_AUTHORIZATION, Key::REFERER, Key::REFRESH, Key::RETRY_AFTER, Key::SEC_FETCH_DEST, Key::SEC_FETCH_MODE, Key::SEC_FETCH_SITE, Key::SEC_FETCH_STORAGE_ACCESS, Key::SEC_FETCH_USER, Key::SEC_PURPOSE, Key::SEC_WEBSOCKET_ACCEPT, Key::SEC_WEBSOCKET_KEY, Key::SERVER, Key::SERVICE_WORKER, Key::SERVICE_WORKER_ALLOWED, Key::SERVICE_WORKER_NAVIGATION_PRELOAD, Key::SET_LOGIN, Key::STRICT_TRANSPORT_SECURITY, Key::UPGRADE_INSECURE_REQUESTS, Key::USER_AGENT, Key::X_CONTENT_TYPE_OPTIONS, Key::X_FRAME_OPTIONS};

    ParseResult parseAndSetValueForKey(Key::Name name, strv rawVal)
    {
        Value *temp;
        HeaderMap::iterator it;

        // Multiple values with q and separated by comma
        if (HEADER_CONTAINING_MULTIPLE_VALS_AND_Q_PARAMETER.count(name))
        {
            // TODO: check for each different Key/group of keys

            std::vector<str> commaSep;
            str container;
            for (auto i = rawVal.begin(); i < rawVal.end(); i++)
            {
                if (*i == ',')
                {
                    commaSep.push_back(container);
                    container.clear(), i++;
                    while (i < rawVal.end() && isHttpSpace(*i))
                        i++;
                }
                else
                    container += *i;
            }

            // TODO: Check if they contain q parameters
            // TODO: use a struct for value and q

            // exists
            it = _hc->mapped.find(name);
            if (it != _hc->mapped.end())
            {
                if (auto p = std::get_if<std::vector<str>>(&it->second))
                    (*p).insert((*p).end(), commaSep.begin(), commaSep.end());
                else
                    it->second = commaSep;
            }
            else
                _hc->mapped[name] = commaSep;
        }

        // Multiple values separated by comma
        else if (HEADER_CONTAINING_MULTIPLE_VALS_VIA_COMMA.count(name))
        {
            // TODO: check for each different Key/group of keys

            std::vector<str> commaSep;
            str container;
            container.reserve(10);
            for (auto i = rawVal.begin(); i < rawVal.end(); i++)
            {
                if (*i == ',')
                {
                    commaSep.push_back(container), container.clear(), i++;
                    while (i < rawVal.end() && isHttpSpace(*i))
                        i++;
                }
                else
                    container += *i;
            }

            if (container.length() > 0)
                commaSep.push_back(container), container.clear();

            // exists
            it = _hc->mapped.find(name);
            if (_hc->mapped.count(name))
            {
                if (auto p = std::get_if<std::vector<str>>(&it->second))
                    p->insert(p->end(), commaSep.begin(), commaSep.end());
                else
                    it->second = commaSep;
            }
            else
                _hc->mapped[name] = commaSep;

            // check if values are ok as specification defined for different keys
            temp = &_hc->mapped[name];
            switch (name)
            {
            case Key::TRANSFER_ENCODING:
                if (auto p = std::get_if<std::vector<str>>(temp))
                {
                    unsigned int chunkedCount, compressCount, deflateCount, gzipCount;
                    chunkedCount = compressCount = deflateCount = gzipCount = 0;
                    for (auto word : *p)
                    {
                        if (word.compare("chunked") == 0)
                            chunkedCount++;
                        else if (word.compare("compress") == 0)
                            compressCount++;
                        else if (word.compare("deflate") == 0)
                            deflateCount++;
                        else if (word.compare("gzip") == 0)
                            gzipCount++;
                        else
                            return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Transfer Encoding"));

                        // if repetitions
                        if (chunkedCount > 1 || compressCount > 1 || deflateCount > 1 || gzipCount > 1)
                            return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Transfer Encoding"));
                    }

                    // if no values
                    if (p->size() == 0)
                        return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "No Transfer Encodings"));
                }
                else
                    return ParseResult(DomainError(HttpStatusCode::INTERNAL_SERVER_ERROR, "Transfer Encoding Not Set"));
                break;

            default:
                break;
            }
        }

        // Contains only single value
        else if (HEADER_CONTAINING_SINGLE_VALUE.count(name))
        {
            // already exists
            it = _hc->mapped.find(name);
            if (it != _hc->mapped.end())
                return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Header Value"));

            // contains comma
            for (auto p = rawVal.begin(); p < rawVal.end(); p++)
                if (*p == ',' || *p == ' ')
                    return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Header Value"));

            // now specific to keys
            // TODO: each key if possible

            switch (name)
            {
            case Key::CONTENT_LENGTH:
            {
                unsigned int count = 0;
                for (auto p = rawVal.begin(); p < rawVal.end(); p++)
                {
                    if (!isdigit(*p))
                        return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Content Length"));
                    count = count * 10 + (*p - '0');
                }
                _hc->mapped[name] = count;
                break;
            }

            default:
                _hc->mapped[name] = str(rawVal);
                break;
            }
        }

        // unknown name
        else
        {
            // will be only one string value, but if 1+, then append string
            // exists
            it = _hc->mapped.find(name);
            if (it != _hc->mapped.end())
            {
                // append if string
                if (auto p = std::get_if<str>(&it->second))
                    *p += ", " + str(rawVal);
                else
                    it->second = str(rawVal);
            }
            else
                _hc->mapped[name] = str(rawVal);
        }

        return ParseResult();
    }

    Context *_hc;

public:
    HttpHeaders(Context *cp) : _hc(cp) {}

    // parse by header line
    ParseResult parse(strv s)
    {
        if (s.length() <= 0)
        {
            _hc->completed = true;
            return ParseResult();
        }

        if (_hc->sizeInBytes > MAX_HEADERS_LENGTH || _hc->sizeInBytes + s.length() > MAX_HEADERS_LENGTH)
            return ParseResult(DomainError(HttpStatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE, "Header Fields Too Large"));

        auto p = s.begin(), e = s.end();
        auto cp = p; // colon position

        // find colon
        while (cp < e && *cp != ':')
            cp++;
        if (cp >= e || *cp != ':')
            return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Header Line"));

        // skip bws from both sides
        while (isHttpSpace(*p))
            p++;
        while (isHttpSpace(*(e - 1)))
            e--;

        str key, val;
        key.reserve(FRONT_RESERVED_KEY_SPACE);
        val.reserve(FRONT_RESERVED_VALUE_SPACE);

        // get the key
        auto pe = cp;
        while (pe > p && isHttpSpace(*(pe - 1)))
            pe--;
        while (p < pe)
        {
            // add the lowered version of char
            if (isValidKeyChar(*p))
                key += tolower(*p++);
            else
                return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Header Key Character"));
        }

        // no key found
        if (key.length() == 0)
            return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "No Header Key Given"));

        // get the value
        auto es = cp + 1;
        while (es < e && isHttpSpace(*es))
            es++;
        while (es < e)
        {
            if (isValidValueChar(*es))
                val += *es++;
            else
                return ParseResult(DomainError(HttpStatusCode::BAD_REQUEST, "Invalid Header Value Character"));
        }

        // create the key name
        Key::Name name;
        if (Key::ToNamedKeys.find(key) != Key::ToNamedKeys.end())
            name = Key::ToNamedKeys.at(key);
        else
        {
            name = _hc->unknownKeys.size() + (Key::ToNamedKeys.size() + 1);
            _hc->unknownKeys[key] = name;
        }

        // parse and push the value for the key
        ParseResult res = parseAndSetValueForKey(name, val);
        if (!res.success)
        {
            _hc->completed = true;
            return res;
        }

        _hc->sizeInBytes += s.size() + 2; // \r\n included in header size
        return ParseResult();
    }
};

#endif