#pragma once

#include <queue>
#include "ResponseMessage.hpp"
#include "RequestMessage.hpp"

struct Connection
{
    const int socket;
    bool isChunked;
    const Connection* parentConnection;  // cgi의 응답을 담은 pipe를 생성한 connection에 대한 참조
    std::string recvedData;
    std::string chunkBuffer;
    std::queue<RequestMessage*> requests;
    std::queue<ResponseMessage*> responses;
    time_t last_activity;

    Connection(const int socket, const Connection* parentConnection = nullptr);
    ~Connection();
};
