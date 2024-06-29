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
    ResponseMessage mResponseMessage; // 동적할당 해서 반환
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

    void addConnectionHeader();

public:
    RequestHandler(std::map<int, Connection*>& connectionsMap, const Config& config, const int socket);
    std::string handleRequest(void);
};
