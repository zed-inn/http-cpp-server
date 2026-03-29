#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include "../value-objects/request-methods.hpp"
#include "../value-objects/target-uri.hpp"
#include "../value-objects/http-protocol.hpp"
#include "../value-objects/http-headers.hpp"
#include "../value-objects/http-status-codes.hpp"

#define PARSE_PROGRESS_METHOD 1
#define PARSE_PROGRESS_TARGET_URI 2
#define PARSE_PROGRESS_PROTOCOL 3
#define PARSE_PROGRESS_HEADERS 4

// Invalid Reason for a request
struct RequestInvalidationReason
{
    int statusCode;
    const char *reason;
} typedef rir;

// Invalidation Reason
class InValidationReason
{
public:
    static constexpr rir NO_REASON = {statusCode : HttpStatusCode::OK, reason : ""};
    static constexpr rir INVALID_REQUEST_METHOD = {statusCode : HttpStatusCode::METHOD_NOT_ALLOWED, reason : "Invalid Method"};
    static constexpr rir METHOD_NOT_ALLOWED = {statusCode : HttpStatusCode::METHOD_NOT_ALLOWED, reason : "Method Not Allowed"};
    static constexpr rir INVALID_PROTOCOL = {statusCode : HttpStatusCode::BAD_REQUEST, reason : "Invalid Protocol"};
    static constexpr rir PROTOCOL_VERSION_NOT_SUPPORTED = {statusCode : HttpStatusCode::HTTP_VERSION_NOT_SUPPORTED, reason : "Protocol Version Not Supported"};
    static constexpr rir BAD_REQUEST_LINE_FORMATTING = {statusCode : HttpStatusCode::BAD_REQUEST, reason : "Invalid Request"};
    static constexpr rir HEADER_FIELDS_TOO_LARGE = {statusCode : HttpStatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE, reason : "Header Fields Too Large"};
    static constexpr rir INVALID_HEADER_FIELDS = {statusCode : HttpStatusCode::BAD_REQUEST, reason : "Invalid Request"};
    static constexpr rir PAYLOAD_FIELDS_PRESENT_IN_HEADER = {statusCode : HttpStatusCode::BAD_REQUEST, reason : "Payload Not Supported Yet"};
} typedef IVReason;

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
    static constexpr size_t MAX_ALLOWED_HEADERS = 8192;

    static constexpr unsigned int FRONT_RESERVED_CONTAINER_SPACE = 256;

    // Property: Progress of parsing request
    struct ParseProgress
    {
        bool method;
        bool targetUri;
        bool protocol;
        bool headers;
    } parsed = {method : 0, targetUri : 0, protocol : 0, headers : 0};

    // Property: Total progress and validity
    rir reason = IVReason::NO_REASON; //  Reason for invalidity
    bool complete = false;            // If the request is not complete and not invalid

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

    // Property: Whitespace removal done before another parsing task
    bool wsRemoved = false;

    // Property: How much header has been parsed?
    unsigned int headerParsed = 0;

    // Property: Request Method
    HttpRequestMethod _method;

    // Property: Target URI
    HttpTargetUri _targetUri;

    // Property: Protocol and its version (HTTP / 1.1,1.0)
    HttpProtocol _protocol;

    // Property: Headers of the request
    HttpHeaders _headers;

public:
    HttpRequest()
    {
        container.reserve(FRONT_RESERVED_CONTAINER_SPACE);
        _headers = HttpHeaders();
    }

    HttpRequest(strv s, const char *it)
    {
        HttpRequest();
        parseMore(s, it);
    }

    // Method: Parse and add more to the request. Returns pointer till was parsed
    const char *parseMore(strv s, const char *it)
    {
        // Parse method
        if (!complete && !parsed.method)
        {
            // Till found a whitespace or till max length allowed
            while (it < s.end() && !isWS(*it) && container.size() < HttpRequestMethod::MAX_METHOD_NAME_LENGTH)
                container += *it++;

            // if is a whitespace or max length reached
            if (isWS(*it) || container.size() >= HttpRequestMethod::MAX_METHOD_NAME_LENGTH)
            {
                _method = HttpRequestMethod(container);

                // if invalid method
                if (_method.name == HttpRequestMethod::INVALID)
                {
                    complete = true;
                    reason = IVReason::INVALID_REQUEST_METHOD;
                    return it;
                }

                // if not in allowed methods
                else if (ALLOWED_METHODS.count(_method.name) == 0)
                {
                    complete = true;
                    reason = IVReason::METHOD_NOT_ALLOWED;
                    return it;
                }

                // in allowed method
                parsed.method = true;
                container.clear();
            }

            // length was short
            else if (it >= s.end())
                return it;
        }

        // Parse target uri
        if (!complete && !parsed.targetUri)
        {
            // clean whitespaces
            if (!wsRemoved)
                while (isWS(*it))
                    it++;
            wsRemoved = true;

            // till another whitespace appears or max length achieved
            while (it < s.end() && !isWS(*it) && container.size() < HttpTargetUri::MAX_TARGET_URI_LENGTH)
                container += *it++;

            // if whitespace appeared or max length achieved
            if (isWS(*it) || container.size() >= HttpTargetUri::MAX_TARGET_URI_LENGTH)
            {
                _targetUri = HttpTargetUri(container);

                parsed.targetUri = true;
                container.clear();
                wsRemoved = false; // set as not removed for the next parsing
            }

            // else if reached end
            else if (it >= s.end())
                return it;
        }

        // parse protocol
        if (!complete && !parsed.protocol)
        {
            // clean whitespaces
            if (!wsRemoved)
                while (isWS(*it))
                    it++;
            wsRemoved = true;

            // till reach the end of the request line or whitespace or max length of protocol
            int maxLengthWithEnding = HttpProtocol::MAX_PROTOCOL_LENGTH + 2;
            while (it < s.end() && container.length() < maxLengthWithEnding)
            {
                if ((container.length() >= 2 && isCRLF(*(container.end() - 2), *(container.end() - 1))))
                    break;
                container += *it++;
            }

            // if is crlf found at last
            if (container.length() >= 2 && isCRLF(*(container.end() - 2), *(container.end() - 1)))
            {
                strv tc(container);
                tc.remove_suffix(2);
                _protocol = HttpProtocol(tc);

                // if protocol not http
                if (_protocol.isInvalid())
                {
                    complete = true;
                    reason = IVReason::INVALID_PROTOCOL;
                    return it;
                }

                // else if bad protocol versions
                int major = _protocol.majorVersion(), minor = _protocol.minorVersion();
                if (major < minHttpMajor || major > maxHttpMajor || minor < minHttpMinor || minor > minHttpMinor)
                {
                    complete = true;
                    reason = IVReason::PROTOCOL_VERSION_NOT_SUPPORTED;
                    return it;
                }

                parsed.protocol = true;
                container.clear();
                wsRemoved = false; // for next it is not removed
            }

            // if not full
            else if (it >= s.end())
                return it;

            // else if container size is max but no crlf
            else
            {
                complete = true;
                reason = IVReason::BAD_REQUEST_LINE_FORMATTING;
                return it;
            }
        }

        // Parse headers
        if (!complete && !parsed.headers)
        {
            // take till found crlf/2xcrlf or max header line size
            int maxLengthWithEnding = HttpHeaders::MAX_HEADER_FIELDS_LENGTH + 2;
            bool parsingDone = false;

            // start parsing header lines
            while (!parsingDone)
            {
                // parse one header lines
                while (it < s.end() && container.size() < maxLengthWithEnding && headerParsed < MAX_ALLOWED_HEADERS)
                {
                    if (container.size() >= 2 && isCRLF(*(container.end() - 2), *(container.end() - 1)))
                        break;
                    container += *it++, headerParsed++;
                }

                // if headers are more than allowed
                if (headerParsed >= MAX_ALLOWED_HEADERS)
                {
                    complete = true;
                    reason = IVReason::HEADER_FIELDS_TOO_LARGE;
                    return it;
                }

                // if container size is more or equal to max limits and no crlf found
                if (container.size() >= maxLengthWithEnding)
                {
                    complete = true;
                    reason = IVReason::HEADER_FIELDS_TOO_LARGE;
                    return it;
                }

                // if crlf found
                if (container.size() >= 2 && isCRLF(*(container.end() - 2), *(container.end() - 1)))
                {
                    // an empty CRLF found, headers over
                    if (container.size() == 2)
                    {
                        parsed.headers = true;

                        // TODO: validate header function should be in request class itself
                        // if bad headers are present
                        if (!_headers.isValidByValue())
                        {
                            complete = true;
                            reason = IVReason::INVALID_HEADER_FIELDS;
                            return it;
                        }

                        // Only allowing and parsing with no payloads
                        if (_headers.contentLength() > 0 || _headers.exists("transfer-encoding"))
                        {
                            complete = true;
                            reason = IVReason::PAYLOAD_FIELDS_PRESENT_IN_HEADER;
                            return it;
                        }

                        // parsing complete
                        parsingDone = true;
                    }

                    // contains header line
                    else
                    {
                        strv tc(container);
                        tc.remove_suffix(2);

                        bool success = _headers.parseAnother(tc);

                        // if formatting not good
                        if (!success)
                        {
                            complete = true;
                            reason = IVReason::INVALID_HEADER_FIELDS;
                            return it;
                        }

                        container.clear();
                    }
                }

                // if string ended
                else if (it >= s.end())
                    return it;
            }
        }

        // request parsing complete
        complete = true;
        return it;
    }

    // Method: If the request is valid now ?
    bool valid() const
    {
        return reason.statusCode == IVReason::NO_REASON.statusCode;
    }

    // Method: If the request is completed/parsed now ?
    bool completed() const
    {
        return complete;
    }

    // Method: If the request is invalid, what's the reason? Returns OK as Status Code if no reason
    rir reasonForInvalid() const
    {
        return reason;
    }

    // Method: Get MethodName
    HttpRequestMethod::Name method() const
    {
        if (!valid())
            return HttpRequestMethod::INVALID;

        return _method.name;
    }

    // Method: Get Target Uri
    strv targetUri() const
    {
        if (!valid())
            return "";

        return _targetUri.path();
    }

    // Method: Get Query Parameters
    HttpTargetUri::QueryParameters *query()
    {
        return _targetUri.getQueryParams();
    }

    // Method:: Get protocol used
    str protocol() const
    {
        if (!valid())
            return "";

        str prot = "HTTP/";
        prot += _protocol.majorVersion() + '0';
        prot += '.';
        prot += _protocol.minorVersion() + '0';
        return prot;
    }

    // Method: Get Headers
    HttpHeaders::HeaderMap *headers()
    {
        return _headers.headerMap();
    }
};