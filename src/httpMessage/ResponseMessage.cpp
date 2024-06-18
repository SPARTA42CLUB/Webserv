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
    mMessageBody.addBody(body);
}
std::string ResponseMessage::toString(void)
{
    mResponseHeaderFields.addField("Content-Length", std::to_string(mMessageBody.size()));
    mResponseHeaderFields.addField("Server", "webserv");
    std::ostringstream oss;
    oss << mStatusLine.toString() << mResponseHeaderFields.toString() << "\r\n" << mMessageBody.toString();
    return oss.str();
}
void ResponseMessage::clearMessageBody()
{
    mMessageBody.clear();
}
