#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include "../value-objects/request-methods.hpp"
#include "../value-objects/target-uri.hpp"
#include "../value-objects/http-protocol.hpp"
#include "../value-objects/http-headers.hpp"

#define PARSE_PROGRESS_METHOD 1
#define PARSE_PROGRESS_TARGET_URI 2
#define PARSE_PROGRESS_PROTOCOL 3
#define PARSE_PROGRESS_HEADERS 4

#define NO_REASON 0
#define INVALID_REQUEST_METHOD -1
#define REQUEST_METHOD_NOT_ALLOWED -2
#define INVALID_PROTOCOL -3
#define PROTOCOL_VERSION_NOT_SUPPORTED -4
#define BAD_REQUEST_LINE_FORMATTING -5
#define REQUEST_HEADER_FIELDS_TOO_LARGE -6
#define BAD_HEADER_FIELDS -7
#define PAYLOAD_FIELDS_PRESENT_IN_HEADER -8

// Each time the buffer, or the character pointer will be given
// to the request class, and the object will try to parse the
// given buffer till valid header/line/payload and then returns
// pointer/index till it has parsed.

// The class will throw error when it parses and detects that
// the parsed request has been invalid.
// It can be set to not throw error too, if infrastructure
// checks each time if the request has been made invalid after
// the last buffer recieved.

class HttpRequest
{

private:
    typedef std::unordered_map<int, bool> Progress;

    static constexpr size_t MAX_ALLOWED_HEADERS = 8192;

    static constexpr unsigned int FRONT_RESERVED_CONTAINER_SPACE = 256;

    // Property: Progress of parsing request
    Progress parsed{
        {PARSE_PROGRESS_METHOD, 0},
        {PARSE_PROGRESS_TARGET_URI, 0},
        {PARSE_PROGRESS_PROTOCOL, 0},
        {PARSE_PROGRESS_HEADERS, 0}};

    // Property: Total progress and validity
    int reason = NO_REASON;
    bool valid = false;
    bool complete = false;

    // Property: Allowed methods in http requests, currently
    static inline const std::unordered_set<HttpRequestMethod::Name> ALLOWED_METHODS = {
        HttpRequestMethod::GET, HttpRequestMethod::HEAD, HttpRequestMethod::DELETE};

    // Property: Allowed Http major and minor versions
    static constexpr int minHttpMajor = 1;
    static constexpr int maxHttpMajor = 1;
    static constexpr int minHttpMinor = 1;
    static constexpr int maxHttpMinor = 1;

    // Property: String to contain request
    str container;

    // Property: How much header has been parsed?
    unsigned int headerParsed = 0;

public:
    // Property: Request Method
    HttpRequestMethod method;

    // Property: Target URI
    HttpTargetUri targetUri;

    // Property: Protocol and its version (HTTP / 1.1,1.0)
    HttpProtocol protocol;

    // Property: Headers of the request
    HttpHeaders headers = HttpHeaders();

    // Method: Parse and add more to the request
    HttpRequest(strv s)
    {
        container.reserve(FRONT_RESERVED_CONTAINER_SPACE);

        parseMore(s);
    }

    void parseMore(strv s)
    {
        auto it = s.begin();

        // Parse method
        if (!complete && reason == NO_REASON && !parsed[PARSE_PROGRESS_METHOD])
        {
            // Till found a whitespace or till max length allowed
            while (it < s.end() && !isWS(*it) && container.size() < HttpRequestMethod::MAX_METHOD_NAME_LENGTH)
                container += *it++;

            // if not given in full
            if (it >= s.end())
                return;

            // else if is a whitespace or max length reached
            else if (isWS(*it) || container.size() >= HttpRequestMethod::MAX_METHOD_NAME_LENGTH)
            {
                method = HttpRequestMethod(container);

                // if invalid method
                if (method.name == HttpRequestMethod::INVALID)
                {
                    complete = true;
                    valid = false;
                    reason = INVALID_REQUEST_METHOD;
                    return;
                }

                // if not in allowed methods
                else if (ALLOWED_METHODS.count(method.name) == 0)
                {
                    complete = true;
                    valid = false;
                    reason = REQUEST_METHOD_NOT_ALLOWED;
                    return;
                }

                // in allowed method
                parsed[PARSE_PROGRESS_METHOD] = true;
                container.clear();
            }
        }

        // Parse target uri
        if (!complete && reason == NO_REASON && !parsed[PARSE_PROGRESS_TARGET_URI])
        {
            // clean whitespaces
            while (isWS(*it))
                it++;

            // till another whitespace appears or max length achieved
            while (it < s.end() && !isWS(*it) && container.size() < HttpTargetUri::MAX_TARGET_URI_LENGTH)
                container += *it++;

            // if reached end
            if (it >= s.end())
                return;

            // else if whitespace appeared or max length achieved
            if (isWS(*it) || container.size() >= HttpTargetUri::MAX_TARGET_URI_LENGTH)
            {
                targetUri = HttpTargetUri(container);

                parsed[PARSE_PROGRESS_TARGET_URI] = 1;
                container.clear();
            }
        }

        // if not parsed protocol, parse protocol
        if (!complete && reason == NO_REASON && !parsed[PARSE_PROGRESS_PROTOCOL])
        {
            // clean whitespaces
            while (isWS(*it))
                it++;

            // till reach the end of the request line or whitespace or max length of protocol
            while (it < s.end() && (it + 1 < s.end() && !isCRLF(*it, *(it + 1))) && container.size() < HttpProtocol::MAX_PROTOCOL_LENGTH)
                container += *it++;

            // not full
            if (it == s.end())
                return;

            // else if is crlf found
            else if (it + 1 < s.end() && isCRLF(*it, *(it + 1)))
            {
                protocol = HttpProtocol(container);

                // if protocol not http
                if (protocol.isInvalid())
                {
                    valid = false;
                    complete = true;
                    reason = INVALID_PROTOCOL;
                    return;
                }

                // else if bad protocol versions
                int major = protocol.majorVersion(), minor = protocol.minorVersion();
                if (major < minHttpMajor || major > maxHttpMajor || minor < minHttpMinor || minor > minHttpMinor)
                {
                    valid = false;
                    complete = true;
                    reason = PROTOCOL_VERSION_NOT_SUPPORTED;
                    return;
                }

                parsed[PARSE_PROGRESS_PROTOCOL] = 1;
                container.clear();
                it += 2; // skip crlf
            }

            // else if container size if max but no crlf
            else
            {
                valid = false;
                complete = true;
                reason = BAD_REQUEST_LINE_FORMATTING;
                return;
            }
        }

        // Parse headers now
        if (!complete && reason == NO_REASON && !parsed[PARSE_PROGRESS_HEADERS])
        {
            // take till found crlf or lf or max header line size
            while (it < s.end() && (it + 1 < s.end() && !isCRLF(*it, *(it + 1))) && container.size() < HttpHeaders::MAX_HEADER_FIELDS_LENGTH && headerParsed < MAX_ALLOWED_HEADERS)
                container += *it++, headerParsed++;

            // if headers are more than allowed
            if (headerParsed >= MAX_ALLOWED_HEADERS)
            {
                valid = false;
                complete = true;
                reason = REQUEST_HEADER_FIELDS_TOO_LARGE;
                return;
            }

            // if string ended
            if (it == s.end())
                return;

            // if container size is more or equal to max limits and no crlf found
            if (container.size() >= HttpHeaders::MAX_HEADER_FIELDS_LENGTH)
            {
                valid = false;
                complete = true;
                reason = REQUEST_HEADER_FIELDS_TOO_LARGE;
                return;
            }

            // if crlf found
            if (it + 1 < s.end() && isCRLF(*it, *(it + 1)))
            {
                bool success = headers.parseAnother(container);

                // if formatting not good
                if (container.size() != 0 && !success)
                {
                    valid = false;
                    complete = true;
                    reason = BAD_HEADER_FIELDS;
                    return;
                }

                container.clear();

                if (it + 3 < s.end() && is2xCRLF(*it, *(it + 1), *(it + 2), *(it + 3)))
                {
                    parsed[PARSE_PROGRESS_HEADERS] = 1;

                    // if bad headers are present
                    if (!headers.isValidByValue())
                    {
                        valid = false;
                        complete = true;
                        reason = BAD_HEADER_FIELDS;
                        return;
                    }

                    if (headers.contentLength() > 0 || headers.exists("transfer-encoding"))
                    {
                        valid = false;
                        complete = true;
                        reason = PAYLOAD_FIELDS_PRESENT_IN_HEADER;
                        return;
                    }

                    valid = true;
                    complete = true;

                    it += 4;
                }
                else
                    it += 2;
            }
        }
    }

    // Method: If the request is valid now ?
    bool isValid() const
    {
        return valid;
    }

    bool isCompleted() const
    {
        return complete;
    }

    std::pair<bool, int> status() const
    {
        return {valid, reason};
    }

    // Method: Is the request is invalid, as such any parameter is bad, ill-valued
    // not define properly as per the constraints set
};