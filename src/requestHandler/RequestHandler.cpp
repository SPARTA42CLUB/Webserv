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


    res.setStatusLine("HTTP/1.1", "200", "OK");
    res.addResponseHeaderField("Content-Type", "text/plain");
    res.addMessageBody("OK");

}

void RequestHandler::badRequest(ResponseMessage& res, std::string body) {

    res.setStatusLine("HTTP/1.1", "400", "NO");
    res.addResponseHeaderField("Content-Type", "text/plain");
    res.addMessageBody("Bad Request: " + body);
}

