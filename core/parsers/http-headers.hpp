#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../interface/parser.hpp"
#include "../utils/http-header-keys.hpp"
#include "../utils/http-utils.hpp"

#ifndef _C_HTTP_HEADERS_PARSER_
#define _C_HTTP_HEADERS_PARSER_ 1

class HttpHeaders : Parser
{
public:
    // headers map
    typedef std::unordered_map<strv, std::vector<strv>> Map;

    struct HeaderContext // headers context
    {
        size_t sizeInBytes; // how much header is done
        bool completed;     // if parsing of headers is over
        Map mapped;         // key value pairs

        HeaderContext() : sizeInBytes(0), completed(false) {}
    } typedef Context;

    static constexpr size_t MAX_HEADERS_LENGTH = 8192;

private:
    static constexpr size_t MAX_HEADER_KEY_LENGTH = 64;
    static constexpr size_t MAX_HEADER_VALUE_LENGTH = 8192;

    static constexpr std::array<bool, 256> ALLOWED_KEY_TABLE = []
    {
        std::array<bool, 256> t{false};
        const unsigned char allowed[] = {'!', '#', '$', '%', '&', '\'', '*', '+', '-', '.', '^', '_', '`', '|', '~'};
        for (unsigned char c : allowed)
            t[c] = true;
        for (unsigned char d = '0'; d <= '9'; ++d)
            t[d] = true;
        for (unsigned char u = 'A'; u <= 'Z'; ++u)
            t[u] = true;
        for (unsigned char l = 'a'; l <= 'z'; ++l)
            t[l] = true;
        return t;
    }();

    static constexpr std::array<bool, 256> ALLOWED_VALUE_TABLE = []
    {
        std::array<bool, 256> t{false};
        t['\t'] = true;
        for (unsigned char c = 32; c <= 126; ++c)
            t[c] = true;
        return t;
    }();

    static const inline std::unordered_set<Key::Name> HEADER_CONTAINING_MULTIPLE_VALS_AND_Q_PARAMETER{
        Key::ACCEPT, Key::ACCEPT_ENCODING, Key::ACCEPT_LANGUAGE, Key::TE};

    static const inline std::unordered_set<Key::Name> HEADER_CONTAINING_MULTIPLE_VALS_VIA_COMMA{
        Key::ACCEPT, Key::ACCEPT_CH, Key::ACCEPT_ENCODING, Key::ACCEPT_LANGUAGE, Key::ACCEPT_PATCH, Key::ACCEPT_POST, Key::ACCEPT_RANGES, Key::ACCESS_CONTROL_ALLOW_HEADERS, Key::ACCESS_CONTROL_ALLOW_METHODS, Key::ACCESS_CONTROL_EXPOSE_HEADERS, Key::ACCESS_CONTROL_REQUEST_HEADERS, Key::AGE, Key::ALLOW, Key::ALT_SVC, Key::CACHE_CONTROL, Key::CONNECTION, Key::CONTENT_DIGEST, Key::CONTENT_ENCODING, Key::CONTENT_LANGUAGE, Key::CONTENT_SECURITY_POLICY, Key::CONTENT_SECURITY_POLICY_REPORT_ONLY, Key::EXPECT, Key::FORWARDED, Key::IF_MATCH, Key::IF_NONE_MATCH, Key::KEEP_ALIVE, Key::LINK, Key::PROXY_AUTHENTICATE, Key::RANGE, Key::REFERRER_POLICY, Key::REPORTING_ENDPOINTS, Key::REPR_DIGEST, Key::SEC_WEBSOCKET_EXTENSIONS, Key::SEC_WEBSOCKET_PROTOCOL, Key::SEC_WEBSOCKET_VERSION, Key::SERVER_TIMING, Key::SET_COOKIE, Key::SOURCEMAP, Key::TE, Key::TIMING_ALLOW_ORIGIN, Key::TRAILER, Key::TRANSFER_ENCODING, Key::UPGRADE, Key::VARY, Key::VIA, Key::WANT_CONTENT_DIGEST, Key::WANT_REPR_DIGEST, Key::WWW_AUTHENTICATE};

    static const inline std::unordered_set<Key::Name> HEADER_CONTAINING_SINGLE_VALUE{
        Key::ACCESS_CONTROL_ALLOW_CREDENTIALS, Key::ACCESS_CONTROL_ALLOW_ORIGIN, Key::ACCESS_CONTROL_MAX_AGE, Key::ACCESS_CONTROL_REQUEST_METHOD, Key::ACTIVATE_STORAGE_ACCESS, Key::ALT_USED, Key::AUTHORIZATION, Key::CLEAR_SITE_DATA, Key::CONTENT_DISPOSITION, Key::CONTENT_LENGTH, Key::CONTENT_LOCATION, Key::CONTENT_RANGE, Key::CONTENT_TYPE, Key::COOKIE, Key::CROSS_ORIGIN_EMBEDDER_POLICY, Key::CROSS_ORIGIN_EMBEDDER_POLICY_REPORT_ONLY, Key::CROSS_ORIGIN_OPENER_POLICY, Key::CROSS_ORIGIN_RESOURCE_POLICY, Key::DATE, Key::ETAG, Key::EXPIRES, Key::FROM, Key::HOST, Key::IF_MODIFIED_SINCE, Key::IF_RANGE, Key::IF_UNMODIFIED_SINCE, Key::INTEGRITY_POLICY, Key::INTEGRITY_POLICY_REPORT_ONLY, Key::LAST_MODIFIED, Key::LOCATION, Key::MAX_FORWARDS, Key::ORIGIN, Key::ORIGIN_AGENT_CLUSTER, Key::PREFER, Key::PREFERENCE_APPLIED, Key::PRIORITY, Key::PROXY_AUTHORIZATION, Key::REFERER, Key::REFRESH, Key::RETRY_AFTER, Key::SEC_FETCH_DEST, Key::SEC_FETCH_MODE, Key::SEC_FETCH_SITE, Key::SEC_FETCH_STORAGE_ACCESS, Key::SEC_FETCH_USER, Key::SEC_PURPOSE, Key::SEC_WEBSOCKET_ACCEPT, Key::SEC_WEBSOCKET_KEY, Key::SERVER, Key::SERVICE_WORKER, Key::SERVICE_WORKER_ALLOWED, Key::SERVICE_WORKER_NAVIGATION_PRELOAD, Key::SET_LOGIN, Key::STRICT_TRANSPORT_SECURITY, Key::UPGRADE_INSECURE_REQUESTS, Key::USER_AGENT, Key::X_CONTENT_TYPE_OPTIONS, Key::X_FRAME_OPTIONS};

    ParseResult validateAndSetValueForNamedKeys(Key::Name name, strv key, strv val)
    {
        // Contains comma separated values
        if (HEADER_CONTAINING_MULTIPLE_VALS_VIA_COMMA.count(name))
        {
            size_t pos;
            strv temp;

            // split on comma and append
            while ((pos = val.find(",")) != strv::npos)
            {
                // Get till comma
                temp = HttpUtils::trimHttpSpaces(val.substr(0, pos));
                if (temp.size() > 0)
                    _hc->mapped[key].push_back(temp);

                // Assign val after comma
                val = HttpUtils::trimHttpSpaces(val.substr(pos + 1));
            }

            // if any value left after last comma
            val = HttpUtils::trimHttpSpaces(val);
            if (val.size() > 0)
                _hc->mapped[key].push_back(val);
        }

        // Only single values are supported
        else if (HEADER_CONTAINING_SINGLE_VALUE.count(name))
        {
            // check if the value appears only once
            auto it = _hc->mapped.find(key);
            if (it != _hc->mapped.end())
                return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Header Value");

            // Checks for specific headers
            switch (name)
            {
            case Key::CONTENT_LENGTH:
                if (val.size() <= 0) // Should have '0' if field given
                    return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Header Value");

                for (auto x : val) // All char must be digits
                    if (!isdigit(x))
                        return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Header Value");
                break;
            }

            _hc->mapped[key].push_back(val);
        }

        // Would necessarily never happen, but if it does, will reduce data integrity issues
        else
            _hc->mapped[key].push_back(val);

        return ParseResult();
    }

    Context *_hc;

public:
    HttpHeaders(Context *cp) : _hc(cp) {}

    // parse by header line
    ParseResult parse(strv s)
    {
        if (_hc->completed)
            return ParseResult(HttpStatusCode::INTERNAL_SERVER_ERROR, "Header Fields Parsing Done");

        if (s.length() <= 0)
        {
            _hc->completed = true;
            return ParseResult();
        }

        if (_hc->sizeInBytes > MAX_HEADERS_LENGTH || _hc->sizeInBytes + s.length() + 2 > MAX_HEADERS_LENGTH)
            return ParseResult(HttpStatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE, "Header Fields Too Large");

        // Find Colon Position
        auto cp = s.find(":");
        if (cp == strv::npos)
            return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Header Line");

        // Trim the Key
        auto key = HttpUtils::trimHttpSpaces(s.substr(0, cp));
        if (key.size() <= 0)
            return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Header Key");

        // Check for max length allowable for header key
        if (key.size() > MAX_HEADER_KEY_LENGTH)
            return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Header Key");

        // Check for header key allowable characters
        for (auto x : key)
            if (!ALLOWED_KEY_TABLE[x])
                return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Header Key");

        // Trim the value
        auto value = HttpUtils::trimHttpSpaces(s.substr(cp + 1));

        // Check for max length allowable for header value
        if (value.size() > MAX_HEADER_VALUE_LENGTH)
            return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Header Value");

        // Check for header value allowable characters
        for (auto x : value)
            if (!ALLOWED_VALUE_TABLE[x])
                return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Header Value");

        // If named key
        auto it = Key::NamedKeys.find(tolower(key));
        if (it != Key::NamedKeys.end())
        {
            auto res = validateAndSetValueForNamedKeys(it->second, key, value);
            if (!res.success)
                return res;
        }
        // Not a named key
        else
            _hc->mapped[key].push_back(value);

        _hc->sizeInBytes += s.size() + 2; // \r\n included in header size
        return ParseResult();
    }
};

#endif