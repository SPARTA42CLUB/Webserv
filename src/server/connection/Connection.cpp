#include "Connection.hpp"
#include <unistd.h>
#include <string>
#include "EventManager.hpp"
#include "Logger.hpp"

Connection::Connection(const int socket, const ServerConfig& serverConfig, const int parentSocket, std::string buffer)
: socket(socket)
, parentSocket(parentSocket)
, childSocket()
, cgiPid(-1)
, isKeepAlive(true)
, isInChunkStream(false)
, isBodyReading(false)
, buffer(buffer)
, reqBuffer(NULL)
, request(NULL)
, responses()
, last_activity(time(NULL))
, serverConfig(serverConfig)
{
    Logger::getInstance().logInfo(std::to_string(socket) + " Connection created\n");
    childSocket[READ_END] = -1;
    childSocket[WRITE_END] = -1;
}

Connection::~Connection()
{
    Logger::getInstance().logInfo(std::to_string(socket) + " Connection closed\n");
    close(socket);
    if (!request)
        delete request;
    if (!reqBuffer)
        delete reqBuffer;
    while (!responses.empty())
    {
        delete responses.front();
        responses.pop();
    }
}

bool isCgiConnection(Connection& connection)
{
    if (connection.parentSocket != -1)
        return true;
    return false;
}

bool checkNeedClose(Connection& connection)
{
    if (connection.responses.size() > 1)
        return false;

    if (connection.isKeepAlive == false)
        return true;

    if (connection.responses.front()->isConnectionClose())
        return true;

    return false;
}
