#pragma once

#include <string>
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "Config.hpp"

class RequestHandler {
public:
	void handleRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig);
	void badRequest(ResponseMessage& res, std::string body);

private:
};
