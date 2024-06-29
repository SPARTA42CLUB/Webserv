#include "RequestMessage.hpp"
#include "HttpException.hpp"

RequestMessage::RequestMessage()
: mRequestLine()
, mRequestHeaderFields()
, mMessageBody()
{
}
RequestMessage::~RequestMessage()
{
}
const StartLine& RequestMessage::getRequestLine() const
{
    return mRequestLine;
}
const HeaderFields& RequestMessage::getRequestHeaderFields() const
{
    return mRequestHeaderFields;
}
const MessageBody& RequestMessage::getMessageBody() const
{
    return mMessageBody;
}
void RequestMessage::parseRequestHeader(const std::string& request)
{
    std::istringstream reqStream(request);
    try
    {
        parseRequestLine(reqStream);
        parseRequestHeaderFields(reqStream);
    }
    catch (const HttpException& e)
    {
        throw e;
    }
}
void RequestMessage::parseRequestLine(std::istringstream& reqStream)
{
    std::string requestLine;
    std::getline(reqStream, requestLine);
    try
    {
        mRequestLine.parseRequestLine(requestLine);
    }
    catch (const HttpException& e)
    {
        throw e;
    }
}
void RequestMessage::parseRequestHeaderFields(std::istringstream& reqStream)
{
    try
    {
        mRequestHeaderFields.parseHeaderFields(reqStream);
    }
    catch (const HttpException& e)
    {
        throw e;
    }
}
void RequestMessage::addMessageBody(const std::string& body)
{
    mMessageBody.addBody(body);
}
void RequestMessage::verifyRequestMessage(void) const
{
    try
    {
        verifyRequestLine();
        verifyRequestHeaderFields();
    }
    catch (const HttpException& e)
    {
        throw e;
    }
}
void RequestMessage::verifyRequestLine(void) const
{
    const std::string method = mRequestLine.getMethod();
    const std::string reqTarget = mRequestLine.getRequestTarget();
    const std::string ver = mRequestLine.getHTTPVersion();
    if (method != "GET" && method != "HEAD" && method != "POST" && method != "DELETE")
    {
        throw HttpException(METHOD_NOT_ALLOWED);
    }
    if (ver != "HTTP/1.1")
    {
        throw HttpException(HTTP_VERSION_NOT_SUPPORTED);
    }
    if (reqTarget[0] != '/')
    {
        throw HttpException(NOT_FOUND);
    }
    if (reqTarget.size() >= MAX_URI_LENGTH)
    {
        throw HttpException(URI_TOO_LONG);
    }
}
void RequestMessage::verifyRequestHeaderFields(void) const
{
    if (mRequestHeaderFields.hasField("Host") == false)
    {
        throw HttpException(BAD_REQUEST);
    }
}
std::string RequestMessage::toString(void) const
{
    return mRequestLine.toRequestLine() + mRequestHeaderFields.toString() + mMessageBody.toString();
}
