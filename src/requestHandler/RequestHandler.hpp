#pragma once

#include <string>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Config.hpp"

class RequestHandler {
public:
	void handleRequest(const HttpRequest& request, HttpResponse response, const ServerConfig& serverConfig);
	void badRequest(HttpResponse response, std::string body);

private:
};
