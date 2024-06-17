#include "RequestLine.hpp"
#include <sstream>

RequestLine::RequestLine()
: mMethod("")
, mRequestURL("")
, mHTTPVersion("")
{
}
RequestLine::~RequestLine()
{
}
void RequestLine::parseRequestLine(const std::string& requestLine)
{
    // TODO: parse request line
    // request-line = <method> SP <request-URL> SP <HTTP-version> CRLF

    std::istringstream iss(requestLine);
    if (iss >> mMethod >> mRequestURL >> mHTTPVersion)
    {
        // success
    }
    else
    {
        // fail
        mMethod = "";
        mRequestURL = "";
        mHTTPVersion = "";
        throw std::exception();
    }
    
}
std::string RequestLine::getMethod(void) const
{
    return mMethod;
}
std::string RequestLine::getRequestURL(void) const
{
    return mRequestURL;
}
std::string RequestLine::getHTTPVersion(void) const
{
    return mHTTPVersion;
}
