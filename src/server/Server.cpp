#include "Server.hpp"
#include "error.hpp"
#include "HttpResponse.hpp"
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

	/*
	 * 다중 서버 소켓을 관리하는 이유 : 하나의 서버 소켓이 하나의 port에 대한 연결 요청을 처리한다.
	 * 예시로 443(https)포트와 8080(http)포트를 동시에 사용하는 경우, 서버 소켓을 2개 생성하여 각각의 포트에 대한 연결 요청을 처리한다.
	 * 또한 서버는 여러개의 IP를 가질 수 있으므로, 서버 소켓을 여러개 생성하여 각각의 IP에 대한 연결 요청을 처리할 수 있다.
	 * 서버가 여러개의 IP를 가지는 경우는 서버가 여러개의 네트워크 인터페이스를 가지는 경우이다.
	*/
	for (size_t i = 0; i < serverSockets.size(); ++i) {

		struct kevent event;
		// 서버 소켓 중 하나인 serverSockets[i]에 대한 "읽기 이벤트가 발생하는 경우"를 이벤트 감시 대상으로 추가
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

	/*
	 * !!수신이 감지된 이벤트를 가져온다!!
	 * events 이벤트 배열에 현재 변경이 감지된 이벤트를 저장한다.
	 * numEvents에는 배열에 담긴 이벤트의 수가 담긴다.
	 */
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
				// 서버 소켓으로 읽기 이벤트가 발생했다는 것의 의미 : 새로운 클라이언트가 연결 요청을 보냈다는 것
				// 서버 소켓인 경우 'accept'를 이용하여 클라이언트의 연결 수락
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
				// kqueue에 클라이언트 소켓을 감시하는 이벤트 추가
				if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
					std::cerr << "Failed to add event to kqueue: " << strerror(errno) << std::endl;
					close(clientSocket);
				}
			} 
			else {
				// 클라이언트 소켓으로 읽기 이벤트가 발생했다는 것의 의미 : 클라이언트가 요청을 보냈다는 것
				// 클라이언트 소켓인 경우 'handleClient'를 호출하여 클라이언트 요청 처리
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
	HttpResponse response;
	response.setStatusCode(200, "OK");
    response.setHeader("Content-Type", "text/plain");
    response.setBody("Hello, World!");

	sendResponse(clientSocket, response);
}

// 응답 전송
void Server::sendResponse(int clientSocket, const HttpResponse& response) {
    std::string responseStr = response.toString();
    ssize_t bytesSent = send(clientSocket, responseStr.c_str(), responseStr.length(), 0);
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
