#include "RequestHandler.hpp"

void RequestHandler::handleRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig) {

	// std::map<std::string, LocationConfig>::const_iterator it = serverConfig.locations.find(request.getPath());
	// if (it != serverConfig.locations.end()) {
	// 	response.setStatusCode(404, "OK");
	// 	response.setHeader("Content-Type", "text/plain");
	// 	response.setBody("Not Found");

	// 	return ;
	// }

	// const LocationConfig& locConfig = it->second;

	// if (std::find(locConfig.allow_methods.begin(), locConfig.allow_methods.end(), request.getMethod()) == locConfig.allow_methods.end()) {
	// 	response.setStatusCode(405, "test");
	// 	response.setHeader("Content-Type", "text/plain");
	// 	response.setBody("Method Not Allowed");

	// 	return ;
	// }
	(void)req;
	(void)serverConfig;

    res.setStatusLine("HTTP/1.1", "200", "OK");
    res.addResponseHeaderField("Content-Type", "text/plain");
    res.addResponseHeaderField("Content-Length", "2");
    res.addMessageBody("OK");
	// res.setStatusCode(200, "OK");
	// res.setHeader("Content-Type", "text/plain");
	// res.setBody("OK");
}

void RequestHandler::badRequest(ResponseMessage& res, std::string body) {

    res.setStatusLine("HTTP/1.1", "400", "NO");
    res.addResponseHeaderField("Content-Type", "text/plain");
    res.addMessageBody("Bad Request: " + body);
	// response.setStatusCode(400, "NO");
	// response.setHeader("Content-Type", "text/plain");
	// response.setBody("Bad Request: " + body);
}

