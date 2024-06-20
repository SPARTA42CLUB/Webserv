#include "RequestLine.hpp"
#include <sstream>
#include "HTTPException.hpp"

RequestLine::RequestLine()
: mMethod("")
, mRequestTarget("")
, mHTTPVersion("")
{
}
RequestLine::~RequestLine()
{
}
void RequestLine::parseRequestLine(const std::string& requestLine)
{
    // TODO: parse request line
    // request-line = <method> SP <request-target> SP <HTTP-version> CRLF

    std::istringstream iss(requestLine);
    if (iss >> mMethod >> mRequestTarget >> mHTTPVersion)
    {
        // CRLF 확인
        std::string remaining;
        std::getline(iss, remaining);
        if (remaining == "\r" || remaining.empty()) // CRLF 혹은 EOF가 남아있는 경우
        {
            return;
        }
    }
    // 파싱 실패 시 예외 처리
    mMethod.clear();
    mRequestTarget.clear();
    mHTTPVersion.clear();
    throw HTTPException(BAD_REQUEST);
}
std::string RequestLine::getMethod(void) const
{
    return mMethod;
}
std::string RequestLine::getRequestTarget(void) const
{
    return mRequestTarget;
}
std::string RequestLine::getHTTPVersion(void) const
{
    return mHTTPVersion;
}
