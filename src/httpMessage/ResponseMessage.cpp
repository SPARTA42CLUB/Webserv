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
void ResponseMessage::addResponseHeaderField(const std::string& key, const std::string& value)
{
    mResponseHeaderFields.addField(key, value);
}
void ResponseMessage::addMessageBody(const std::string& body)
{
    std::stringstream ss;
    ss << body.length();
    ResponseMessage::addResponseHeaderField("Content-Length", ss.str());
    mMessageBody.addBody(body);
}
std::string ResponseMessage::toString(void) const
{
    std::ostringstream oss;
    oss << mStatusLine.toString() << mResponseHeaderFields.toString() << "\r\n" << mMessageBody.toString();
    return oss.str();
}
