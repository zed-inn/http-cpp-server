#include <vector>
#include <unordered_map>
#include <variant>
#include "../interface/parser.hpp"
#include "../utils/http-utils.hpp"

#ifndef _C_HTTP_TARGET_URI_PARSER_
#define _C_HTTP_TARGET_URI_PARSER_ 1

// Conditional String allocates str on special conditions
typedef std::variant<str, strv> ConditionalStr;

// Operator '<<' for using cout to print ConditionalStr
std::ostream &operator<<(std::ostream &os, ConditionalStr const &v)
{
    std::visit([&](auto const &s)
               { os << s; }, v);
    return os;
}

class AuthorityValidation
{
public:
    static inline bool isValidIPv6(strv addr)
    {
        // Check if converted from ipv4
        auto v4pref = addr.find("::ffff:");
        if (v4pref == 0)
            return isValidIPv4(addr);

        // Check if length within 39
        if (addr.size() > 39)
            return false;

        // Checks specific lengths
        if (addr.size() < 2) // atleast needs two colons
            return false;
        else if (addr.size() == 2 && (addr[0] != ':' || addr[1] != ':')) // must be '::'
            return false;
        else if (*addr.begin() == ':' && *addr.end() == ':') // Mustn't start and end with colons
            return false;

        // Using char as all of them will be within <256
        unsigned char c, digits, colons, colons2x, fields, dgsInField;
        digits = colons2x = colons = fields = dgsInField = 0;

        for (auto p = addr.begin(); p < addr.end(); p++)
        {
            c = *p; // For NOT indirection many times

            // Checks for allowable characters
            if (isHexDigit(c))
                digits++, dgsInField++;
            else if (c == ':')
            {
                colons++;

                // Check digits in one field
                if (dgsInField > 4)
                    return false;
                dgsInField = 0;

                // Checks for 2x colons
                if (p + 1 < addr.end() && *(p + 1) == ':')
                    colons2x++;
            }
            else
                return false;
        }

        // Checks if all values within valid range
        if (colons > 7 || digits > 32 || colons2x > 1 || dgsInField > 4)
            return false;

        // Fields could be less in case of '::' appearing
        return colons2x ? fields <= 8 : fields == 8;
    }

    static inline bool isValidIPv4(strv addr)
    {
        // Checks size with valid bounds
        if (addr.size() < 7 || addr.size() > 15)
            return false;

        // Checks for allowable characters
        for (auto x : addr)
            if (!(x >= '0' && x <= '9') && x != '.')
                return false;

        size_t pos;
        strv val;
        unsigned int ival;
        unsigned char fields = 0;

        while ((pos = addr.find('.')) != strv::npos)
        {
            // Checks if field contains 1-3 digits
            val = addr.substr(0, pos);
            if (val.size() <= 0 || val.size() > 3)
                return false;

            // Check the numeric value to be in 0-255
            ival = stoi(str(val));
            if (ival > 255)
                return false;
            fields++; // Increment fields

            addr = addr.substr(pos + 1);
        }

        // If some field still remains after last '.'
        if (addr.size() <= 0)
            return false;

        // Check last field
        ival = stoi(str(addr));
        if (ival > 255)
            return false;
        fields++;

        return fields == 4;
    }

    static inline bool isValidDomainName(strv domain)
    {
        // Checks if there is something
        if (domain.size() <= 0)
            return false;

        // Checks it shouldn't start with dots
        if (*domain.begin() == '.' || *domain.end() == '.')
            return false;

        // Checks for allowable characters
        for (auto x : domain)
            if (!isalnum(x) && x != '.')
                return false;

        // Checks if 2 '.' doesn't come together
        size_t pos;
        while ((pos = domain.find('.')) != strv::npos)
        {
            if (pos + 1 < domain.size() && domain[pos + 1] == '.')
                return false;
            domain = domain.substr(pos + 1);
        }

        return true;
    }

    static inline bool isValidPortNumber(strv p)
    {
        // Checks if size is less than 5 (0-65535)
        if (p.size() <= 0 || p.size() > 5)
            return false;

        // Checks for allowable characters
        for (auto x : p)
            if (x < '0' || x > '9')
                return false;

        // Checks numerica value to be within 0-65535
        unsigned int ip = stoi(str(p));
        return ip <= 65535;
    }
};

class HttpTargetUri : Parser
{
private:
    typedef std::unordered_map<ConditionalStr, ConditionalStr> Query;

public:
    struct URIContext
    {
        bool isPathAuthorityForm; // In context when needed for OPTIONS method
        ConditionalStr path;
        Query query;

    } typedef Context;

    static constexpr size_t MAX_TARGET_URI_LENGTH = 2048;

private:
    static constexpr size_t MAX_QUERY_KEY_LENGTH = 256;
    static constexpr size_t MAX_QUERY_VALUE_LENGTH = 2048;

    static constexpr size_t FRONT_RESERVED_PATH_SPACE = 32;
    static constexpr size_t FRONT_RESERVED_QUERY_SPACE = 16;

    Context *_uc;

    ParseResult decodeUriEncoded(strv v, str *s, strv nonConvertables = "")
    {
        size_t p, tmp;
        int charNum;

        while ((p = v.find('%')) != strv::npos)
        {
            // Add till before '%'
            *s += v.substr(0, p);

            // Decode
            if (p + 2 < v.size() && isHexDigit(v[p + 1]) && isHexDigit(v[p + 2]))
            {
                // Convert hex to regular ascii value
                charNum = stoi(str({v[p + 1], v[p + 2]}), nullptr, 16);

                // If not allowed to be converted
                if ((tmp = nonConvertables.find(charNum)) != strv::npos)
                {
                    s += '%', v = v.substr(p + 1);
                    continue;
                }

                // Increment by 3 for %XX
                *s += charNum, v = v.substr(p + 3);
            }

            // Not valid %XX
            else
                return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid URI");
        }

        // Some path still remaining
        if (v.size() > 0)
            *s += v;

        return ParseResult();
    }

    ParseResult decodeAndSaveQueryParameters(strv key, strv val)
    {
        ConditionalStr _key, _val;
        ParseResult res;

        // Decode key first

        size_t pos;
        if ((pos = key.find('%')) == strv::npos && (pos = key.find('+')) == strv::npos)
            _key = key;

        // have to decode
        else
        {
            str s;
            s.reserve(FRONT_RESERVED_QUERY_SPACE);

            if (!(res = decodeUriEncoded(key, &s)).success)
                return res;

            // Convert all '+' to ' '
            for (auto p = s.begin(); p < s.end(); p++)
                if (*p == '+')
                    *p = ' ';

            _key = s; // Set the string to be key
        }

        // Decode value now

        if ((pos = val.find('%')) == strv::npos && (pos = key.find('+')) == strv::npos)
            _val = val;

        // have to decode
        else
        {
            str s;
            s.reserve(FRONT_RESERVED_QUERY_SPACE);

            if (!(res = decodeUriEncoded(val, &s)).success)
                return res;

            // Convert all '+' to ' '
            for (auto p = s.begin(); p < s.end(); p++)
                if (*p == '+')
                    *p = ' ';

            _val = s; // Set the string to be key
        }

        // Save the pair
        _uc->query[_key] = _val;

        return ParseResult();
    }

    ParseResult parseQuery(strv qs)
    {
        // Separate by '&'
        size_t pos, eq;
        strv temp, key, val;
        ParseResult res;
        while ((pos = qs.find('&')) != strv::npos)
        {
            // Get only one block
            temp = qs.substr(0, pos);

            if ((eq = temp.find('=')) == strv::npos)
                key = temp, val = temp.substr(temp.size() - 1, 0);
            else
                key = temp.substr(0, eq), val = temp.substr(eq + 1);

            // Decode query and val
            if (!(res = decodeAndSaveQueryParameters(key, val)).success)
                return res;

            qs = qs.substr(pos + 1); // Move to next &-block
        }

        // If still some query-string left
        if (qs.size() > 0)
        {
            if ((eq = qs.find('=')) == strv::npos)
                key = qs, val = qs.substr(qs.size() - 1, 0);
            else
                key = qs.substr(0, eq), val = qs.substr(eq + 1);

            // Decode query and val
            if (!(res = decodeAndSaveQueryParameters(key, val)).success)
                return res;
        }

        return ParseResult();
    }

    ParseResult validateAuthority(strv a)
    {
        // Checks if it is something
        if (a.size() <= 0)
            return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Authority");

        // Check if Ipv6
        if (a[0] == '[')
        {
            // Check if it closes and is valid ipv6
            auto b = a.find(']');
            if (b == strv::npos || !AuthorityValidation::isValidIPv6(a.substr(1, b)))
                return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Authority");

            a = a.substr(b + 1); // Move to [:port] part
        }

        // Check if Ipv4 or domain name
        else if (isalnum(a[0]))
        {
            // Get the colon position
            auto c = a.find(':');
            strv temp = c != strv::npos ? a.substr(0, c) : a;

            // Checks if not ipv4 and not domain name
            if (!AuthorityValidation::isValidIPv4(temp) && !AuthorityValidation::isValidDomainName(temp))
                return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Authority");

            a = c != strv::npos ? a.substr(c) : a.substr(a.size() - 1, 0); // including ':' due to ipv6 also having
        }

        // Invalid
        else
            return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Authority");

        // Validate port

        // If nothing here, or only colon
        if (a.size() <= 0 || (a.size() == 1 && a[0] == ':'))
            return ParseResult();

        // If colon and value later
        if (a.size() > 1 && AuthorityValidation::isValidPortNumber(a.substr(1)))
            return ParseResult();
        else
            return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid Authority");
    }

    ParseResult decodeAndSaveUriPath(strv u)
    {
        // Check if something to decode '%'
        auto pf = u.find('%');
        if (pf == strv::npos)
        {
            _uc->path = u;
            return ParseResult();
        }

        // Decoding is required
        str s;
        ParseResult res;
        s.reserve(FRONT_RESERVED_PATH_SPACE);

        // Skip if decoded to be ?, # or / which can change meaning of uri
        if (!(res = decodeUriEncoded(u, &s, "?#/")).success)
            return res;

        _uc->path = s;

        return ParseResult();
    }

    ParseResult parsePath(strv u)
    {
        if (u.size() <= 0)
            return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid URI");

        size_t ht;
        ParseResult res;

        // Origin form
        if (u[0] == '/')
            return decodeAndSaveUriPath(u);

        // Asterisk form
        else if (u[0] == '*')
        {
            // is not just '*'
            if (u.size() > 1)
                return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid URI");

            // Set to just '/'
            _uc->path = str("/");
        }

        // Absolute form
        else if ((ht = u.find("http://")) == 0 || (ht = u.find("https://")) == 0)
        {
            // Check if 5th character is 's' for differentiating http or https scheme
            u.remove_prefix(u[4] == 's' ? 8 : 7);

            // Split in authority and path
            auto sl = u.find('/');
            if (sl == strv::npos)
                return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid URI");

            // Validate authority
            if (!(res = validateAuthority(u.substr(0, sl))).success)
                return res;

            return decodeAndSaveUriPath(u.substr(sl));
        }

        // Authority form
        else
        {
            // Must not have any '/' in uri
            auto sl = u.find('/');
            if (sl != strv::npos)
                return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid URI");

            // Validate authority
            if (!(res = validateAuthority(u)).success)
                return res;

            // Set to just '/'
            _uc->path = str("/");
            _uc->isPathAuthorityForm = true;
        }

        return ParseResult();
    }

public:
    HttpTargetUri(Context *uric) : _uc(uric) {}

    ParseResult parse(strv s) override
    {
        if (s.size() > MAX_TARGET_URI_LENGTH)
            return ParseResult(HttpStatusCode::URI_TOO_LONG, "URI Too Long");

        if (s.size() <= 0)
            return ParseResult(HttpStatusCode::BAD_REQUEST, "Empty URI");

        // If any space exists in path
        auto sp = s.find_first_of(" \t");
        if (sp != strv::npos)
            return ParseResult(HttpStatusCode::BAD_REQUEST, "Invalid URI");

        // Remove fragment, if any
        auto f = s.find('#');
        if (f != strv::npos)
            s = s.substr(0, f);

        // Separate parsing of query and path
        auto q = s.find('?');
        if (q == strv::npos)
            return parsePath(s);
        else
        {
            auto res = parsePath(s.substr(0, q));
            if (!res.success)
                return res;
            return parseQuery(s.substr(q + 1));
        }
    };
};

#endif