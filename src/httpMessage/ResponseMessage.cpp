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
void ResponseMessage::clearMessageBody()
{
    mMessageBody.clear();
}

void ResponseMessage::setByStatusCode(const int statusCode)
{
    if (statusCode == BAD_REQUEST)
    {
        badRequest();
    }
    else if (statusCode == FORBIDDEN)
    {
        forbidden();
    }
    else if (statusCode == NOT_FOUND)
    {
        notFound();
    }
    else if (statusCode == METHOD_NOT_ALLOWED)
    {
        methodNotAllowed();
    }
    else if (statusCode == URI_TOO_LONG)
    {
        uriTooLong();
    }
    else if (statusCode == PAYLOAD_TOO_LARGE)
    {
        payloadTooLarge();
    }
    else if (statusCode == HTTP_VERSION_NOT_SUPPORTED)
    {
        httpVersionNotSupported();
    }
}

void ResponseMessage::badRequest(void)
{
    setStatusLine("HTTP/1.1", BAD_REQUEST, "Bad Request");
    addMessageBody("<html><head><title>400 Bad Request</title></head><body><h1>400 Bad Request</h1></body></html>");
    addResponseHeaderField("Content-Type", "text/html");
    addResponseHeaderField("Connection", "close");
    addSemanticHeaderFields();
}
void ResponseMessage::forbidden(void)
{
    setStatusLine("HTTP/1.1", FORBIDDEN, "Forbidden");
    addMessageBody("<html><head><title>403 Forbidden</title></head><body><h1>403 Forbidden</h1></body></html>");
    addResponseHeaderField("Content-Type", "text/html");
    addResponseHeaderField("Connection", "close");
    addSemanticHeaderFields();
}
void ResponseMessage::notFound(void)
{
    setStatusLine("HTTP/1.1", NOT_FOUND, "Not Found");
    addMessageBody("<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>");
    addResponseHeaderField("Content-Type", "text/html");
    addResponseHeaderField("Connection", "close");
    addSemanticHeaderFields();
}
void ResponseMessage::methodNotAllowed(void)
{
    setStatusLine("HTTP/1.1", METHOD_NOT_ALLOWED, "Method Not Allowed");
    addMessageBody("<html><head><title>405 Method Not Allowed</title></head><body><h1>405 Method Not Allowed</h1></body></html>");
    addResponseHeaderField("Content-Type", "text/html");
    addResponseHeaderField("Connection", "close");
    addSemanticHeaderFields();
}
void ResponseMessage::uriTooLong(void)
{
    setStatusLine("HTTP/1.1", URI_TOO_LONG, "Request-URI Too Long");
    addMessageBody("<html><head><title>414 Request-URI Too Long</title></head><body><h1>414 Request-URI Too Long</h1></body></html>");
    addResponseHeaderField("Content-Type", "text/html");
    addResponseHeaderField("Connection", "close");
    addSemanticHeaderFields();
}
void ResponseMessage::payloadTooLarge(void)
{
    setStatusLine("HTTP/1.1", PAYLOAD_TOO_LARGE, "Payload Too Large");
    addMessageBody("<html><head><title>413 Payload Too Large</title></head><body><h1>413 Payload Too Large</h1></body></html>");
    addResponseHeaderField("Content-Type", "text/html");
    addResponseHeaderField("Connection", "close");
    addSemanticHeaderFields();
}
void ResponseMessage::httpVersionNotSupported(void)
{
    setStatusLine("HTTP/1.1", HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported");
    addMessageBody(
        "<html><head><title>505 HTTP Version Not Supported</title></head><body><h1>505 HTTP Version Not Supported</h1></body></html>");
    addResponseHeaderField("Content-Type", "text/html");
    addResponseHeaderField("Connection", "close");
    addSemanticHeaderFields();
}

void ResponseMessage::addSemanticHeaderFields()
{
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[128];
    // Date: Tue, 15 Nov 1994 08:12:31 GMT
    // NOTE: strftime() 함수는 c함수라서 평가에 어긋남, 하지만 Date 헤더 필드가 필수가 아니라서 보너스 느낌으로 넣어둠
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    std::string date = buffer;

    // NOTE: expression result unused
    // mServerConfig.locations.find(mLocation)->CONFIG.cgi;

    addResponseHeaderField("Content-Length", getMessageBodySize());
    addResponseHeaderField("Server", "webserv");
    addResponseHeaderField("Date", date);
}
