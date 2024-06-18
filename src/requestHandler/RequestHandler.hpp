#pragma once

#include <string>
#include "Config.hpp"
#include "HTTPException.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"

class RequestHandler
{
private:
    void verifyRequestLine(const RequestLine& reqLine);
    void verifyRequestHeaderFields(const HeaderFields& reqHeaderFields);
    void badRequest(ResponseMessage& res);
    void notFound(ResponseMessage& res);
    void methodNotAllowed(ResponseMessage& res);
    void httpVersionNotSupported(ResponseMessage& res);

public:
    void verifyRequest(const RequestMessage& req, const ServerConfig& serverConfig);
    void handleRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig);
    void handleException(const HTTPException& e, ResponseMessage& res);
};
