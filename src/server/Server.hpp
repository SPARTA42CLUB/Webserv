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
#include "Connection.hpp"
#include "Logger.hpp"

class Server {
public:
	Server(const Config& config);
	~Server();

	void run();

private:
	const Config& config;
	EventManager eventManager;
	std::vector<int> serverSockets;
    std::map<int, Connection*> connectionsMap;
    std::map<int, int> pipeToSocketMap;
	Logger& logger;

	void setupServerSockets();
	int createServerSocket(ServerConfig serverConfig);
	void setNonBlocking(int socket);
	void checkKeepAlive();

	void acceptClient(int serverSocket);

	void handleClientReadEvent(struct kevent& event);
	void handleClientWriteEvent(struct kevent& event);

	void closeConnection(int socket);

	bool isServerSocket(int socket);
};
