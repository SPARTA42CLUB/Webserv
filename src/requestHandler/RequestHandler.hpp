#pragma once

#include <string>
#include "Config.hpp"
#include "HTTPException.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "Connection.hpp"

class RequestHandler
{
private:
    Connection& mConnection;
    const RequestMessage* mRequestMessage;
    ResponseMessage* mResponseMessage; // 동적할당 해서 반환
    const ServerConfig& mServerConfig;
    // MethodExecuter methodExecuter; // 얘가 분배 이후 메소드를 실제로 실행하는 함수를 모두 지님
    // CGIHandler cgiHandler;
    std::string mPath;

    // Response (Success)
    void getRequest(void);
	void rangeRequest(void);
    void headRequest(void);
    void postRequest(void);
    void deleteRequest(void);
    void addSemanticHeaderFields(void);
    void addContentType(void);

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

    bool checkStatusCode(const int statusCode);

public:
    RequestHandler(Connection& connection, const Config& config);
    ResponseMessage* handleRequest(void);
};
