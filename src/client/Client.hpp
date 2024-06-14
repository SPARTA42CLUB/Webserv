#pragma once

#include <string>
#include <netinet/in.h>
#include "Config.hpp"

class Client {
public:
	Client(int fd, const struct sockaddr_in& addr, const ServerConfig& serverConfig);
	~Client();

	int getFd() const;
	std::string getIpAddr() const;
	int getPort() const;
	const ServerConfig& getServerConfig() const;

private:
	int mFd;
	struct sockaddr_in mAddr;
	std::string mIpAddr;
	int mPort;
	const ServerConfig& mServerConfig;
};
