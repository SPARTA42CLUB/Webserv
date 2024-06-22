#pragma once

#include <vector>
#include <string>

#include "Config.hpp"
#include "ResponseMessage.hpp"
#include "RequestMessage.hpp"

class Connection {
private:
	int fd;
	ServerConfig serverConfig;
	time_t last_activity;
	std::string recvData;
	std::vector<ResponseMessage*> responses;
	bool isChunked;

	RequestMessage lastRequest;

public:
	Connection(int serverSocket);
	~Connection();
	int getFd();
	void update_last_activity();
};
