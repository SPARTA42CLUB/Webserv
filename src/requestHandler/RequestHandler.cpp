#include "RequestHandler.hpp"

void RequestHandler::handleRequest(const HttpRequest& request, HttpResponse response, const ServerConfig& serverConfig) {

	std::map<std::string, LocationConfig>::const_iterator it = serverConfig.locations.find(request.getPath());
	if (it != serverConfig.locations.end()) {
		response.setStatusCode(404, "OK");
		response.setHeader("Content-Type", "text/plain");
		response.setBody("Not Found");

		return ;
	}

	const LocationConfig& locConfig = it->second;

	if (std::find(locConfig.allow_methods.begin(), locConfig.allow_methods.end(), request.getMethod()) == locConfig.allow_methods.end()) {
		response.setStatusCode(405, "test");
		response.setHeader("Content-Type", "text/plain");
		response.setBody("Method Not Allowed");

		return ;
	}

	response.setStatusCode(200, "OK");
	response.setHeader("Content-Type", "text/plain");
	response.setBody("OK");
}

void RequestHandler::badRequest(HttpResponse response, std::string body) {

	response.setStatusCode(400, "NO");
	response.setHeader("Content-Type", "text/plain");
	response.setBody("Bad Request: " + body);
}

