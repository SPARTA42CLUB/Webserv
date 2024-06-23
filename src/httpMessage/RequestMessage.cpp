#include "RequestMessage.hpp"
#include "HTTPException.hpp"

RequestMessage::RequestMessage()
: mRequestLine()
, mRequestHeaderFields()
, mMessageBody()
{
}
RequestMessage::RequestMessage(const std::string& request)
: mRequestLine()
, mRequestHeaderFields()
, mMessageBody()
{
    parseRequestMessage(request);
    verifyRequest(request);
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
    mRequestHeaderFields.parseHeaderFields(reqStream);
}
void RequestMessage::parseMessageBody(std::istringstream& reqStream)
{
    mMessageBody.parseMessageBody(reqStream);
}
void RequestMessage::verifyRequest(const RequestMessage& req)
{
    try
    {
        verifyRequestLine(req.getRequestLine());
        verifyRequestHeaderFields(req.getRequestHeaderFields());
    }
    catch (const HTTPException& e)
    {
        throw e;
    }
}
void RequestMessage::verifyRequestLine(const RequestLine& reqLine)
{
    const std::string method = reqLine.getMethod();
    const std::string reqTarget = reqLine.getRequestTarget();
    const std::string ver = reqLine.getHTTPVersion();
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
void RequestMessage::verifyRequestHeaderFields(const HeaderFields& reqHeaderFields)
{
    if (reqHeaderFields.hasField("Host") == false)
    {
        throw HTTPException(BAD_REQUEST);
    }
}
