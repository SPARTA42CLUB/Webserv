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

class Server {
public:
	Server(const Config& config);
	~Server();

	void run();

private:
	const Config& config;
	EventManager eventManager;
	std::vector<int> serverSockets;
	std::map<int, Connection> connections;

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
