#include "ResponseMessage.hpp"
#include "HttpException.hpp"
#include "Connection.hpp"

ResponseMessage::ResponseMessage()
: mStatusLine()
, mResponseHeaderFields()
, mMessageBody()
{
}
ResponseMessage::~ResponseMessage()
{
}
void ResponseMessage::parseResponseMessage(const std::string& response)
{
    std::istringstream resStream(response);
    try
    {
        parseStatusLine(resStream);
        parseResponseHeaderFields(resStream);
        parseMessageBody(resStream);
    }
    catch (const HttpException& e)
    {
        throw HttpException(BAD_GATEWAY);
    }
}
void ResponseMessage::parseStatusLine(std::istringstream& resStream)
{
    std::string StatusLine;
    std::getline(resStream, StatusLine);
    try
    {
        mStatusLine.parseStatusLine(StatusLine);
    }
    catch (const HttpException& e)
    {
        throw HttpException(BAD_GATEWAY);
    }
}
void ResponseMessage::parseResponseHeaderFields(std::istringstream& resStream)
{
    try
    {
        mResponseHeaderFields.parseHeaderFields(resStream);
    }
    catch (const HttpException& e)
    {
        throw HttpException(BAD_GATEWAY);
    }
}
void ResponseMessage::parseMessageBody(std::istringstream& resStream)
{
    std::ostringstream oss;
    oss << resStream.rdbuf();
    mMessageBody.addBody(oss.str());
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
    return mStatusLine.toStatusLine() + mResponseHeaderFields.toString() + mMessageBody.toString();
}
size_t ResponseMessage::getMessageBodySize() const
{
    return mMessageBody.size();
}
const HeaderFields& ResponseMessage::getResponseHeaderFields() const
{
    return mResponseHeaderFields;
}
void ResponseMessage::clearMessageBody()
{
    mMessageBody.clear();
}
std::string ResponseMessage::ResponseMessage::getErrorMsgBody(const int statusCode, const ServerConfig& serverConfig) const
{
    std::string errorPageBody("");
    std::map<size_t, std::string>::const_iterator errIt = serverConfig.error_pages.find(statusCode);
    if (errIt == serverConfig.error_pages.end())
    {
        return errorPageBody;
    }
    std::map<std::string, LocationConfig>::const_iterator locIt = serverConfig.locations.find(errIt->second);
    if (locIt == serverConfig.locations.end())
    {
        return errorPageBody;
    }
    const std::string path = locIt->second.root + errIt->second;
    std::ifstream file(path);
    if (file.is_open())
    {
        std::ostringstream oss;
        oss << file.rdbuf();
        errorPageBody = oss.str();
    }
    return errorPageBody;
}
void ResponseMessage::setByStatusCode(const int statusCode, const ServerConfig& serverConfig)
{
    const std::string errorMsgBody = getErrorMsgBody(statusCode, serverConfig);
    if (statusCode == BAD_REQUEST)
    {
        badRequest(errorMsgBody);
    }
    else if (statusCode == FORBIDDEN)
    {
        forbidden(errorMsgBody);
    }
    else if (statusCode == NOT_FOUND)
    {
        notFound(errorMsgBody);
    }
    else if (statusCode == METHOD_NOT_ALLOWED)
    {
        methodNotAllowed(errorMsgBody);
    }
    else if (statusCode == URI_TOO_LONG)
    {
        uriTooLong(errorMsgBody);
    }
    else if (statusCode == CONTENT_TOO_LARGE)
    {
        contentTooLarge(errorMsgBody);
    }
    else if (statusCode == BAD_GATEWAY)
    {
        badGateway(errorMsgBody);
    }
    else if (statusCode == SERVICE_UNAVAILABLE)
    {
        serviceUnavailable(errorMsgBody);
    }
    else if (statusCode == HTTP_VERSION_NOT_SUPPORTED)
    {
        httpVersionNotSupported(errorMsgBody);
    }
}
void ResponseMessage::badRequest(const std::string& body)
{
    if (body.empty())
        addMessageBody("<html><head><title>400 Bad Request</title></head><body><h1>400 Bad Request</h1></body></html>");
    else
        addMessageBody(body);
    setStatusLine("HTTP/1.1", BAD_REQUEST, "Bad Request");
    addResponseHeaderField("Content-Type", "text/html");
    addResponseHeaderField("Connection", "close");
    addSemanticHeaderFields();
}
void ResponseMessage::forbidden(const std::string& body)
{
    if (body.empty())
        addMessageBody("<html><head><title>403 Forbidden</title></head><body><h1>403 Forbidden</h1></body></html>");
    else
        addMessageBody(body);
    setStatusLine("HTTP/1.1", FORBIDDEN, "Forbidden");
    addResponseHeaderField("Content-Type", "text/html");
    addSemanticHeaderFields();
}
void ResponseMessage::notFound(const std::string& body)
{
    if (body.empty())
        addMessageBody("<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>");
    else
        addMessageBody(body);
    setStatusLine("HTTP/1.1", NOT_FOUND, "Not Found");
    addResponseHeaderField("Content-Type", "text/html");
    addSemanticHeaderFields();
}
void ResponseMessage::methodNotAllowed(const std::string& body)
{
    if (body.empty())
        addMessageBody("<html><head><title>405 Method Not Allowed</title></head><body><h1>405 Method Not Allowed</h1></body></html>");
    else
        addMessageBody(body);
    setStatusLine("HTTP/1.1", METHOD_NOT_ALLOWED, "Method Not Allowed");
    addResponseHeaderField("Content-Type", "text/html");
    addSemanticHeaderFields();
}
void ResponseMessage::uriTooLong(const std::string& body)
{
    if (body.empty())
        addMessageBody("<html><head><title>414 Request-URI Too Long</title></head><body><h1>414 Request-URI Too Long</h1></body></html>");
    else
        addMessageBody(body);
    setStatusLine("HTTP/1.1", URI_TOO_LONG, "Request-URI Too Long");
    addResponseHeaderField("Content-Type", "text/html");
    addSemanticHeaderFields();
}
void ResponseMessage::contentTooLarge(const std::string& body)
{
    if (body.empty())
        addMessageBody("<html><head><title>413 Content Too Large</title></head><body><h1>413 Content Too Large</h1></body></html>");
    else
        addMessageBody(body);
    setStatusLine("HTTP/1.1", CONTENT_TOO_LARGE, "Content Too Large");
    addResponseHeaderField("Content-Type", "text/html");
    addResponseHeaderField("Connection", "close");
    addSemanticHeaderFields();
}
void ResponseMessage::badGateway(const std::string& body)
{
    if (body.empty())
        addMessageBody("<html><head><title>502 Bad Gateway</title></head><body><h1>502 Bad Gateway</h1></body></html>");
    else
        addMessageBody(body);
    setStatusLine("HTTP/1.1", BAD_GATEWAY, "Bad Gateway");
    addResponseHeaderField("Content-Type", "text/html");
    addSemanticHeaderFields();
}
void ResponseMessage::serviceUnavailable(const std::string& body)
{
    if (body.empty())
        addMessageBody("<html><head><title>503 Service Unavailable</title></head><body><h1>503 Service Unavailable</h1></body></html>");
    else
        addMessageBody(body);
    setStatusLine("HTTP/1.1", SERVICE_UNAVAILABLE, "Service Unavailable");
    addResponseHeaderField("Content-Type", "text/html");
    addSemanticHeaderFields();
}
void ResponseMessage::httpVersionNotSupported(const std::string& body)
{
    if (body.empty())
        addMessageBody("<html><head><title>505 HTTP Version Not Supported</title></head><body><h1>505 HTTP Version Not Supported</h1></body></html>");
    else
        addMessageBody(body);
    setStatusLine("HTTP/1.1", HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported");
    addResponseHeaderField("Content-Type", "text/html");
    addSemanticHeaderFields();
}
void ResponseMessage::addSemanticHeaderFields(void)
{
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[128];
    // Date: Tue, 15 Nov 1994 08:12:31 GMT
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    std::string date = buffer;

    addResponseHeaderField("Date", date);
    addResponseHeaderField("Server", "webserv");
    addResponseHeaderField("Content-Length", getMessageBodySize());
}

void ResponseMessage::setConnection(Connection& connection)
{
    if (checkNeedClose(connection))
    {
        addResponseHeaderField("Connection", "close");
    }
    else
    {
        addResponseHeaderField("Connection", "keep-alive");
    }
}
