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
#include "RequestHandler.hpp"
#include "EventManager.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class Server {
public:
	Server(const Config& config);
	~Server();

	void run();

private:
	const Config& config;
	EventManager eventManager;
	RequestHandler requestHandler;
	std::vector<int> serverSockets;
	std::vector<int> clientSockets;
	std::map<int, ServerConfig> socketToConfigMap;

	void setupServerSockets();
	void setNonBlocking(int socket);

	void acceptClient(int serverSocket);
	void handleClientReadEvent(struct kevent& event);

	void sendResponse(int socket, const HttpResponse& response);
	void closeConnection(int socket);
};
