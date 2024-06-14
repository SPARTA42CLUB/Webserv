#include "Client.hpp"
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

Client::Client(int fd, const struct sockaddr_in& addr, const ServerConfig& serverConfig)
	: mFd(fd), mAddr(addr), mServerConfig(serverConfig) {
	mIpAddr = inet_ntoa(mAddr.sin_addr);
	mPort = ntohs(mAddr.sin_port);
}

Client::~Client() {
	close(mFd);
}

int Client::getFd() const {
	return mFd;
}

std::string Client::getIpAddr() const {
	return mIpAddr;
}

int Client::getPort() const {
	return mPort;
}

const ServerConfig& Client::getServerConfig() const {
	return mServerConfig;
}
