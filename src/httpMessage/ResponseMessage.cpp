#include "ResponseMessage.hpp"

ResponseMessage::ResponseMessage()
: mStatusLine()
, mResponseHeaderFields()
, mMessageBody()
{
}
ResponseMessage::~ResponseMessage()
{
}
void ResponseMessage::setStatusLine(const std::string& httpVersion, const std::string& statusCode, const std::string& reasonPhrase)
{
    mStatusLine.setHTTPVersion(httpVersion);
    mStatusLine.setStatusCode(statusCode);
    mStatusLine.setReasonPhrase(reasonPhrase);
}
void ResponseMessage::setStatusLine(const std::string& httpVersion, const int statusCode, const std::string& reasonPhrase)
{
    mStatusLine.setHTTPVersion(httpVersion);
    mStatusLine.setStatusCode(statusCode);
    mStatusLine.setReasonPhrase(reasonPhrase);
}
void ResponseMessage::addResponseHeaderField(const std::string& key, const std::string& value)
{
    mResponseHeaderFields.addField(key, value);
}
void ResponseMessage::addResponseHeaderField(const std::string& key, const int value)
{
    mResponseHeaderFields.addField(key, value);
}
void ResponseMessage::addMessageBody(const std::string& body)
{
    mMessageBody.addBody(body);
}
std::string ResponseMessage::toString(void) const
{
    std::ostringstream oss;
    oss << mStatusLine.toStatusLine() << mResponseHeaderFields.toString() << "\r\n" << mMessageBody.toString();
    return oss.str();
}
size_t ResponseMessage::getMessageBodySize() const
{
    return mMessageBody.size();
}
bool ResponseMessage::isKeepAlive() const
{
    return mResponseHeaderFields.getField("Connection") == "keep-alive";
}
void ResponseMessage::clearMessageBody()
{
    mMessageBody.clear();
}
