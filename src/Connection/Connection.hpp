#pragma once

#include <deque>
#include <string>

#include "Logger.hpp"
#include "ResponseMessage.hpp"
#include "RequestMessage.hpp"
#include "Config.hpp"

class Connection {
private:
	int socket;
	int chunkedFd;
	int CGIPipeFd; // NOTE: 2개 필요할 수도?
	bool isChunked;
	bool isCGI;
	std::string recvedData;
	std::deque<ResponseMessage*> responses;
	time_t last_activity;
	Logger& logger;
	const Config& config;

	void handleChunkedRequest();
	std::string getCompleteChunk();
	void handleNormalRequest();
	std::string getCompleteRequest();

	void updateLastActivity();

public:
	Connection(int socket, const Config& config);
	~Connection();

	ssize_t excuteByRecv();
	ssize_t sendToSocket();

	time_t getLastActivity() const;
};
