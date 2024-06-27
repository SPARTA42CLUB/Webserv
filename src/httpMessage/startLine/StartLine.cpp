#include "StartLine.hpp"
#include <sstream>
#include "HttpException.hpp"

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
const std::string& StartLine::getReasonPhrase() const
{
    return mElements[REASON_PHRASE];
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
