#include <unordered_map>
#include "./strs.hpp"

#ifndef _C_HTTP_HEADER_KEYS_
#define _C_HTTP_HEADER_KEYS_ 1

class HttpHeaderKey
{
public:
    // key and key map
    typedef unsigned short Name;
    typedef std::unordered_map<str, Name> Map;

    // mapped header keys
    static constexpr Name ACCEPT = 1;
    static constexpr Name ACCEPT_CH = 2;
    static constexpr Name ACCEPT_ENCODING = 3;
    static constexpr Name ACCEPT_LANGUAGE = 4;
    static constexpr Name ACCEPT_PATCH = 5;
    static constexpr Name ACCEPT_POST = 6;
    static constexpr Name ACCEPT_RANGES = 7;
    static constexpr Name ACCESS_CONTROL_ALLOW_CREDENTIALS = 8;
    static constexpr Name ACCESS_CONTROL_ALLOW_HEADERS = 9;
    static constexpr Name ACCESS_CONTROL_ALLOW_METHODS = 10;
    static constexpr Name ACCESS_CONTROL_ALLOW_ORIGIN = 11;
    static constexpr Name ACCESS_CONTROL_EXPOSE_HEADERS = 12;
    static constexpr Name ACCESS_CONTROL_MAX_AGE = 13;
    static constexpr Name ACCESS_CONTROL_REQUEST_HEADERS = 14;
    static constexpr Name ACCESS_CONTROL_REQUEST_METHOD = 15;
    static constexpr Name ACTIVATE_STORAGE_ACCESS = 16;
    static constexpr Name AGE = 17;
    static constexpr Name ALLOW = 18;
    static constexpr Name ALT_SVC = 19;
    static constexpr Name ALT_USED = 20;
    static constexpr Name AUTHORIZATION = 21;
    static constexpr Name CACHE_CONTROL = 22;
    static constexpr Name CLEAR_SITE_DATA = 23;
    static constexpr Name CONNECTION = 24;
    static constexpr Name CONTENT_DIGEST = 25;
    static constexpr Name CONTENT_DISPOSITION = 26;
    static constexpr Name CONTENT_ENCODING = 27;
    static constexpr Name CONTENT_LANGUAGE = 28;
    static constexpr Name CONTENT_LENGTH = 29;
    static constexpr Name CONTENT_LOCATION = 30;
    static constexpr Name CONTENT_RANGE = 31;
    static constexpr Name CONTENT_SECURITY_POLICY = 32;
    static constexpr Name CONTENT_SECURITY_POLICY_REPORT_ONLY = 33;
    static constexpr Name CONTENT_TYPE = 34;
    static constexpr Name COOKIE = 35;
    static constexpr Name CROSS_ORIGIN_EMBEDDER_POLICY = 36;
    static constexpr Name CROSS_ORIGIN_EMBEDDER_POLICY_REPORT_ONLY = 37;
    static constexpr Name CROSS_ORIGIN_OPENER_POLICY = 38;
    static constexpr Name CROSS_ORIGIN_RESOURCE_POLICY = 39;
    static constexpr Name DATE = 40;
    static constexpr Name ETAG = 41;
    static constexpr Name EXPECT = 42;
    static constexpr Name EXPIRES = 43;
    static constexpr Name FORWARDED = 44;
    static constexpr Name FROM = 45;
    static constexpr Name HOST = 46;
    static constexpr Name IF_MATCH = 47;
    static constexpr Name IF_MODIFIED_SINCE = 48;
    static constexpr Name IF_NONE_MATCH = 49;
    static constexpr Name IF_RANGE = 50;
    static constexpr Name IF_UNMODIFIED_SINCE = 51;
    static constexpr Name INTEGRITY_POLICY = 52;
    static constexpr Name INTEGRITY_POLICY_REPORT_ONLY = 53;
    static constexpr Name KEEP_ALIVE = 54;
    static constexpr Name LAST_MODIFIED = 55;
    static constexpr Name LINK = 56;
    static constexpr Name LOCATION = 57;
    static constexpr Name MAX_FORWARDS = 58;
    static constexpr Name ORIGIN = 59;
    static constexpr Name ORIGIN_AGENT_CLUSTER = 60;
    static constexpr Name PREFER = 61;
    static constexpr Name PREFERENCE_APPLIED = 62;
    static constexpr Name PRIORITY = 63;
    static constexpr Name PROXY_AUTHENTICATE = 64;
    static constexpr Name PROXY_AUTHORIZATION = 65;
    static constexpr Name RANGE = 66;
    static constexpr Name REFERER = 67;
    static constexpr Name REFERRER_POLICY = 68;
    static constexpr Name REFRESH = 69;
    static constexpr Name REPORTING_ENDPOINTS = 70;
    static constexpr Name REPR_DIGEST = 71;
    static constexpr Name RETRY_AFTER = 72;
    static constexpr Name SEC_FETCH_DEST = 73;
    static constexpr Name SEC_FETCH_MODE = 74;
    static constexpr Name SEC_FETCH_SITE = 75;
    static constexpr Name SEC_FETCH_STORAGE_ACCESS = 76;
    static constexpr Name SEC_FETCH_USER = 77;
    static constexpr Name SEC_PURPOSE = 78;
    static constexpr Name SEC_WEBSOCKET_ACCEPT = 79;
    static constexpr Name SEC_WEBSOCKET_EXTENSIONS = 80;
    static constexpr Name SEC_WEBSOCKET_KEY = 81;
    static constexpr Name SEC_WEBSOCKET_PROTOCOL = 82;
    static constexpr Name SEC_WEBSOCKET_VERSION = 83;
    static constexpr Name SERVER = 84;
    static constexpr Name SERVER_TIMING = 85;
    static constexpr Name SERVICE_WORKER = 86;
    static constexpr Name SERVICE_WORKER_ALLOWED = 87;
    static constexpr Name SERVICE_WORKER_NAVIGATION_PRELOAD = 88;
    static constexpr Name SET_COOKIE = 89;
    static constexpr Name SET_LOGIN = 90;
    static constexpr Name SOURCEMAP = 91;
    static constexpr Name STRICT_TRANSPORT_SECURITY = 92;
    static constexpr Name TE = 93;
    static constexpr Name TIMING_ALLOW_ORIGIN = 94;
    static constexpr Name TRAILER = 95;
    static constexpr Name TRANSFER_ENCODING = 96;
    static constexpr Name UPGRADE = 97;
    static constexpr Name UPGRADE_INSECURE_REQUESTS = 98;
    static constexpr Name USER_AGENT = 99;
    static constexpr Name VARY = 100;
    static constexpr Name VIA = 101;
    static constexpr Name WANT_CONTENT_DIGEST = 102;
    static constexpr Name WANT_REPR_DIGEST = 103;
    static constexpr Name WWW_AUTHENTICATE = 104;
    static constexpr Name X_CONTENT_TYPE_OPTIONS = 105;
    static constexpr Name X_FRAME_OPTIONS = 106;

    // Header names by mapped to Name
    static const inline Map NamedKeys{
        {"accept", ACCEPT},
        {"accept-ch", ACCEPT_CH},
        {"accept-encoding", ACCEPT_ENCODING},
        {"accept-language", ACCEPT_LANGUAGE},
        {"accept-patch", ACCEPT_PATCH},
        {"accept-post", ACCEPT_POST},
        {"accept-ranges", ACCEPT_RANGES},
        {"access-control-allow-credentials", ACCESS_CONTROL_ALLOW_CREDENTIALS},
        {"access-control-allow-headers", ACCESS_CONTROL_ALLOW_HEADERS},
        {"access-control-allow-methods", ACCESS_CONTROL_ALLOW_METHODS},
        {"access-control-allow-origin", ACCESS_CONTROL_ALLOW_ORIGIN},
        {"access-control-expose-headers", ACCESS_CONTROL_EXPOSE_HEADERS},
        {"access-control-max-age", ACCESS_CONTROL_MAX_AGE},
        {"access-control-request-headers", ACCESS_CONTROL_REQUEST_HEADERS},
        {"access-control-request-method", ACCESS_CONTROL_REQUEST_METHOD},
        {"activate-storage-access", ACTIVATE_STORAGE_ACCESS},
        {"age", AGE},
        {"allow", ALLOW},
        {"alt-svc", ALT_SVC},
        {"alt-used", ALT_USED},
        {"authorization", AUTHORIZATION},
        {"cache-control", CACHE_CONTROL},
        {"clear-site-data", CLEAR_SITE_DATA},
        {"connection", CONNECTION},
        {"content-digest", CONTENT_DIGEST},
        {"content-disposition", CONTENT_DISPOSITION},
        {"content-encoding", CONTENT_ENCODING},
        {"content-language", CONTENT_LANGUAGE},
        {"content-length", CONTENT_LENGTH},
        {"content-location", CONTENT_LOCATION},
        {"content-range", CONTENT_RANGE},
        {"content-security-policy", CONTENT_SECURITY_POLICY},
        {"content-security-policy-report-only", CONTENT_SECURITY_POLICY_REPORT_ONLY},
        {"content-type", CONTENT_TYPE},
        {"cookie", COOKIE},
        {"cross-origin-embedder-policy", CROSS_ORIGIN_EMBEDDER_POLICY},
        {"cross-origin-embedder-policy-report-only", CROSS_ORIGIN_EMBEDDER_POLICY_REPORT_ONLY},
        {"cross-origin-opener-policy", CROSS_ORIGIN_OPENER_POLICY},
        {"cross-origin-resource-policy", CROSS_ORIGIN_RESOURCE_POLICY},
        {"date", DATE},
        {"etag", ETAG},
        {"expect", EXPECT},
        {"expires", EXPIRES},
        {"forwarded", FORWARDED},
        {"from", FROM},
        {"host", HOST},
        {"if-match", IF_MATCH},
        {"if-modified-since", IF_MODIFIED_SINCE},
        {"if-none-match", IF_NONE_MATCH},
        {"if-range", IF_RANGE},
        {"if-unmodified-since", IF_UNMODIFIED_SINCE},
        {"integrity-policy", INTEGRITY_POLICY},
        {"integrity-policy-report-only", INTEGRITY_POLICY_REPORT_ONLY},
        {"keep-alive", KEEP_ALIVE},
        {"last-modified", LAST_MODIFIED},
        {"link", LINK},
        {"location", LOCATION},
        {"max-forwards", MAX_FORWARDS},
        {"origin", ORIGIN},
        {"origin-agent-cluster", ORIGIN_AGENT_CLUSTER},
        {"prefer", PREFER},
        {"preference-applied", PREFERENCE_APPLIED},
        {"priority", PRIORITY},
        {"proxy-authenticate", PROXY_AUTHENTICATE},
        {"proxy-authorization", PROXY_AUTHORIZATION},
        {"range", RANGE},
        {"referer", REFERER},
        {"referrer-policy", REFERRER_POLICY},
        {"refresh", REFRESH},
        {"reporting-endpoints", REPORTING_ENDPOINTS},
        {"repr-digest", REPR_DIGEST},
        {"retry-after", RETRY_AFTER},
        {"sec-fetch-dest", SEC_FETCH_DEST},
        {"sec-fetch-mode", SEC_FETCH_MODE},
        {"sec-fetch-site", SEC_FETCH_SITE},
        {"sec-fetch-storage-access", SEC_FETCH_STORAGE_ACCESS},
        {"sec-fetch-user", SEC_FETCH_USER},
        {"sec-purpose", SEC_PURPOSE},
        {"sec-websocket-accept", SEC_WEBSOCKET_ACCEPT},
        {"sec-websocket-extensions", SEC_WEBSOCKET_EXTENSIONS},
        {"sec-websocket-key", SEC_WEBSOCKET_KEY},
        {"sec-websocket-protocol", SEC_WEBSOCKET_PROTOCOL},
        {"sec-websocket-version", SEC_WEBSOCKET_VERSION},
        {"server", SERVER},
        {"server-timing", SERVER_TIMING},
        {"service-worker", SERVICE_WORKER},
        {"service-worker-allowed", SERVICE_WORKER_ALLOWED},
        {"service-worker-navigation-preload", SERVICE_WORKER_NAVIGATION_PRELOAD},
        {"set-cookie", SET_COOKIE},
        {"set-login", SET_LOGIN},
        {"sourcemap", SOURCEMAP},
        {"strict-transport-security", STRICT_TRANSPORT_SECURITY},
        {"te", TE},
        {"timing-allow-origin", TIMING_ALLOW_ORIGIN},
        {"trailer", TRAILER},
        {"transfer-encoding", TRANSFER_ENCODING},
        {"upgrade", UPGRADE},
        {"upgrade-insecure-requests", UPGRADE_INSECURE_REQUESTS},
        {"user-agent", USER_AGENT},
        {"vary", VARY},
        {"via", VIA},
        {"want-content-digest", WANT_CONTENT_DIGEST},
        {"want-repr-digest", WANT_REPR_DIGEST},
        {"www-authenticate", WWW_AUTHENTICATE},
        {"x-content-type-options", X_CONTENT_TYPE_OPTIONS},
        {"x-frame-options", X_FRAME_OPTIONS},
    };

} typedef Key;

#endif