#include "Server.hpp"
#include "error.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <arpa/inet.h>

// ServerConfig 클래스 생성자
Server::Server(const std::vector<ServerConfig>& serverConfigs) : serverConfigs(serverConfigs) {
	// kqueue 생성
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

// Server 소켓 생성 및 설정
void Server::setupServerSockets() {
	// 서버 설정 개수만큼 반복
	for (size_t i = 0; i < serverConfigs.size(); ++i) {
		// 소켓 생성
		int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (serverSocket == -1)
			exit_with_error("Failed to create socket");

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
	}
}

// 소켓 논블로킹 설정
void Server::setNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0); // 파일 디스크립터 플래그 가져오기
	if (flags == -1) {
		std::cerr << "Failed to get flags: " << strerror(errno) << std::endl;
		std::exit(EXIT_FAILURE);
	}
	// 논블로킹 설정
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::cerr << "Failed to set non-blocking: " << strerror(errno) << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

// 서버 실행
void Server::run() {
	// 이벤트 처리 루프
	while (true) {
		handleEvents();
	}
}

// 이벤트 처리
void Server::handleEvents() {
	// 이벤트 배열 생성
	struct kevent events[1024];

	// 이벤트 생성
	int numEvents = kevent(kq, NULL, 0, events, 1024, NULL);
	if (numEvents == -1) {
		std::cerr << "kevent error: " << strerror(errno) << std::endl;
		return;
	}

	// 이벤트 수신 감지 및 처리
	for (int i = 0; i < numEvents; ++i) {
		// 에러 발생 시
		if (events[i].flags & EV_ERROR) {
			std::cerr << "Event error: " << strerror(events[i].data) << std::endl;
			closeConnection(events[i].ident);
			continue;
		}

		if (events[i].filter == EVFILT_READ) {
			// 읽기 이벤트 발생 시
			if (std::find(serverSockets.begin(), serverSockets.end(), events[i].ident) != serverSockets.end()) {
				// 서버 소켓인 경우
				// 클라이언트 소켓 생성
				int clientSocket = accept(events[i].ident, NULL, NULL);

				// 클라이언트 소켓 생성 실패 시
				if (clientSocket == -1) {
					std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
					continue;
				}
				setNonBlocking(clientSocket);
				struct kevent event;
				// 클라이언트 소켓 이벤트 추가
				EV_SET(&event, clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
				if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
					std::cerr << "Failed to add event to kqueue: " << strerror(errno) << std::endl;
					close(clientSocket);
				}
			} 
			else {
				// 클라이언트 소켓인 경우
				handleClient(events[i].ident);
			}
		} else if (events[i].filter == EVFILT_WRITE) {
			// 쓰기 이벤트 발생 시
			// Handle write event if needed
		}
	}
}

// 클라이언트 요청 처리
void Server::handleClient(int clientSocket) {
	// 클라이언트 요청 수신
	char buffer[4096];
	ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead <= 0) {
		if (bytesRead == 0) {
			// 클라이언트 연결 종료 시
			std::cerr << "Client disconnected" << std::endl;
		} else {
			// 수신 에러 발생 시
			std::cerr << "recv error: " << strerror(errno) << std::endl;
		}
		// 클라이언트 소켓 종료
		closeConnection(clientSocket);
		return;
	}

	// 수신 데이터 출력
	buffer[bytesRead] = '\0';
	std::cout << "Received request: " << buffer << std::endl;

	// 클라이언트 요청 처리
	// Handle request and generate response
	std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
	sendResponse(clientSocket, response);
}

// 응답 전송
void Server::sendResponse(int clientSocket, const std::string& response) {
	ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
	if (bytesSent == -1) {
		std::cerr << "send error: " << strerror(errno) << std::endl;
		closeConnection(clientSocket);
	}
}

// 클라이언트 소켓 종료
void Server::closeConnection(int clientSocket) {
	close(clientSocket);
	struct kevent event;
	EV_SET(&event, clientSocket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	kevent(kq, &event, 1, NULL, 0, NULL);
}
