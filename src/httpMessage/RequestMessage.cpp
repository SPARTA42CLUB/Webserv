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
