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
    // MethodExecuter methodExecuter; // 얘가 분배 이후 메소드를 실제로 실행하는 함수를 모두 지님
    // CGIHandler cgiHandler;
    std::string mPath;

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
    void handleRequest(const RequestMessage& req);
    void handleException(const HTTPException& e);
};
