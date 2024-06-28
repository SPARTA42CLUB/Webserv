#pragma once

#include <string>
#include "Config.hpp"
#include "HttpException.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "Connection.hpp"

class RequestHandler
{
private:
    std::map<int, Connection*>& mConnectionsMap;
    const int mSocket;
    const RequestMessage* mRequestMessage;
    ResponseMessage* mResponseMessage; // 동적할당 해서 반환
    const ServerConfig& mServerConfig;
    LocationConfig mLocConfig;
    std::string mPath;
    std::string mQueryString;

    int setPath(void);

    int handleMethod(void);
    // Response (Success)
    int getRequest(void);
	int rangeRequest(void);
    int headRequest(void);
    int postRequest(void);
    int deleteRequest(void);

    bool checkCGI(void);
    void executeCGI(void);

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

    void addConnectionHeader();

public:
    RequestHandler(std::map<int, Connection*>& connectionsMap, const Config& config, const int socket);
    ResponseMessage* handleRequest(void);
};
