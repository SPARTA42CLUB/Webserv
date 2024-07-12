#pragma once

#include <queue>
#include "ResponseMessage.hpp"
#include "RequestMessage.hpp"
#include "Config.hpp"

struct Connection
{
    const int fd;
    const int parentFd;
    int childFd[2];
    pid_t cgiPid;
    bool isKeepAlive;
    bool isInChunkStream;
    bool isBodyReading;
    std::string buffer;
    RequestMessage* reqBuffer;
    RequestMessage* request;
    std::queue<ResponseMessage*> responses;
    time_t last_activity;
    const ServerConfig& serverConfig;

    Connection(const int fd, const ServerConfig& serverConfig, const int parentFd = -1, std::string buffer = "");
    ~Connection();
};

bool checkNeedClose(const Connection& connection);
