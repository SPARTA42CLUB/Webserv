#pragma once

#include <queue>
#include "ResponseMessage.hpp"
#include "RequestMessage.hpp"

struct Connection
{
    const int socket;
    int parentSocket;
    int childSocket;
    bool isChunked;
    std::string recvedData;
    std::string chunkBuffer;
    std::queue<RequestMessage*> requests;
    std::queue<ResponseMessage*> responses;
    time_t last_activity;

    Connection(const int socket, const int parentSocket = -1, const int childSocket = -1);
    ~Connection();
};
