#pragma once

#include <queue>
#include "ResponseMessage.hpp"
#include "RequestMessage.hpp"

struct Connection
{
    const int socket;
    bool isChunked;
    int parentSocket;
    int childSocket;
    std::string recvedData;
    std::string chunkBuffer;
    std::queue<RequestMessage*> requests;
    std::queue<ResponseMessage*> responses;
    time_t last_activity;

    Connection(const int socket);
    ~Connection();
};
