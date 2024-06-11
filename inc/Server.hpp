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

class Server {
public:
	Server(const std::vector<ServerConfig>& serverConfigs);
	void run();

private:
	std::vector<int> serverSockets;
	std::vector<ServerConfig> serverConfigs;
	int kq;

	void setupServerSockets();
	void setNonBlocking(int fd);
	void handleEvents();
	void handleClient(int clientSocket);
	void sendResponse(int clientSocket, const std::string& response);
	void closeConnection(int clientSocket);
};
