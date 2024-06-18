#include "Server.hpp"
#include "error.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "HTTPException.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <cerrno>
#include <cstdlib>
#include <arpa/inet.h>


// ServerConfig 클래스 생성자
Server::Server(const Config& config) : config(config), eventManager() {

	// Server 소켓 생성 및 설정
	setupServerSockets();
}

Server::~Server() {
	for (size_t i = 0; i < serverSockets.size(); ++i) {
		close(serverSockets[i]);
	}

	for (size_t i = 0; i < clientSockets.size(); ++i)
		close(clientSockets[i]);
}

// Server 소켓 생성 및 설정
void Server::setupServerSockets() {

	/*
	 * 다중 서버 소켓을 관리하는 이유 : 하나의 서버 소켓이 하나의 port에 대한 연결 요청을 처리한다.
	 * 예시로 443(https)포트와 8080(http)포트를 동시에 사용하는 경우, 서버 소켓을 2개 생성하여 각각의 포트에 대한 연결 요청을 처리한다.
	 * 또한 서버는 여러개의 IP를 가질 수 있으므로, 서버 소켓을 여러개 생성하여 각각의 IP에 대한 연결 요청을 처리할 수 있다.
	 * 서버가 여러개의 IP를 가지는 경우는 서버가 여러개의 네트워크 인터페이스를 가지는 경우이다.
	*/
	std::vector<ServerConfig> serverConfigs = config.getServerConfigs();

	for (size_t i = 0; i < serverConfigs.size(); ++i) {
		// 소켓 생성
		int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (serverSocket == -1)
			throw std::runtime_error("Failed to create socket");

		{
			// 개발 편의용 세팅. 서버 소켓이 이미 사용중이더라도 실행되게끔 설정
			int optval = 1;
			setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
		}

		// 소켓 옵션 설정
		setNonBlocking(serverSocket);

		// 서버 주소 설정
		struct sockaddr_in serverAddr;
		memset(&serverAddr, 0, sizeof(serverAddr)); // 0으로 초기화
		serverAddr.sin_family = AF_INET; // IPv4
		serverAddr.sin_port = htons(serverConfigs[i].port); // 포트 설정
		inet_pton(AF_INET, serverConfigs[i].host.c_str(), &serverAddr.sin_addr); // IP 주소 설정

		// 소켓 바인딩
		if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
			exit_with_error_close("Failed to bind socket", serverSocket);

		// 소켓 리스닝
		if (listen(serverSocket, 10) == -1)
			exit_with_error_close("Failed to listen on socket", serverSocket);

		// 서버 소켓 벡터에 추가
		serverSockets.push_back(serverSocket);
		socketToConfigMap[serverSocket] = serverConfigs[i];

		eventManager.addEvent(serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE);
	}
}

// 소켓 논블로킹 설정
void Server::setNonBlocking(int socket) {

	// 파일 디스크립터 플래그 가져오기
	int flags = fcntl(socket, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("Failed to get flags");

	// 논블로킹 설정
	if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set non-blocking");
}

// 서버 실행
void Server::run() {
	// 이벤트 처리 루프
	while (true) {
		std::vector<struct kevent> events = eventManager.getCurrentEvents();
		for (std::vector<struct kevent>::iterator it = events.begin(); it != events.end(); ++it) {
			struct kevent& event = *it;

			if (event.flags & EV_ERROR) {
				std::cerr << "Event error: " << strerror(event.data) << std::endl;
				close(event.ident);
				continue;
			}

			if (event.filter == EVFILT_READ) {
				if (std::find(serverSockets.begin(), serverSockets.end(), event.ident) != serverSockets.end())
					acceptClient(event.ident);
				else
					handleClientReadEvent(event);
			}
		}
		checkTimeout();
	}
}

void Server::checkTimeout() {
	time_t now = time(NULL);

	for (size_t i = 0; i < clientSockets.size(); ++i) {
		int clientSocket = clientSockets[i];
		int timeout = socketToConfigMap[clientSocket].keepalive_timeout;

		if (difftime(now, last_activity_map[clientSocket]) > timeout) {
			std::cout << "Timeout. Connection closed: " << clientSocket << std::endl;
			closeConnection(clientSocket);
		}
	}
}

void Server::update_last_activity(int socket) {
	last_activity_map[socket] = time(NULL);
}

void Server::acceptClient(int serverSocket) {

    // 서버 소켓으로 읽기 이벤트가 발생했다는 것의 의미 : 새로운 클라이언트가 연결 요청을 보냈다는 것
    // 서버 소켓인 경우 'accept'를 이용하여 클라이언트의 연결 수락
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

	int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
	if (clientSocket == -1) {
		std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
		return ;
	}

	setNonBlocking(clientSocket);
	update_last_activity(clientSocket);

	try {
		eventManager.addEvent(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE);
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		close(clientSocket);
		return ;
	}

	clientSockets.push_back(clientSocket);
	socketToConfigMap[clientSocket] = socketToConfigMap[serverSocket];
}

// 클라이언트 요청 처리
void Server::handleClientReadEvent(struct kevent& event) {

	if (event.flags & EV_EOF) {
		closeConnection(event.ident);
		return ;
	}

	update_last_activity(event.ident);

	// 클라이언트 요청 수신
	char buffer[4096];
	ssize_t bytesRead;
	std::string requestData;

	if ((bytesRead = recv(event.ident, buffer, sizeof(buffer) - 1, 0)) < 0) {
		std::cerr << "recv error" << std::endl;
		closeConnection(event.ident);
		return ;
	}

	buffer[bytesRead] = '\0';
	requestData += buffer;

	RequestMessage req_msg(requestData);
	ResponseMessage resMsg;
	try {
        requestHandler.verifyRequest(RequestMessage(requestData), socketToConfigMap[event.ident]);
		requestHandler.handleRequest(req_msg, resMsg, socketToConfigMap[event.ident]);
	} catch (const HTTPException& e) {
		requestHandler.handleException(e, resMsg);
	}
    if (!shouldKeepAlive(req_msg))
		closeConnection(event.ident);
    logHTTPMessage(event.ident, resMsg, requestData);
    sendResponse(event.ident, resMsg);
}

void Server::logHTTPMessage(int socket, ResponseMessage& res, const std::string& reqData) 
{
    std::ofstream logFile("access.log", std::ios::app);

    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);

    logFile << asctime(timeinfo)
    << "Client IP: " << socketToConfigMap[socket].host << '\n'
    << "Request:\n" << reqData
    << "Response:\n" << res.toString() 
    << "\n----------------------------------------------------------------------------------------\n" << std::endl;
    logFile.close();
}

void Server::sendResponse(int socket, ResponseMessage& res)
{
	std::string responseStr = res.toString();
	ssize_t bytesSent = send(socket, responseStr.c_str(), responseStr.length(), 0);
	if (bytesSent == -1) {
		std::cerr << "send error: " << strerror(errno) << std::endl;
		closeConnection(socket);
	}
}

bool Server::shouldKeepAlive(const RequestMessage& req) {
	if (req.getRequestHeaderFields().getField("Connection") == "close")
		return false;
	return true;
}

// 클라이언트 소켓 종료
void Server::closeConnection(int socket) {
	close(socket);

	struct kevent event;
	EV_SET(&event, socket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	kevent(eventManager.getKqueue(), &event, 1, NULL, 0, NULL);

	std::vector<int>::iterator it = std::find(clientSockets.begin(), clientSockets.end(), socket);
	if (it != clientSockets.end())
		clientSockets.erase(it);

	socketToConfigMap.erase(socket);
	last_activity_map.erase(socket);

}
