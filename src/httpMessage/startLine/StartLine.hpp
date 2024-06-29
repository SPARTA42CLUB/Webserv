#ifndef START_LINE_HPP
#define START_LINE_HPP

#include <string>

// https://www.rfc-editor.org/rfc/rfc9110.html#name-status-codes
// https://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
enum eInformational
{
    CONTINUE = 100,
    SWITCHING_PROTOCOLS = 101
};

enum eSuccess
{
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NON_AUTHORITATIVE_INFORMATION = 203,
    NO_CONTENT = 204,
    RESET_CONTENT = 205,
    PARTIAL_CONTENT = 206
};

enum eRedirection
{
    MULTIPLE_CHOICES = 300,
    MOVED_PERMANENTLY = 301,
    FOUND = 302,
    SEE_OTHER = 303,
    NOT_MODIFIED = 304,
    USE_PROXY = 305,
    TEMPORARY_REDIRECT = 307,
    PERMANENT_REDIRECT = 308
};

enum eClientError
{
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    PAYMENT_REQUIRED = 402,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    NOT_ACCEPTABLE = 406,
    PROXY_AUTHENTICATION_REQUIRED = 407,
    REQUEST_TIMEOUT = 408,
    CONFLICT = 409,
    GONE = 410,
    LENGTH_REQUIRED = 411,
    PRECONDITION_FAILED = 412,
    CONTENT_TOO_LARGE = 413,
    URI_TOO_LONG = 414,
    UNSUPPORTED_MEDIA_TYPE = 415,
    RANGE_NOT_SATISFIABLE = 416,
    EXPECTATION_FAILED = 417,
    IM_A_TEAPOT = 418,
    MISDIRECTED_REQUEST = 421,
    UNPROCESSABLE_ENTITY = 422,
    LOCKED = 423,
    FAILED_DEPENDENCY = 424,
    UPGRADE_REQUIRED = 426,
    PRECONDITION_REQUIRED = 428,
    TOO_MANY_REQUESTS = 429,
    REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    UNAVAILABLE_FOR_LEGAL_REASONS = 451
};

enum eServerError
{
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    BAD_GATEWAY = 502,
    SERVICE_UNAVAILABLE = 503,
    GATEWAY_TIMEOUT = 504,
    HTTP_VERSION_NOT_SUPPORTED = 505,
    VARIANT_ALSO_NEGOTIATES = 506,
    INSUFFICIENT_STORAGE = 507,
    LOOP_DETECTED = 508,
    NOT_EXTENDED = 510,
    NETWORK_AUTHENTICATION_REQUIRED = 511
};

class StartLine
{
private:
    std::string mElements[3];
    enum eStartLine
    {
        HTTP_VERSION = 0,
        STATUS_CODE = 1,
        REASON_PHRASE = 2,
        METHOD = 1,
        REQUEST_TARGET = 2
    };
    StartLine(const StartLine& rhs);
    StartLine& operator=(const StartLine& rhs);

public:
    StartLine();
    ~StartLine();
    const std::string& getHTTPVersion() const;
    const std::string& getStatusCode() const;
    const std::string& getMethod() const;
    const std::string& getRequestTarget() const;
    void setHTTPVersion(const std::string& httpVersion);
    void setStatusCode(const std::string& statusCode);
    void setStatusCode(const int statusCode);
    void setReasonPhrase(const std::string& reasonPhrase);
    void parseRequestLine(const std::string& requestLine);
    void parseStatusLine(const std::string& statusLine);
    const std::string toRequestLine() const;
    const std::string toStatusLine() const;

};

bool checkStatusCode(int statusCode);

#endif
