#include "Server.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "Exception.hpp"
#include "HTTPException.hpp"
#include "RequestHandler.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"

// ServerConfig 클래스 생성자
Server::Server(const Config& config)
: config(config)
, eventManager()
{
    // Server 소켓 생성 및 설정
    setupServerSockets();
}

Server::~Server()
{
    for (size_t i = 0; i < serverSockets.size(); ++i)
    {
        close(serverSockets[i]);
    }

    for (size_t i = 0; i < clientSockets.size(); ++i)
        close(clientSockets[i]);
}

// Server 소켓 생성 및 설정
void Server::setupServerSockets()
{
    /*
     * 다중 서버 소켓을 관리하는 이유 : 하나의 서버 소켓이 하나의 port에 대한 연결 요청을 처리한다.
     * 예시로 443(https)포트와 8080(http)포트를 동시에 사용하는 경우, 서버 소켓을 2개 생성하여 각각의 포트에 대한 연결 요청을 처리한다.
     * 또한 서버는 여러개의 IP를 가질 수 있으므로, 서버 소켓을 여러개 생성하여 각각의 IP에 대한 연결 요청을 처리할 수 있다.
     * 서버가 여러개의 IP를 가지는 경우는 서버가 여러개의 네트워크 인터페이스를 가지는 경우이다.
     */
    std::vector<ServerConfig> serverConfigs = config.getServerConfigs();

    for (size_t i = 0; i < serverConfigs.size(); ++i)
    {
        // 소켓 생성
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1)
            throw std::runtime_error("Failed to create socket");

        /* 개발 편의용 세팅. 서버 소켓이 이미 사용중이더라도 실행되게끔 설정 */
        int optval = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        /* ----------------------------------------------------------------- */

        // 소켓 옵션 설정
        setNonBlocking(serverSocket);

        // 서버 주소 설정
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));                               // 0으로 초기화
        serverAddr.sin_family = AF_INET;                                          // IPv4
        serverAddr.sin_port = htons(serverConfigs[i].port);                       // 포트 설정
        inet_pton(AF_INET, serverConfigs[i].host.c_str(), &serverAddr.sin_addr);  // IP 주소 설정

        // 소켓 바인딩
        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
        {
            close(serverSocket);
            throw Exception(FAILED_TO_BIND_SOCKET);
        }

        // 소켓 리스닝
        if (listen(serverSocket, 10) == -1)
        {
            close(serverSocket);
            throw Exception(FAILED_TO_LISTEN_SOCKET);
        }

        // 서버 소켓 벡터에 추가
        serverSockets.push_back(serverSocket);
        socketToConfigMap[serverSocket] = serverConfigs[i];

        eventManager.addReadEvent(serverSocket);
    }
}

// 소켓 논블로킹 설정
void Server::setNonBlocking(int socket)
{
    // 파일 디스크립터 플래그 가져오기
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("Failed to get flags");

    // 논블로킹 설정
    if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("Failed to set non-blocking");
}

// 서버 실행
void Server::run()
{
    // 이벤트 처리 루프
    while (true)
    {
        std::vector<struct kevent> events = eventManager.getCurrentEvents();
        for (std::vector<struct kevent>::iterator it = events.begin(); it != events.end(); ++it)
        {
            struct kevent& event = *it;

            if (event.flags & EV_ERROR)
            {
                std::cerr << "Event error: " << strerror(event.data) << std::endl;
                close(event.ident);
                continue;
            }

            if (std::find(serverSockets.begin(), serverSockets.end(), event.ident) != serverSockets.end())
            {
                acceptClient(event.ident);
            }
            else if (event.filter == EVFILT_READ)
            {
                handleClientReadEvent(event);
            }
            else if (event.filter == EVFILT_WRITE)
            {
                handleClientWriteEvent(event);
            }
        }

        checkTimeout();
    }
}

void Server::checkTimeout()
{
    time_t now = time(NULL);

    for (size_t i = 0; i < clientSockets.size(); ++i)
    {
        int clientSocket = clientSockets[i];
        int timeout = socketToConfigMap[clientSocket].keepalive_timeout;

        if (difftime(now, last_activity_map[clientSocket]) > timeout)
        {
            std::cout << "Timeout. Connection closed: " << clientSocket << std::endl;
            closeConnection(clientSocket);
        }
    }
}

void Server::update_last_activity(int socket)
{
    last_activity_map[socket] = time(NULL);
}

void Server::acceptClient(int serverSocket)
{
    // 서버 소켓으로 읽기 이벤트가 발생했다는 것의 의미 : 새로운 클라이언트가 연결 요청을 보냈다는 것
    // 서버 소켓인 경우 'accept'를 이용하여 클라이언트의 연결 수락
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
    if (clientSocket == -1)
    {
        std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
        return;
    }

    setNonBlocking(clientSocket);
    update_last_activity(clientSocket);

    try
    {
        eventManager.addReadEvent(clientSocket);
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
        close(clientSocket);
        return;
    }

    clientSockets.push_back(clientSocket);
    socketToConfigMap[clientSocket] = socketToConfigMap[serverSocket];
}

// 클라이언트 요청 처리
void Server::handleClientReadEvent(struct kevent& event)
{
    // 입력 스트림이 EOF인 경우 연결 종료
    if (event.flags & EV_EOF)
    {
        closeConnection(event.ident);
        return;
    }

    uintptr_t socket = event.ident;

    update_last_activity(socket);

    // 클라이언트 요청 수신
    char buffer[4096];
    ssize_t bytesRead;
    std::string& requestData = recvDataMap[socket];

    if ((bytesRead = recv(socket, buffer, sizeof(buffer) - 1, 0)) < 0)
    {
        std::cerr << "recv error" << std::endl;
        closeConnection(socket);
        return;
    }

    buffer[bytesRead] = '\0';
    requestData += buffer;

    size_t requestLength = 0;

    bool hasComplete = false;

    // 읽은 데이터가 하나의 Request 단위가 완성 됐을 때 처리
    while (isCompleteRequest(requestData, requestLength))
    {
        std::string completeRequest = requestData.substr(0, requestLength);
        completeDataMap[socket].push_back(completeRequest);
        requestData.erase(0, requestLength);

        hasComplete = true;
    }

    // 리소스 효율을 위해 요청이 하나 완료 되면 그 때 WRITE 이벤트 등록
    if (hasComplete)
        eventManager.addWriteEvent(socket);
}

bool Server::isCompleteRequest(const std::string& data, size_t& requestLength)
{
    size_t headerEnd = data.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return false;

    requestLength = headerEnd + 4;

    std::istringstream headerStream(data.substr(0, headerEnd + 4));
    std::string headerLine;
    while (std::getline(headerStream, headerLine) && headerLine != "\r")
    {
        if (headerLine.find("Content-Length:") != std::string::npos)
        {
            requestLength += std::stoul(headerLine.substr(16));
        }
    }

    return data.size() >= requestLength;
}

void Server::handleClientWriteEvent(struct kevent& event)
{
    uintptr_t socket = event.ident;
    std::vector<std::string>& completeData = completeDataMap[socket];
    bool bKeepAlive = true;

    for (size_t i = 0; i < completeData.size(); ++i)
    {
        std::string requestData = completeData[i];
        bKeepAlive = handleRequest(socket, requestData);
    }

    while (!completeData.empty())
    {
        completeData.pop_back();
    }

    eventManager.deleteWriteEvent(socket);
    if (!bKeepAlive)
        closeConnection(socket);
}

bool Server::handleRequest(int socket, const std::string& requestData)
{
    ResponseMessage resMsg;
    RequestHandler requestHandler(resMsg, socketToConfigMap[socket]);
    try
    {
        RequestMessage reqMsg(requestData);
        requestHandler.verifyRequest(reqMsg);
        requestHandler.handleRequest(reqMsg);
    }
    catch (const HTTPException& e)
    {
        requestHandler.handleException(e);
    }

    logHTTPMessage(socket, resMsg, requestData);
    sendResponse(socket, resMsg);

    // 만약 reqMsg가 keep-alive여도 400 bad request가 떨어지면 keep-alive를 무시하고 연결을 끊는다.
    // ResponseMessage에서 keep-alive를 확인
    return resMsg.isKeepAlive();
}

void Server::logHTTPMessage(int socket, ResponseMessage& res, const std::string& reqData)
{
    std::ofstream logFile("access.log", std::ios::app);

    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);

    logFile << asctime(timeinfo) << "Client IP: " << socketToConfigMap[socket].host << '\n'
            << "Request:\n"
            << reqData << "Response:\n"
            << res.toString() << "\n----------------------------------------------------------------------------------------\n"
            << std::endl;
    logFile.close();
}

void Server::sendResponse(int socket, ResponseMessage& res)
{
    std::string responseStr = res.toString();
    eventManager.addWriteEvent(socket);

    std::vector<struct kevent> events = eventManager.getCurrentEvents();
    for (std::vector<struct kevent>::iterator it = events.begin(); it != events.end(); ++it)
    {
        struct kevent& event = *it;

        if (event.ident != (uintptr_t)socket)
            continue;

        if (event.filter == EVFILT_WRITE)
        {
            if ((send(socket, responseStr.c_str(), responseStr.length(), 0)) < 0)
            {
                std::cerr << "send error: " << strerror(errno) << std::endl;
                eventManager.deleteWriteEvent(socket);
                closeConnection(socket);
                return;
            }
        }
    }
}

// 클라이언트 소켓 종료
void Server::closeConnection(int socket)
{
    eventManager.deleteReadEvent(socket);

    close(socket);

    std::vector<int>::iterator it = std::find(clientSockets.begin(), clientSockets.end(), socket);
    if (it != clientSockets.end())
        clientSockets.erase(it);

    socketToConfigMap.erase(socket);
    last_activity_map.erase(socket);

    if (!recvDataMap.empty())
        recvDataMap.erase(socket);

    if (!completeDataMap.empty())
        completeDataMap.erase(socket);
}
