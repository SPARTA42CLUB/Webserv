#pragma once

#include <deque>
#include <string>

#include "Logger.hpp"
#include "ResponseMessage.hpp"
#include "RequestMessage.hpp"

#define DEFAULT_KEEPALIVE_TIME 60

class Connection {
private:
	int socket;
	int chunkedFd;
	int CGIPipeFd;
	bool isChunked;
	bool isCGI;
	std::string recvedData;
	std::deque<ResponseMessage*> responses;
	time_t last_activity;
	int keepalive_time;
	Logger& logger;

public:
	Connection(int socket);
	~Connection();

	ssize_t recvToSocket();
	ssize_t sendToSocket();

	void handleChunkedRequest();
	std::string getCompleteChunk();
	void handleNormalRequest();
	std::string getCompleteRequest();

	void update_last_activity();
	bool isKeepAlive();
};
