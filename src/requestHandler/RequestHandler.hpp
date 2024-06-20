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
    std::string mPath;

    // Request
    void verifyRequestLine(const RequestLine& reqLine);
    void verifyRequestHeaderFields(const HeaderFields& reqHeaderFields);

    // Response (Success)
    void getRequest(const RequestMessage& req);
    void headRequest(const RequestMessage& req);
    void postRequest(const RequestMessage& req);
    void deleteRequest(const RequestMessage& req);
    void addSemanticHeaderFields(ResponseMessage& res);
    void addContentType(ResponseMessage& res);

    // Response (Exception)
    void badRequest(void);
    void notFound(void);
    void methodNotAllowed(void);
    void httpVersionNotSupported(void);
    void uriTooLong(void);
    void forbidden(void);


public:
    RequestHandler(ResponseMessage& res, const ServerConfig& serverConfig);
    void verifyRequest(const RequestMessage& req);
    void handleRequest(const RequestMessage& req);
    void handleException(const HTTPException& e);
    bool isKeepAlive(void) const;
};
