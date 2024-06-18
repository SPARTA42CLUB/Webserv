#pragma once

#include <string>
#include "Config.hpp"
#include "HTTPException.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"

class RequestHandler
{
private:
    // Request
    void verifyRequestLine(const RequestLine& reqLine);
    void verifyRequestHeaderFields(const HeaderFields& reqHeaderFields);

    // Response (Success)
    void getRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig);
    void headRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig);
    void postRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig);
    void deleteRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig);

    // Response (Exception)
    void badRequest(ResponseMessage& res);
    void notFound(ResponseMessage& res);
    void methodNotAllowed(ResponseMessage& res);
    void httpVersionNotSupported(ResponseMessage& res);
    void uriTooLong(ResponseMessage& res);

public:
    void verifyRequest(const RequestMessage& req, const ServerConfig& serverConfig);
    void handleRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig);
    void handleException(const HTTPException& e, ResponseMessage& res);
};
