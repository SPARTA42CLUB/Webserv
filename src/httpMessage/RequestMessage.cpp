#include "RequestMessage.hpp"
#include "HTTPException.hpp"

RequestMessage::RequestMessage()
: mRequestLine()
, mRequestHeaderFields()
, mMessageBody()
, mStatusCode(200)
{
}
RequestMessage::RequestMessage(const std::string& request)
: mRequestLine()
, mRequestHeaderFields()
, mMessageBody()
, mStatusCode(200)
{
    try
    {
        parseRequestMessage(request);
        verifyRequest();
    }
    catch (const HTTPException& e)
    {
        mStatusCode = e.getStatusCode();
    }
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
void RequestMessage::parseRequestMessage(const std::string& request)
{
    std::istringstream reqStream(request);
    try
    {
        parseRequestLine(reqStream);
        parseRequestHeaderFields(reqStream);
        parseMessageBody(reqStream);
    }
    catch (const HTTPException& e)
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
    catch (const HTTPException& e)
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
    catch (const HTTPException& e)
    {
        throw e;
    }
}
void RequestMessage::parseMessageBody(std::istringstream& reqStream)
{
    try
    {
        mMessageBody.parseMessageBody(reqStream);
    }
    catch (const HTTPException& e)
    {
        throw e;
    }
}
void RequestMessage::verifyRequest(void)
{
    try
    {
        verifyRequestLine();
        verifyRequestHeaderFields();
    }
    catch (const HTTPException& e)
    {
        throw e;
    }
}
void RequestMessage::verifyRequestLine(void)
{
    const std::string method = mRequestLine.getMethod();
    const std::string reqTarget = mRequestLine.getRequestTarget();
    const std::string ver = mRequestLine.getHTTPVersion();
    if (method != "GET" && method != "HEAD" && method != "POST" && method != "DELETE")
    {
        throw HTTPException(METHOD_NOT_ALLOWED);
    }
    if (ver != "HTTP/1.1")
    {
        throw HTTPException(HTTP_VERSION_NOT_SUPPORTED);
    }
    if (reqTarget[0] != '/')
    {
        throw HTTPException(NOT_FOUND);
    }
    if (reqTarget.size() >= 8200)  // nginx max uri length
    {
        throw HTTPException(URI_TOO_LONG);
    }
}
void RequestMessage::verifyRequestHeaderFields(void)
{
    if (mRequestHeaderFields.hasField("Host") == false)
    {
        throw HTTPException(BAD_REQUEST);
    }
}
std::string RequestMessage::toString(void) const
{
    std::ostringstream oss;
    return oss.str();
}

int RequestMessage::getStatusCode() const
{
    return mStatusCode;
}
