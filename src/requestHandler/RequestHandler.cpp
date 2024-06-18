#include "RequestHandler.hpp"

void RequestHandler::verifyRequestLine(const RequestLine& reqLine)
{
    const std::string method = reqLine.getMethod();
    const std::string req_url = reqLine.getRequestURL();
    const std::string ver = reqLine.getHTTPVersion();
    if (method != "GET" && method != "POST" && method != "DELETE" && method != "PUT")
    {
        throw HTTPException(METHOD_NOT_ALLOWED, "Method Not Allowed");
    }
    if (ver != "HTTP/1.1")
    {
        throw HTTPException(HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported");
    }
    if (req_url[0] != '/')
    {
        throw HTTPException(NOT_FOUND, "Not Found");
    }
}
void RequestHandler::verifyRequestHeaderFields(
    const HeaderFields& reqHeaderFields)
{
    if (reqHeaderFields.hasField("Host") == false)
    {
        throw HTTPException(BAD_REQUEST, "Bad Request");
    }
}
void RequestHandler::verifyRequest(const RequestMessage& req, const ServerConfig& serverConfig)
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

    (void)serverConfig;
}
void RequestHandler::handleRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig)
{
    // std::map<std::string, LocationConfig>::const_iterator it =
    // serverConfig.locations.find(request.getPath()); if (it !=
    // serverConfig.locations.end()) { 	response.setStatusCode(404, "OK");
    // 	response.setHeader("Content-Type", "text/plain");
    // 	response.setBody("Not Found");

    // 	return ;
    // }

    // const LocationConfig& locConfig = it->second;

    // if (std::find(locConfig.allow_methods.begin(),
    // locConfig.allow_methods.end(), request.getMethod()) ==
    // locConfig.allow_methods.end()) { 	response.setStatusCode(405, "test");
    // 	response.setHeader("Content-Type", "text/plain");
    // 	response.setBody("Method Not Allowed");

    // 	return ;
    // }
    (void)req;
    (void)serverConfig;

    res.setStatusLine(
        req.getRequestLine().getHTTPVersion(), std::to_string(OK), "OK");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><body><h1>Request Received</h1></body></html>");
}

void RequestHandler::handleException(const HTTPException& e, ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", e.getStatusCode(), e.getReasonPhrase());
}
