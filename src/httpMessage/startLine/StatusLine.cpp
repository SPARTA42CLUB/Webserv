#include "StatusLine.hpp"

StatusLine::StatusLine()
: mHTTPVersion("")
, mStatusCode("")
, mReasonPhrase("")
{
}
StatusLine::~StatusLine()
{
}
const std::string& StatusLine::getHTTPVersion() const
{
    return mHTTPVersion;
}
const std::string& StatusLine::getStatusCode() const
{
    return mStatusCode;
}
const std::string& StatusLine::getReasonPhrase() const
{
    return mReasonPhrase;
}
void StatusLine::setHTTPVersion(const std::string& httpVersion)
{
    mHTTPVersion = httpVersion;
}
void StatusLine::setStatusCode(const std::string& statusCode)
{
    mStatusCode = statusCode;
}
void StatusLine::setReasonPhrase(const std::string& reasonPhrase)
{
    mReasonPhrase = reasonPhrase;
}
const std::string StatusLine::toString() const
{
    return mHTTPVersion + ' ' + mStatusCode + ' ' + mReasonPhrase + "\r\n";
}
