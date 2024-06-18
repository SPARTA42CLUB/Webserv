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

void RequestHandler::handleException(const HTTPException& e, ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", e.getStatusCode(), e.getReasonPhrase());
}
