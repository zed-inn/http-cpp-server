#ifndef _C_HTTP_STATUS_CODE_IANA_REGISTERED_
#define _C_HTTP_STATUS_CODE_IANA_REGISTERED_ 1

class HttpStatusCode
{
public:
    typedef unsigned short IANAStatusCode;

    static constexpr IANAStatusCode CONTINUE = 100;
    static constexpr IANAStatusCode SWITCHING_PROTOCOLS = 101;
    static constexpr IANAStatusCode PROCESSING = 102;
    static constexpr IANAStatusCode EARLY_HINTS = 103;
    static constexpr IANAStatusCode UPLOAD_RESUMPTION_SUPPORTED = 104;
    static constexpr IANAStatusCode OK = 200;
    static constexpr IANAStatusCode CREATED = 201;
    static constexpr IANAStatusCode ACCEPTED = 202;
    static constexpr IANAStatusCode NON_AUTHORITATIVE_INFORMATION = 203;
    static constexpr IANAStatusCode NO_CONTENT = 204;
    static constexpr IANAStatusCode RESET_CONTENT = 205;
    static constexpr IANAStatusCode PARTIAL_CONTENT = 206;
    static constexpr IANAStatusCode MULTI_STATUS = 207;
    static constexpr IANAStatusCode ALREADY_REPORTED = 208;
    static constexpr IANAStatusCode IM_USED = 226;
    static constexpr IANAStatusCode MULTIPLE_CHOICES = 300;
    static constexpr IANAStatusCode MOVED_PERMANENTLY = 301;
    static constexpr IANAStatusCode FOUND = 302;
    static constexpr IANAStatusCode SEE_OTHER = 303;
    static constexpr IANAStatusCode NOT_MODIFIED = 304;
    static constexpr IANAStatusCode USE_PROXY = 305;
    static constexpr IANAStatusCode UNUSED_306 = 306;
    static constexpr IANAStatusCode TEMPORARY_REDIRECT = 307;
    static constexpr IANAStatusCode PERMANENT_REDIRECT = 308;
    static constexpr IANAStatusCode BAD_REQUEST = 400;
    static constexpr IANAStatusCode UNAUTHORIZED = 401;
    static constexpr IANAStatusCode PAYMENT_REQUIRED = 402;
    static constexpr IANAStatusCode FORBIDDEN = 403;
    static constexpr IANAStatusCode NOT_FOUND = 404;
    static constexpr IANAStatusCode METHOD_NOT_ALLOWED = 405;
    static constexpr IANAStatusCode NOT_ACCEPTABLE = 406;
    static constexpr IANAStatusCode PROXY_AUTHENTICATION_REQUIRED = 407;
    static constexpr IANAStatusCode REQUEST_TIMEOUT = 408;
    static constexpr IANAStatusCode CONFLICT = 409;
    static constexpr IANAStatusCode GONE = 410;
    static constexpr IANAStatusCode LENGTH_REQUIRED = 411;
    static constexpr IANAStatusCode PRECONDITION_FAILED = 412;
    static constexpr IANAStatusCode PAYLOAD_TOO_LARGE = 413;
    static constexpr IANAStatusCode URI_TOO_LONG = 414;
    static constexpr IANAStatusCode UNSUPPORTED_MEDIA_TYPE = 415;
    static constexpr IANAStatusCode RANGE_NOT_SATISFIABLE = 416;
    static constexpr IANAStatusCode EXPECTATION_FAILED = 417;
    static constexpr IANAStatusCode UNUSED_418 = 418;
    static constexpr IANAStatusCode MISDIRECTED_REQUEST = 421;
    static constexpr IANAStatusCode UNPROCESSABLE_CONTENT = 422;
    static constexpr IANAStatusCode LOCKED = 423;
    static constexpr IANAStatusCode FAILED_DEPENDENCY = 424;
    static constexpr IANAStatusCode TOO_EARLY = 425;
    static constexpr IANAStatusCode UPGRADE_REQUIRED = 426;
    static constexpr IANAStatusCode PRECONDITION_REQUIRED = 428;
    static constexpr IANAStatusCode TOO_MANY_REQUESTS = 429;
    static constexpr IANAStatusCode REQUEST_HEADER_FIELDS_TOO_LARGE = 431;
    static constexpr IANAStatusCode UNAVAILABLE_FOR_LEGAL_REASONS = 451;
    static constexpr IANAStatusCode INTERNAL_SERVER_ERROR = 500;
    static constexpr IANAStatusCode NOT_IMPLEMENTED = 501;
    static constexpr IANAStatusCode BAD_GATEWAY = 502;
    static constexpr IANAStatusCode SERVICE_UNAVAILABLE = 503;
    static constexpr IANAStatusCode GATEWAY_TIMEOUT = 504;
    static constexpr IANAStatusCode HTTP_VERSION_NOT_SUPPORTED = 505;
    static constexpr IANAStatusCode VARIANT_ALSO_NEGOTIATES = 506;
    static constexpr IANAStatusCode INSUFFICIENT_STORAGE = 507;
    static constexpr IANAStatusCode LOOP_DETECTED = 508;
    static constexpr IANAStatusCode NOT_EXTENDED = 510;
    static constexpr IANAStatusCode NETWORK_AUTHENTICATION_REQUIRED = 511;
};

#endif