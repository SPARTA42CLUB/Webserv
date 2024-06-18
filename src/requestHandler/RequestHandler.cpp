#include "RequestHandler.hpp"

void RequestHandler::handleRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig)
{
    std::string reqTarget = req.getRequestLine().getRequestTarget();
    std::string method = req.getRequestLine().getMethod();
    std::map<std::string, LocationConfig>::const_iterator it = serverConfig.locations.find(reqTarget);
    if (it == serverConfig.locations.end())
    {
        throw HTTPException(NOT_FOUND);
    }

    // NOTE: allow_methods 블록이 없을 경우 모든 메소드 허용?
    const LocationConfig& locConfig = it->second;
    if (std::find(locConfig.allow_methods.begin(), locConfig.allow_methods.end(), method) == locConfig.allow_methods.end())
    {
        throw HTTPException(METHOD_NOT_ALLOWED);
    }

    // Method에 따른 처리
    if (req.getRequestLine().getMethod() == "GET")
    {
        getRequest(req, res, serverConfig);
    }
    else if (req.getRequestLine().getMethod() == "HEAD")
    {
        headRequest(req, res, serverConfig);
    }
    else if (req.getRequestLine().getMethod() == "POST")
    {
        postRequest(req, res, serverConfig);
    }
    else if (req.getRequestLine().getMethod() == "DELETE")
    {
        deleteRequest(req, res, serverConfig);
    }
}
void RequestHandler::getRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig)
{
    (void)req;
    (void)res;
    (void)serverConfig;
    res.setStatusLine(req.getRequestLine().getHTTPVersion(), std::to_string(OK), "OK");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><body><h1>GET Request</h1></body></html>");
}
void RequestHandler::headRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig)
{
    (void)req;
    (void)res;
    (void)serverConfig;
    res.setStatusLine(req.getRequestLine().getHTTPVersion(), std::to_string(OK), "OK");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><body><h1>HEAD Request</h1></body></html>");
}
void RequestHandler::postRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig)
{
    (void)req;
    (void)res;
    (void)serverConfig;
    res.setStatusLine(req.getRequestLine().getHTTPVersion(), std::to_string(OK), "OK");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><body><h1>POST Request</h1></body></html>");
}
void RequestHandler::deleteRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig)
{
    (void)req;
    (void)res;
    (void)serverConfig;
    res.setStatusLine(req.getRequestLine().getHTTPVersion(), std::to_string(OK), "OK");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><body><h1>DELETE Request</h1></body></html>");
}
void RequestHandler::verifyRequestLine(const RequestLine& reqLine)
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
void RequestHandler::verifyRequestHeaderFields(const HeaderFields& reqHeaderFields)
{
    if (reqHeaderFields.hasField("Host") == false)
    {
        throw HTTPException(BAD_REQUEST);
    }
}
void RequestHandler::verifyRequest(const RequestMessage& req, const ServerConfig& serverConfig)
{
    (void)serverConfig;
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
void RequestHandler::handleException(const HTTPException& e, ResponseMessage& res)
{
    if (e.getStatusCode() == BAD_REQUEST)
    {
        badRequest(res);
    }
    else if (e.getStatusCode() == NOT_FOUND)
    {
        notFound(res);
    }
    else if (e.getStatusCode() == METHOD_NOT_ALLOWED)
    {
        methodNotAllowed(res);
    }
    else if (e.getStatusCode() == HTTP_VERSION_NOT_SUPPORTED)
    {
        httpVersionNotSupported(res);
    }
    else if (e.getStatusCode() == URI_TOO_LONG)
    {
        uriTooLong(res);
    }
}
void RequestHandler::badRequest(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", std::to_string(BAD_REQUEST), "Bad Request");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><head><title>400 Bad Request</title></head><body><h1>400 Bad Request</h1></body></html>");
}
void RequestHandler::notFound(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", std::to_string(NOT_FOUND), "Not Found");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>");
}
void RequestHandler::methodNotAllowed(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", std::to_string(METHOD_NOT_ALLOWED), "Method Not Allowed");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody(
        "<html><head><title>405 Method Not Allowed</title></head><body><h1>405 Method Not Allowed</h1></body></html>");
}
void RequestHandler::httpVersionNotSupported(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", std::to_string(HTTP_VERSION_NOT_SUPPORTED), "HTTP Version Not Supported");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><head><title>505 HTTP Version Not Supported</title></head><body><h1>505 HTTP Version Not Supported</h1></body></html>");
}
void RequestHandler::uriTooLong(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", std::to_string(URI_TOO_LONG), "Request-URI Too Long");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><head><title>414 Request-URI Too Long</title></head><body><h1>414 Request-URI Too Long</h1></body></html>");
}
