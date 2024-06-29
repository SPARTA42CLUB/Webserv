#include "Connection.hpp"
#include <unistd.h>
#include <iostream>
#include <string>
#include "EventManager.hpp"
#include "Logger.hpp"

Connection::Connection(const int socket, const int parentSocket, std::string recvedData)
: socket(socket)
, parentSocket(parentSocket)
, childSocket()
, isChunked(false)
, recvedData(recvedData)
, chunkBuffer()
, chunkBodySize(0)
, requests()
, responses()
, last_activity(time(NULL))
, serverConfig()
{
    Logger::getInstance().logInfo(std::to_string(socket) + " Connection created\n");
    childSocket[READ_END] = -1;
    childSocket[WRITE_END] = -1;
}

Connection::~Connection()
{
    Logger::getInstance().logInfo(std::to_string(socket) + " Connection closed\n");
    close(socket);
    while (!requests.empty())
    {
        delete requests.front();
        requests.pop();
    }
    while (!responses.empty())
    {
        responses.pop();
    }
}

bool isCgiConnection(Connection& connection)
{
    if (connection.parentSocket != -1)
        return true;
    return false;
}

/*
ssize_t Connection::executeByRecv()
{
    char buffer[4096];
    ssize_t bytesRead;

    if ((bytesRead = recv(socket, buffer, sizeof(buffer) - 1, 0)) <= 0)
        return bytesRead;

    buffer[bytesRead] = '\0';
        recvedData += std::string(buffer, buffer + bytesRead);

    // 청크 인코딩 여부에 따라 청크 요청 처리
    if (isChunked)
    {
        std::string chunk;

        while ((chunk = getCompleteChunk()).length() > 0)
        {
            // NOTE: 헤더 보고 파일 경로 수정해야 함.
            ChunkedRequestReader reader("upload/testfile.png", chunk);
                bool isChunkedEnd = reader.processRequest();

            recvedData.erase(0, chunk.length());

            if (isChunkedEnd) {
                isChunked = false; // 마지막 청크 후 청크 상태 해제
                return ;
            }
        }
    }

    // 일반 요청 처리
    handleNormalRequest();

    updateLastActivity();
    return bytesRead;
}
*/

/*
// NOTE: 그냥 2 더하면 안되고 \r\n인지 검사해야 함.
std::string Connection::getCompleteChunk()
{
    size_t pos = 0;
    size_t chunkSizeEnd = recvedData.find("\r\n", pos);
    if (chunkSizeEnd == std::string::npos)
        return ""; // 청크 크기가 아직 도착하지 않음""

    size_t chunkSize = std::strtoul(recvedData.substr(pos, chunkSizeEnd - pos).c_str(), NULL, 16);
    pos = chunkSizeEnd + 2; // 청크 크기 끝을 지나서 데이터 시작

    if (pos + chunkSize + 2 > recvedData.size())
        return ""; // 청크 데이터가 아직 도착하지 않음
    return recvedData.substr(0, pos + chunkSize + 2);
}
*/

/*
// // NOTE: 여기는 RequestHandler 손보고 고쳐야 할듯..
void Connection::handleNormalRequest()
{
    // std::string requestString;
    // while ((requestString = getCompleteRequest()).length() > 0)
    // {
    //     recvedData.erase(0, requestString.length());

    //     RequestMessage reqMsg(requestString);
    //     RequestHandler requestHandler;
    //     requestHandler.handleRequest(reqMsg);
    // }

    // ResponseMessage* res = new ResponseMessage();
    // ResponseMessage& resMsg = *res;
    // RequestHandler requestHandler(resMsg); // NOTE: RequestHandler 수정돼야 함.

    // try
    // {



    //     if (reqMsg.getRequestHeaderFields().getField("Transfer-Encoding") == "chunked") {
    //         delete res;
    //         recvedData.erase(0, requestLength);
    //         isChunked = true;

    //         handleChunkedRequest();
    //         return ;
    //     }

    //     logger.logHttpMessage(*res, completeRequest);
    // }
    // catch (const HttpException& e)
    // {
    //     requestHandler.handleException(e);
    // }

    // responses.push_back(res);

    // recvedData.erase(0, requestLength);

    // // 리소스 효율을 위해 요청이 하나 완료되면 그 때 WRITE 이벤트 등록
    // if (!responses.empty()) {
    //     eventManager.addWriteEvent(socket);
    // }

}
*/

/*

*/

/*
// NOTE: 응답 생성할 때마다 keepalive 설정하게 변경해야 함
ssize_t Connection::sendToSocket()
{
    if (responses.empty())
        return 0;

    std::string data = responses.front()->toString();
    ssize_t bytesSend = send(socket, data.c_str(), data.length(), 0);

    delete responses.front();
    responses.pop_front();

    updateLastActivity();

    return bytesSend;
}
*/

/*
void Connection::updateLastActivity()
{
    last_activity = time(NULL);
}
*/

/*
time_t Connection::getLastActivity() const
{
    (void)config;
    return last_activity;
}
*/
