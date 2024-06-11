#include "Server.hpp"
#include "error.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <arpa/inet.h>

Server::Server(const std::vector<ServerConfig>& serverConfigs) : serverConfigs(serverConfigs) {

	setupServerSockets();
	kq = kqueue();

	if (kq == -1)
		exit_with_error("Failed to create kqueue");

	for (size_t i = 0; i < serverSockets.size(); ++i) {

		struct kevent event;
		EV_SET(&event, serverSockets[i], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

		if (kevent(kq, &event, 1, NULL, 0, NULL) == -1)
			exit_with_error("Failed to add event to kqueue");
	}
}

void Server::setupServerSockets() {

	for (size_t i = 0; i < serverConfigs.size(); ++i) {

		int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (serverSocket == -1)
			exit_with_error("Failed to create socket");

		setNonBlocking(serverSocket);

		struct sockaddr_in serverAddr;
		memset(&serverAddr, 0, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(serverConfigs[i].port);
		inet_pton(AF_INET, serverConfigs[i].host.c_str(), &serverAddr.sin_addr);

		if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
			exit_with_error_close("Failed to bind socket", serverSocket);

		if (listen(serverSocket, 10) == -1)
			exit_with_error_close("Failed to listen on socket", serverSocket);

		serverSockets.push_back(serverSocket);
	}
}

void Server::setNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << "Failed to get flags: " << strerror(errno) << std::endl;
		std::exit(EXIT_FAILURE);
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::cerr << "Failed to set non-blocking: " << strerror(errno) << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

void Server::run() {
	while (true) {
		handleEvents();
	}
}

void Server::handleEvents() {
	struct kevent events[1024];
	int numEvents = kevent(kq, NULL, 0, events, 1024, NULL);
	if (numEvents == -1) {
		std::cerr << "kevent error: " << strerror(errno) << std::endl;
		return;
	}

	for (int i = 0; i < numEvents; ++i) {
		if (events[i].flags & EV_ERROR) {
			std::cerr << "Event error: " << strerror(events[i].data) << std::endl;
			closeConnection(events[i].ident);
			continue;
		}

		if (events[i].filter == EVFILT_READ) {
			if (std::find(serverSockets.begin(), serverSockets.end(), events[i].ident) != serverSockets.end()) {
				// New connection
				int clientSocket = accept(events[i].ident, NULL, NULL);
				if (clientSocket == -1) {
					std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
					continue;
				}
				setNonBlocking(clientSocket);
				struct kevent event;
				EV_SET(&event, clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
				if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
					std::cerr << "Failed to add event to kqueue: " << strerror(errno) << std::endl;
					close(clientSocket);
				}
			} else {
				// Handle client request
				handleClient(events[i].ident);
			}
		} else if (events[i].filter == EVFILT_WRITE) {
			// Handle write event if needed
		}
	}
}

void Server::handleClient(int clientSocket) {
	char buffer[4096];
	ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead <= 0) {
		if (bytesRead == 0) {
			std::cerr << "Client disconnected" << std::endl;
		} else {
			std::cerr << "recv error: " << strerror(errno) << std::endl;
		}
		closeConnection(clientSocket);
		return;
	}

	buffer[bytesRead] = '\0';
	std::cout << "Received request: " << buffer << std::endl;

	// Handle request and generate response
	std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
	sendResponse(clientSocket, response);
}

void Server::sendResponse(int clientSocket, const std::string& response) {
	ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
	if (bytesSent == -1) {
		std::cerr << "send error: " << strerror(errno) << std::endl;
		closeConnection(clientSocket);
	}
}

void Server::closeConnection(int clientSocket) {
	close(clientSocket);
	struct kevent event;
	EV_SET(&event, clientSocket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	kevent(kq, &event, 1, NULL, 0, NULL);
}
