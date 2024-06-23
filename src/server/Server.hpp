#pragma once

#include <vector>
#include <map>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/event.h>
#include "Config.hpp"
#include "EventManager.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "Connection.hpp"

struct Connection
{
    int socket;
    int chunkedFD;
    bool isCGI;
    std::string recvDataMap;
    std::vector<ResponseMessage*> responsesMap;
    time_t last_activity;

    Connection()
    : socket(-1)
    , chunkedFD(-1)
    , isCGI(false)
    , recvDataMap()
    , responsesMap()
    , last_activity(0)
    {
    }
};

class Server {
public:
	Server(const Config& config);
	~Server();

	void run();

private:
	const Config& config;
	EventManager eventManager;
	std::vector<int> serverSockets;
	std::vector<int> clientSockets;
    std::map<int, Connection> connectionsMap;
    std::map<int, int> pipeToSocketMap;

	void setupServerSockets();
	int createServerSocket(ServerConfig serverConfig);
	void setNonBlocking(int socket);

	void acceptClient(int serverSocket);
	void handleClientReadEvent(struct kevent& event);
	bool isCompleteRequest(const std::string& data, size_t& requestLength);
	bool isCompleteChunk(const std::string& data, size_t& chunkLength, bool& isLastChunk);

	void handleChunkedRequest(int socket, std::string& requestData);
	void handleNormalRequest(int socket, std::string& requestData, size_t requestLength);

	void handleClientWriteEvent(struct kevent& event);

    void logHTTPMessage(int socket, const ResponseMessage& res, const std::string& reqData);
	void sendResponse(int socket, ResponseMessage& res);
	void closeConnection(Connection& connection);

	void update_last_activity(int socket);
	void checkTimeout();
};
