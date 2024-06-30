#pragma once

#include <queue>
#include "ResponseMessage.hpp"
#include "RequestMessage.hpp"
#include "Config.hpp"

struct Connection
{
    const int socket;
    int parentSocket;
    int childSocket[2];
    bool isChunked;
    bool isBodyReading;
    std::string recvedData;
    RequestMessage* reqBuffer;
    RequestMessage* request;
    std::queue<ResponseMessage*> responses;
    time_t last_activity;
    const ServerConfig& serverConfig;

    Connection(const int socket, const ServerConfig& serverConfig, const int parentSocket = -1, std::string recvedData = "");
    ~Connection();
};

bool isCgiConnection(Connection& connection);
