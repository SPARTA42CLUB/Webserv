#pragma once

#include <string>
#include "Config.hpp"
#include "HTTPException.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"

class RequestHandler
{
private:
    ResponseMessage& mResponseMessage;
    const ServerConfig& mServerConfig;
    std::string mLocation;
    std::string mPath;

    // Request
    void verifyRequestLine(const RequestLine& reqLine);
    void verifyRequestHeaderFields(const HeaderFields& reqHeaderFields);

    // Response (Success)
    void getRequest(const RequestMessage& req);
	void rangeRequest(const RequestMessage& req);
    void headRequest(const RequestMessage& req);
    void postRequest(const RequestMessage& req);
    void deleteRequest(const RequestMessage& req);
    void addSemanticHeaderFields(ResponseMessage& res);
    void addContentType(ResponseMessage& res);

    // Response (Exception)
    // 3xx
    void found(void); // Require 'Location' header field
    // 4xx
    void badRequest(void);
    void forbidden(void);
    void notFound(void);
    void methodNotAllowed(void);
    void uriTooLong(void);
    // 5xx
    void httpVersionNotSupported(void);


public:
    RequestHandler(ResponseMessage& res, const ServerConfig& serverConfig);
    void verifyRequest(const RequestMessage& req);
    void handleRequest(const RequestMessage& req);
    void handleException(const HTTPException& e);
};
