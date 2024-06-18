#include "RequestMessage.hpp"

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
void RequestMessage::parseRequestMessage(const std::string& request)
{
    std::istringstream reqStream(request);
    try
    {
        parseRequestLine(reqStream);
        parseRequestHeaderFields(reqStream);
        parseMessageBody(reqStream);
    }
    catch(const std::exception& e)
    {
        // TODO: handle exception
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
    catch(const std::exception& e)
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

const HeaderFields& RequestMessage::getHeaderFields() const {
    return mRequestHeaderFields;
}
