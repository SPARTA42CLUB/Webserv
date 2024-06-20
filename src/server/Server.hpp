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
	std::map<int, ServerConfig> socketToConfigMap;
	std::map<int, time_t> last_activity_map;
	std::map<int, std::string> recvDataMap;
	std::map<int, std::vector<std::string> > completeDataMap;

	void setupServerSockets();
	void setNonBlocking(int socket);

	void acceptClient(int serverSocket);
	void handleClientReadEvent(struct kevent& event);
	void handleClientWriteEvent(struct kevent& event);

	bool isCompleteRequest(const std::string& data, size_t& requestLength);
	bool handleRequest(int socket, const std::string& requestData);

    void logHTTPMessage(int socket, ResponseMessage& res, const std::string& reqData);
	void sendResponse(int socket, ResponseMessage& res);
	void closeConnection(int socket);

	void update_last_activity(int socket);
	void checkTimeout();

	bool shouldKeepAlive(const RequestMessage& req);
};
