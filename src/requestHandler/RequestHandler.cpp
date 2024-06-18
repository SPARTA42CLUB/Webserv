#include "RequestHandler.hpp"

void RequestHandler::handleRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig) {

	std::string url = req.getRequestLine().getRequestURL();
	std::string method = req.getRequestLine().getMethod();

	std::map<std::string, LocationConfig>::const_iterator it = serverConfig.locations.find(url);
	if (it == serverConfig.locations.end()) {
		res.setStatusLine("HTTP/1.1", "404", "NO");
		res.addResponseHeaderField("Content-Type", "text/plain");
		res.addMessageBody("Not Found");

		return ;
	}

	const LocationConfig& locConfig = it->second;

	if (std::find(locConfig.allow_methods.begin(), locConfig.allow_methods.end(), method) == locConfig.allow_methods.end()) {
		res.setStatusLine("HTTP/1.1", "405", "NO");
		res.addResponseHeaderField("Content-Type", "text/plain");
		res.addMessageBody("Method Not Allowed");

		return ;
	}

    res.setStatusLine(
        req.getRequestLine().getHTTPVersion(), std::to_string(OK), "OK");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><body><h1>Request Received</h1></body></html>");
}

void RequestHandler::verifyRequestLine(const RequestLine& reqLine)
{
    const std::string method = reqLine.getMethod();
    const std::string req_tg = reqLine.getRequestTarget();
    const std::string ver = reqLine.getHTTPVersion();
    if (method != "GET" && method != "POST" && method != "DELETE" && method != "PUT")
    {
        throw HTTPException(METHOD_NOT_ALLOWED);
    }
    if (ver != "HTTP/1.1")
    {
        throw HTTPException(HTTP_VERSION_NOT_SUPPORTED);
    }
    if (req_tg[0] != '/')
    {
        throw HTTPException(NOT_FOUND);
    }
    if (req_tg.size() >= 8200) // nginx max uri length
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
        res.setStatusLine("HTTP/1.1", std::to_string(URI_TOO_LONG), "Request-URI Too Long");
        res.addResponseHeaderField("Content-Type", "text/html");
        res.addMessageBody("<html><head><title>414 Request-URI Too Long</title></head><body><h1>414 Request-URI Too Long</h1></body></html>");
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
    res.addMessageBody("<html><head><title>405 Method Not Allowed</title></head><body><h1>405 Method Not Allowed</h1></body></html>");
}
void RequestHandler::httpVersionNotSupported(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", std::to_string(HTTP_VERSION_NOT_SUPPORTED), "HTTP Version Not Supported");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><head><title>505 HTTP Version Not Supported</title></head><body><h1>505 HTTP Version Not Supported</h1></body></html>");
}
