#pragma once

#include <string>
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "HTTPException.hpp"
#include "Config.hpp"

class RequestHandler {
private:
    void verifyRequestLine(const RequestLine& reqLine);
    void verifyRequestHeaderFields(const HeaderFields& reqHeaderFields);

public:
    void verifyRequest(const RequestMessage& req, const ServerConfig& serverConfig);
	void handleRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig);
    void handleException(const HTTPException& e, ResponseMessage& res);
};
