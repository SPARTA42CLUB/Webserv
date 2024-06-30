#include "StartLine.hpp"
#include <sstream>
#include "HttpException.hpp"
#include "parse.hpp"

StartLine::StartLine()
: mElements()
{
    std::fill(mElements, mElements + 3, "");
}
StartLine::~StartLine()
{
}
const std::string& StartLine::getHTTPVersion() const
{
    return mElements[HTTP_VERSION];
}
const std::string& StartLine::getStatusCode() const
{
    return mElements[STATUS_CODE];
}
const std::string& StartLine::getMethod() const
{
    return mElements[METHOD];
}
const std::string& StartLine::getRequestTarget() const
{
    return mElements[REQUEST_TARGET];
}
void StartLine::setHTTPVersion(const std::string& httpVersion)
{
    mElements[HTTP_VERSION] = httpVersion;
}
void StartLine::setStatusCode(const std::string& statusCode)
{
    mElements[STATUS_CODE] = statusCode;
}
void StartLine::setStatusCode(const int statusCode)
{
    mElements[STATUS_CODE] = std::to_string(statusCode);
}
void StartLine::setReasonPhrase(const std::string& reasonPhrase)
{
    mElements[REASON_PHRASE] = reasonPhrase;
}
const std::string StartLine::toRequestLine() const
{
    return mElements[METHOD] + " " + mElements[REQUEST_TARGET] + " " + mElements[HTTP_VERSION] + "\r\n";
}
const std::string StartLine::toStatusLine() const
{
    return mElements[HTTP_VERSION] + " " + mElements[STATUS_CODE] + " " + mElements[REASON_PHRASE] + "\r\n";
}
void StartLine::parseRequestLine(const std::string& requestLine)
{
    // <method> SP <request-target> SP <HTTP-version> CRLF

    std::istringstream iss(requestLine);
    if (iss >> mElements[METHOD] >> mElements[REQUEST_TARGET] >> mElements[HTTP_VERSION])
    {
        std::string remaining;
        std::getline(iss, remaining);
        if (remaining == "\r")  // CRLF 확인
        {
            return;
        }
    }
    // 파싱 실패 시 예외 처리
    mElements[METHOD].clear();
    mElements[REQUEST_TARGET].clear();
    mElements[HTTP_VERSION].clear();
    throw HttpException(BAD_REQUEST);
}
void StartLine::parseStatusLine(const std::string& statusLine)
{
    // <HTTP-version> SP <status-code> SP <reason-phrase> CRLF

    std::istringstream iss(statusLine);
    if (iss >> mElements[HTTP_VERSION] >> mElements[STATUS_CODE] >> mElements[REASON_PHRASE])
    {
        std::string remaining;
        std::getline(iss, remaining);
        trim(remaining);
        if (remaining.empty())
        {
            return;
        }
    }
    // 파싱 실패 시 예외 처리
    mElements[HTTP_VERSION].clear();
    mElements[STATUS_CODE].clear();
    mElements[REASON_PHRASE].clear();
    throw HttpException(BAD_GATEWAY);
}
bool checkStatusCode(int statusCode)
{
    switch (statusCode) {
        case BAD_REQUEST:
        case FORBIDDEN:
        case NOT_FOUND:
        case METHOD_NOT_ALLOWED:
        case URI_TOO_LONG:
        case CONTENT_TOO_LARGE:
        case HTTP_VERSION_NOT_SUPPORTED:
            return false;
        default:
            return true;
    }
}
